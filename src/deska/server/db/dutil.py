# this is deska util module with helper functions for jsonapi

import Postgres
import json
import re
import generated

class DeskaException(Exception):
	'''Exception class for deska exceptions'''

	typeDict = {
		'70002': 'ChangesetAlreadyOpenError',
		'70003': 'NoChangesetError',
		'70010': 'ReCreateObjectError',
		'70021': 'NotFoundError',
		'70007': 'ObsoleteParentError',
		'70004': 'ConstraintError',
		'70005': 'ReadOnlyError',
		'23502': 'ConstraintError',#NOT NULL
		'23514': 'ConstraintError',#check_positive
		'*': 'ServerError'
	}

	def __init__(self,dberr):
		'''Construct DeskaException from Postgres.dberr exception'''
		self.code = dberr.code
		self.message = dberr.message
		Postgres.NOTICE("Creating exception with code "+self.code)
		self.parseDberr()

	def parseDberr(self):
		'''Parse Postgres.dberr into variables used for json dump'''
		self.type = self.getType(self.code)
		if self.code == '42601':
			'''bad sql syntax'''
			self.message = "Syntax error, something strange happend."
		#if self.code == '42883':
		#	self.message = "Either kindName or attribute does not exists."

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
	if type(s) == Postgres.types.int8:
		return int(s)
	if type(s) == Postgres.types.int4:
		return int(s)
	if type(s) == Postgres.types.int2:
		return int(s)
	if type(s) == Postgres.types.float4:
		return float(s)
	if type(s) == Postgres.types.float8:
		return float(s)
	if type(s) == Postgres.types.bool:
		return bool(s)
	if type(s) == Postgres.types.text.Array:
		return [str(x) for x in list(s)]
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
			Postgres.NOTICE("Running command: "+select)
			plan = prepare(select)
			return plan.column_names, plan(*args)
	except Postgres.Exception as dberr:
		raise DeskaException(dberr)

def params(argString):
	'''Get python structure from string'''
	return json.loads(argString)

def hasTemplate(kindName):
	'''Test if given kind is templated'''
	if "template_"+kindName in generated.atts(kindName):
		return True
	else:
		return False

def getDataFunction(funcName,kindName):
	'''get name of the data function'''
	resolved = ["multipleResolvedObjectData","multipleResolvedObjectDataWithOrigin","resolvedObjectData", "resolvedObjectDataWithOrigin"]
	resolvedDict = {
		"multipleResolvedObjectData": "_resolved_data($1)",
		"multipleResolvedObjectDataWithOrigin": "_resolved_data_template_info($1)",
		"resolvedObjectData": "_resolved_object_data($1,$2)",
		"resolvedObjectDataWithOrigin": "_resolved_object_data_template_info($1,$2)"
	}
	nameDict = {
		"kindInstances": "_data_version($1)",
		"multipleObjectData": "_data_version($1)",
		"multipleResolvedObjectData": "_data_version($1)",
		"multipleResolvedObjectDataWithOrigin": "_data_version($1)",
		"objectData": "_get_data($1,$2)",
		"resolvedObjectData": "_get_data($1,$2)",
		"resolvedObjectDataWithOrigin": "_get_data($1,$2)"
	}
	if funcName in resolved:
		'''check if it is templated and if not, fake resolved data by unresolved function, that give same results'''
		if hasTemplate(kindName):
			'''return resolved version'''
			return resolvedDict[funcName]
	return nameDict[funcName]

def getSelect(kindName, functionName, columns = "*", join = "", where = ""):
	return 'SELECT {0} FROM {1}{2} AS {1} {3}{4}'.format(columns, kindName, getDataFunction(functionName,kindName), join, where)

def fakeOriginColumns(columns,objectName):
	'''fake data into small arrays of [origin,value]'''
	data = dict()
	for col in columns:
		if columns[col] == None:
			data[col] = [None,None]
		else:
			data[col] = [objectName,columns[col]]
	return data

def collectOriginColumns(columns,objectName):
	'''collect data into small arrays of [origin,value]'''
	origin = dict()
	data = dict()
	for col in columns:
		if re.match('.*_templ$',col):
			origin[col[0:len(col)-6]] = columns[col]
		# add also template origin, that is None, all the time
		elif re.match('^template_.*',col):
			if columns[col] == None:
				'''Caused by api, we cannot read this data from db, we must fake it'''
				origin[col] = None
			else:
				origin[col] = objectName
			data[col] = columns[col]
		else:
			data[col] = columns[col]
	for col in origin:
		data[col] = [origin[col],data[col]]
	return data

def oneKindDiff(kindName,diffname,a = None,b = None):
	'''get diff for one kind'''
	if diffname == "resolved":
		if hasTemplate(kindName):
			diffname = "_resolved_diff"
		else:
			diffname = "_diff"
	else:
		diffname = "_diff"
	with xact():
		Postgres.NOTICE("Running diff: {0}{1}({2},{3})".format(kindName,diffname,a,b))
		if (a is None) and (b is None):
			# diff for temporaryChangeset
			diffname = "_init_ch" + diffname
			init = proc(kindName + diffname + "(bigint)")
			#send proper values for diff_set_attr... (#292)
			a, b = 0, 0
			init(a)
		elif (b is None):
			# diff for temporaryChangeset with changeset parameter
			diffname = "_init_ch" + diffname
			init = proc(kindName + diffname + "(bigint)")
			#get changeset ids first
			changeset2id = proc("changeset2id(text)")
			a = changeset2id(a)
			#send proper values for diff_set_attr... (#292)
			b = 0
			init(a)
		else:
			# diff for 2 revisions
			diffname = "_init" + diffname
			init = proc(kindName + diffname + "(bigint,bigint)")
			#get changeset ids first
			revision2num = proc("revision2num(text)")
			a = revision2num(a)
			b = revision2num(b)
			init(a,b)

		terminate = proc(kindName + "_terminate_diff()")
		created = prepare("SELECT * FROM " + kindName + "_diff_created()")
		renamed = prepare("SELECT * FROM " + kindName + "_diff_rename()")
		setattr = prepare("SELECT * FROM " + kindName + "_diff_set_attributes($1,$2)")
		deleted = prepare("SELECT * FROM " + kindName + "_diff_deleted()")

		res = list()
		for line in deleted():
			obj = dict()
			obj["command"] = "deleteObject"
			obj["kindName"] = kindName
			obj["objectName"] = mystr(line[0])
			res.append(obj)
		for line in renamed():
			obj = dict()
			obj["command"] = "renameObject"
			obj["kindName"] = kindName
			obj["oldObjectName"] = mystr(line[0])
			obj["newObjectName"] = mystr(line[1])
			res.append(obj)
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
			obj["oldAttributeData"] = mystr(line[2])
			obj["attributeData"] = mystr(line[3])
			res.append(obj)
		terminate()
	return res

