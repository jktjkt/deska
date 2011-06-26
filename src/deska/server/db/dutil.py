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

class DutilException(DeskaException):
	'''Exception in pgpython code'''

	def __init__(self,type,message):
		self.type = type
		self.message = message

def jsn(name,tag):
	'''Create json sceleton'''
	return {"response": name, "tag": tag}

def errorJson(command,tag,typ,message):
	'''Create json error string'''
	jsn = dict({"response": command, "tag": tag,
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

	def __init__(self,data,condId):
		'''Constructor, set local data and parse condition'''
		try:
			self.val = data["value"]
			self.op = data["condition"]
			self.id = "${0}".format(condId)
			if "metadata" in data:
				self.kind = "metadata"
				self.col = data["metadata"]
				# we cannot see now, if it is about pending changeset or list revisions
				if self.col not in ["revision","message","author","timestamp","changeset"]:
					raise DutilException("FilterError","Attribute {0} does not exists.".format(self.col))
			elif "kind" in data:
				self.kind = data["kind"]
				if self.kind not in generated.kinds():
					raise DutilException("FilterError","Kind {0} does not exists.".format(self.col))
				self.col = data["column"]
				# add also name 
				if self.col not in generated.atts(self.kind) and self.col != "name":
					raise DutilException("FilterError","Attribute {0} does not exists.".format(self.col))
			else:
				raise DutilException("FilterError","Syntax error in condition.")
			self.parse()
			return
		except Exception as e:
			raise DutilException("FilterError","Syntax error in condition: "+str(e))
	
	def parse(self):
		'''Update condition data for easy creation of Deska SQL condition'''
		if self.col == "changeset" and self.kind == "metadata":
			self.col = "id"
			self.id = "changeset2id({0})".format(self.id)
		if self.col == "revision" and self.kind == "metadata":
			self.col = "num"
			self.id = "revision2num({0})".format(self.id)

		self.op = self.opMap[self.op]
	
	def get(self):
		'''Return deska SQL condition'''
		return "{0}.{1} {2} {3}".format(self.kind,self.col,self.op,self.id), self.val
	
	def getAffectedKind(self):
		'''Return kind in condition'''
		return self.kind

class Filter():
	'''Class for handling filters'''

	def __init__(self,filterData):
		'''loads json filter data'''
		# counter for value id's in select string
		self.counter = 1
		# list of values for select string
		self.values = list()
		# set of kinds
		self.kinds = set()
		self.where = ''
		if filterData is None:
			self.data = None
			return
		try:
			self.data = json.loads(filterData)
		except Exception as err:
			raise DutilException("FilterError","Syntax error in filter.")
		self.where = self.parse(self.data)
		
	def getWhere(self):
		'''Return where part of sql statement'''
		if self.data is None:
			return '',[]
		return "WHERE " + self.where, self.values
	
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
		if "operator" in data:
			operator = data["operator"]
			if operator == "and":
				res = [self.parse(expresion) for expresion in data["operands"]]
				return "(" + ") AND (".join(res) + ")"
			elif operator == "or":
				res = [self.parse(expresion) for expresion in data["operands"]]
				return "(" + ") OR (".join(res) + ")"
			else:
				raise DutilException("FilterError","Bad operands.")
		cond = Condition(data,self.counter)
		self.counter = self.counter + 1
		# collect affected kinds (need for join)
		self.kinds.add(cond.getAffectedKind())
		ret, value = cond.get()
		#add value into list
		self.values.append(value)
		return ret

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
