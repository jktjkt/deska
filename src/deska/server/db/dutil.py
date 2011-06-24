# this is deska util module with helper functions for jsonapi

import Postgres
import json
import generated

class DeskaException(Exception):
	'''Exception class for deska exceptions'''

	typeDict = {
		'70002': 'ChangesetAlreadyOpenError',
		'70003': 'NoChangesetError',
		'70010': 'ReCreateObjectError',
		'70021': 'NotFoundError',
		'*': 'ServerError'
	}

	def __init__(self,dberr):
		'''Construct DeskaException from Postgres.dberr exception'''
		self.code = dberr.code
		self.message = dberr.message
		self.parseDberr()
		
	def parseDberr(self):
		'''Parse Postgres.dberr into variables used for json dump'''
		self.type = self.getType(self.code)
		if self.code == '42601':
			self.message = "Syntax error, something strange happend."
		if self.code == '42883':
			self.message = "Either kindName or attribute does not exists."

	def getType(self,errcode):
		'''Return DeskaExceptionType for given error code'''
		try:
			return self.typeDict[errcode]
		except:
			return "ServerError"


	def json(self,command,jsn = None):
		'''Create json error representation for deska API'''
		if jsn is None:
			jsn = dict()
		err = dict()
		err["type"] = self.type
		err["message"] = self.message

		jsn["dbException"] = err
		jsn["response"] = command

		return json.dumps(jsn)

def jsn(name,tag):
	'''Create json sceleton'''
	return {"response": name, "tag": tag}

def errorJson(self,command,tag,typ,message):
	'''Create json error string'''
	jsn = dict({"response": command, "tag": tag
		"dbException": {"type": typ, "message": message}
		})
	return json.dumps(jsn)

def mystr(s):
	'''Like str but only not for all'''
	if s is None:
		return s
	return str(s)

def fcall(fname,*args):
	'''Call stored procedure with params.
	@param fname ID of stored procedure like name(text)
	atr1, atr2 ... parameters for the stored procedure
	'''
	try:
		with xact():
			func = proc(fname)
			return func(*args)
		return 1
	except Postgres.Exception as dberr:
		raise DeskaException(dberr)

def getdata(select,*args):
	'''Get data from database.
	@param select Select statement
	atr1, atr2 ... parameters for the statement
	@returns tuple of column names and data cursor
	'''
	try:
		with xact():
			plan = prepare(select)
			return plan.column_names, plan(*args)
	except Postgres.Exception as dberr:
		raise DeskaException(dberr)
		
def params(argString):
	'''Get python structure from string'''
	return json.loads(argString)	
	

class Condition():
	'''Class to store and handle column/value/operator data'''

	opMap = {
		"columnEq": "=",
		"columnNe": "!=",
		"columnGt": ">",
		"columnGe": ">=",
		"columnLe": "<=",
		"columnLt": "<"
	}

	def __init__(self,data):
		'''Constructor, set local data and parse condition'''
		try:
			self.col = data["column"]
			self.val = data["value"]
			self.op = data["condition"]
			self.kind = data["kind"]
			self.parse()
			return
		except:
			pass # do not raise here
		Postgres.ERROR("Syntax error in condition",code = 70020)
	
	def parse(self):
		'''Update condition data for easy creation of Deska SQL condition'''
		if type(self.val) == str:
			self.val = "'{0}'".format(self.val)

		if self.col == "changeset" and self.kind == "metadata":
			self.col = "id"
			self.val = "changeset2id({0})".format(self.val)
		if self.col == "revision" and self.kind == "metadata":
			self.col = "num"
			self.val = "revision2num({0})".format(self.val)

		self.op = self.opMap[self.op]
	
	def get(self):
		'''Return deska SQL condition'''
		return "{0}.{1} {2} {3}".format(self.kind,self.col,self.op,self.val)
	
	def getAffectedKind(self):
		'''Return kind in condition'''
		return self.kind

class Filter():
	'''Class for handling filters'''

	def __init__(self,filterData):
		'''loads json filter data'''
		self.kinds = set()
		self.where = ''
		if filterData is None:
			self.data = None
			return
		try:
			self.data = json.loads(filterData)
			self.where = self.parse(self.data)
			return
		except Exception as err:
			pass # do not raise another exception in except part
		Postgres.ERROR("Syntax error when parsing filterData.",code = 70020)
		
	def getWhere(self):
		'''Return where part of sql statement'''
		if self.data is None:
			return ''
		return "WHERE " + self.where
	
	def getJoin(self,mykind):
		'''Return join part of sql statement'''
		ret = ''
		self.kinds = self.kinds - set({mykind})
		for kind in self.kinds:
			if mykind == "metadata":
				joincond = "{0}.id = {1}.version".format(mykind,kind)
			else:
				joincond = "{0}.uid = {1}.{0}".format(mykind,kind)
			ret = ret + " JOIN {tbl}_history AS {tbl} ON {cond} ".format(tbl = kind, cond = joincond)
		return ret
	
	def parse(self,data):
		'''Parse filter data and create SQL WHERE part'''
		if self.data == '':
			return ''
		try:
			operator = data["operator"]
			if operator == "and":
				res = [self.parse(expresion) for expresion in data["operands"]]
				return "(" + ") AND (".join(res) + ")"
			elif operator == "or":
				res = [self.parse(expresion) for expresion in data["operands"]]
				return "(" + ") OR (".join(res) + ")"
			else:
				Postgres.ERROR("Syntax error: bad operands",code = 70020)
		except:
			pass
		cond = Condition(data)
		# collect affected kinds (need for join)
		self.kinds.add(cond.getAffectedKind())
		return cond.get()

def kinds():
        return list(["vendor","hardware","host","interface"])

def oneKindDiff(kindName,a = None,b = None):
	with xact():
		if (a is None) and (b is None):
			# diff for temporaryChangeset
			init = proc(kindName + "_init_diff()")
			init()
		else:
			# diff for 2 revisions
			init = proc(kindName + "_init_diff(bigint,bigint)")
			#get changeset ids first
			revision2num = proc("revision2num(text)")
			a = revision2num(a)
			b = revision2num(b)
			init(a,b)

		terminate = proc(kindName + "_terminate_diff()")
		created = prepare("SELECT * FROM " + kindName + "_diff_created()")
		setattr = prepare("SELECT * FROM " + kindName + "_diff_set_attributes($1,$2)")
		deleted = prepare("SELECT * FROM " + kindName + "_diff_deleted()")

		res = list()
		for line in created():
			obj = dict()
			obj["command"] = "createObject"
			obj["kindName"] = kindName
			obj["objectName"] = mystr(line[0])
			res.append(obj)
		for line in setattr(a,b):
			obj = dict()
			obj["command"] = "setAttribute"
			obj["kindName"] = kindName
			obj["objectName"] = mystr(line[0])
			obj["attributeName"] = mystr(line[1])
			obj["oldValue"] = mystr(line[2])
			obj["newValue"] = mystr(line[3])
			res.append(obj)
		for line in deleted():
			obj = dict()
			obj["command"] = "deleteObject"
			obj["kindName"] = kindName
			obj["objectName"] = mystr(line[0])
			res.append(obj)
		terminate()
	return res
