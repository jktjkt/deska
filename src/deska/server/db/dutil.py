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
		'70005': 'SpecialReadOnlyAttributeError',
		'70006': 'AlreadyExistsError',
		'70011': 'ChangesetParsingError',
		'70012': 'RevisionParsingError',
		'70013': 'RevisionRangeError',
		'70014': 'ChangesetRangeError',
		'70015': 'ChangesetLockingError',
		'70016': 'ConstraintError',#check_in_cycle
		'23502': 'ConstraintError',#NOT NULL
		'23514': 'ConstraintError',#check_positive
		'10111': 'SqlError',#check_positive
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

def pytypes(iterable,specialCols = []):
	if specialCols == []:
		'''No columns with special type (array)'''
		return Postgres.convert_postgres_objects(iterable)
	# OK, we need to do some manual type convesions
	iterable = list(iterable)
	for i in range(0,len(iterable)):
		if type(iterable[i]) == Postgres.types.text.Array:
			iterable[i] = [str(x) for x in list(iterable[i])]
	return Postgres.convert_postgres_objects(iterable)

def fcall(fname,*args):
	'''Call stored procedure with params.
	@param fname ID of stored procedure like name(text)
	atr1, atr2 ... parameters for the stored procedure
	'''
	try:
		with xact():
			Postgres.NOTICE("Running function: {0}({1})".format(fname,args))
			func = proc(fname)
			return func(*args)
		return 1
	except Postgres.Exception as dberr:
		raise DeskaException(dberr)

import time
def getdata(select,*args):
	'''Get data from database.
	@param select Select statement
	atr1, atr2 ... parameters for the statement
	@returns tuple of column names and data cursor
	'''
	try:
		with xact():
			Postgres.NOTICE("Running command: "+select)
			Postgres.NOTICE(args)
			t = time.time()
			plan = prepare(select)
			ret = plan(*args)
			Postgres.NOTICE("Takes: {0}s".format(time.time() - t))
			return plan.column_names, ret

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

def getDataSuffix(kindName,directAccess):
	'''get suffix of name of the data function'''
	if directAccess:
		return ""
	if hasTemplate(kindName):
		return "_resolved_data($1)"
	return "_data_version($1)"

def getDataFunction(funcName,kindName,directAccess):
	'''get name of the data function'''
	if directAccess:
		return ""
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

def getSelect(kindName, functionName, columns = "*", join = "", where = "", directAccess = False):
	'''Create SELECT string'''
	return 'SELECT {0} FROM {1}{2} AS {1} {3}{4}'.format(columns, kindName, getDataFunction(functionName,kindName,directAccess), join, where)

def getAtts(atts,kindName,addName = False):
	'''Get attributes - columns definition'''
	specialCols = list()
	if addName:
		atts["name"] = "identifier"
	if atts == {}:
		return {"":"*"}, []
	# dot the atts with kindName
	new_atts = dict()
	for att in atts:
		if atts[att] in ["macaddr","date"]:
			new_atts[att] = "{0}.{1}::text".format(kindName,att)
		elif atts[att] in ["ipv4","ipv6"]:
			new_atts[att] = "host({0}.{1}) AS {1}".format(kindName,att)
		elif atts[att] == "timestamp":
			new_atts[att] = "to_char({0}.{1},'YYYY-MM-DD HH24:MI:SS') AS {1}".format(kindName,att)
		elif atts[att] == "identifier_set":
			new_atts[att] = "{0}.{1}".format(kindName,att)
			specialCols.append(att)
		else:
			new_atts[att] = "{0}.{1}".format(kindName,att)
	return new_atts, specialCols

def getColumnIndexes(columns,specialCols):
	'''get list of indexes of given special columns in column/table result'''
	i = 0
	ret = list()
	# now we don't need to have exact mapping, just need indexes number in any order
	# if someone would need it, this must be changed - for col in specialCols, find index of col in columns...
	for col in columns:
		if col in specialCols:
			ret.append(i)
		i = i + 1
	return ret

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
		if (a is None) and (b is None):
			# diff for temporaryChangeset
			diffname = "_init_ch" + diffname
			init = proc(kindName + diffname + "(bigint)")
			#send proper values for diff_set_attr... (#292)
			a = 0
			init(a)
			paramsDef = "$1"
			params = [a]
		elif (b is None):
			# diff for temporaryChangeset with changeset parameter
			diffname = "_init_ch" + diffname
			init = proc(kindName + diffname + "(bigint)")
			#get changeset ids first
			changeset2id = proc("changeset2id(text)")
			a = changeset2id(a)
			init(a)
			paramsDef = "$1"
			params = [a]
		else:
			# diff for 2 revisions
			diffname = "_init" + diffname
			init = proc(kindName + diffname + "(bigint,bigint)")
			#get changeset ids first
			revision2num = proc("revision2num(text)")
			a = revision2num(a)
			b = revision2num(b)
			init(a,b)
			paramsDef = "$1,$2"
			params = [a,b]
		Postgres.NOTICE("Running diff: {0}{1} {2}".format(kindName,diffname,[int(x) for x in params]))

		terminate = proc(kindName + "_terminate_diff()")
		created = prepare("SELECT * FROM {0}_diff_created({1})".format(kindName,paramsDef))
		renamed = prepare("SELECT * FROM {0}_diff_rename({1})".format(kindName,paramsDef))
		setattr = prepare("SELECT * FROM {0}_diff_set_attributes({1})".format(kindName,paramsDef))
		if "identifier_set" in generated.atts(kindName).values():
			'''for identifier_set'''
			setattr2 = prepare("SELECT * FROM {0}_diff_refs_set_set_attributes({1})".format(kindName,paramsDef))
		else:
			setattr2 = None
		deleted = prepare("SELECT * FROM {0}_diff_deleted({1})".format(kindName,paramsDef))

		res = list()
		for line in deleted(*params):
			line = pytypes(line)
			obj = dict()
			obj["command"] = "deleteObject"
			obj["kindName"] = kindName
			obj["objectName"] = line[0]
			res.append(obj)
		for line in renamed(*params):
			line = pytypes(line)
			obj = dict()
			obj["command"] = "renameObject"
			obj["kindName"] = kindName
			obj["oldObjectName"] = line[0]
			obj["newObjectName"] = line[1]
			res.append(obj)
		for line in created(*params):
			line = pytypes(line)
			obj = dict()
			obj["command"] = "createObject"
			obj["kindName"] = kindName
			obj["objectName"] = line[0]
			res.append(obj)
		setattrRes = setattr(*params)
		if setattr2 is not None:
			setattrRes = setattrRes + setattr2(*params)
		for line in setattrRes:
			line = pytypes(line,[2,3])
			obj = dict()
			obj["command"] = "setAttribute"
			obj["kindName"] = kindName
			obj["objectName"] = line[0]
			obj["attributeName"] = line[1]
			obj["oldAttributeData"] = line[2]
			obj["attributeData"] = line[3]
			res.append(obj)
		terminate()
	return res

