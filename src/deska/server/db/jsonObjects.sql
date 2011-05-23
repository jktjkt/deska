SET search_path TO api,deska;

CREATE OR REPLACE FUNCTION jsn.setAttribute(kindName text, objectName text, attributeName text, attributeData text)
RETURNS text
AS
$$
import Postgres
import json

def call(fname,atr1,atr2):
	try:
		with xact():
			func = proc(fname)
			func(atr1, atr2)
		return 1
	except Postgres.Exception as dberr:
		if dberr.pg_errordata.code == "42883":
			# wait with raising exception
			pass
		else:
			raise
	# we cannot raise exception from except part, so wait for here
	raise Postgres.ERROR('Kind "{kind}" does not exists.'.format(kind = kindName),code = 10111)

@pytypes
def main(kindName,objectName,attributeName,attributeData):
	fname = kindName+"_set_"+attributeName+"(text,text)"
	call(fname,objectName,attributeData)

	jsn = dict()
	jsn["response"] = "setAttribute"
	jsn["kindName"] = kindName
	jsn["objectName"] = objectName
	jsn["attributeName"] = attributeName
	jsn["attributeData"] = attributeData
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.renameObject(kindName text, oldName text, newName text)
RETURNS text
AS
$$
import Postgres
import json

def call(fname,atr1,atr2):
	try:
		with xact():
			func = proc(fname)
			func(atr1)
		return 1
	except Postgres.Exception as dberr:
		if dberr.pg_errordata.code == "42883":
			# wait with raising exception
			pass
		else:
			raise
	# we cannot raise exception from except part, so wait for here
	raise Postgres.ERROR('Kind "{kind}" does not exists.'.format(kind = kindName),code = 10111)

@pytypes
def main(kindName,oldName,newName):
	fname = kindName+"_set_name(text,text)"
	call(fname,oldname,newname)

	jsn = dict()
	jsn["response"] = "renameObject"
	jsn["kindName"] = kindName
	jsn["oldName"] = oldName
	jsn["newName"] = newName
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.createObject(kindName text, objectName text)
RETURNS text
AS
$$
import Postgres
import json

def call(fname,atr1):
	try:
		with xact():
			func = proc(fname)
			func(atr1)
		return 1
	except Postgres.Exception as dberr:
		if dberr.pg_errordata.code == "42883":
			# wait with raising exception
			pass
		else:
			raise
	# we cannot raise exception from except part, so wait for here
	raise Postgres.ERROR('Kind "{kind}" does not exists.'.format(kind = kindName),code = 10111)

@pytypes
def main(kindName,objectName):
	fname = kindName+"_add(text)"
	call(fname,objectName)

	jsn = dict()
	jsn["response"] = "createObject"
	jsn["kindName"] = kindName
	jsn["objectName"] = objectName
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.deleteObject(kindName text, objectName text)
RETURNS text
AS
$$
import Postgres
import json

def call(fname,atr1):
	try:
		with xact():
			func = proc(fname)
			func(atr1)
		return 1
	except Postgres.Exception as dberr:
		if dberr.pg_errordata.code == "42883":
			# wait with raising exception
			pass
		else:
			raise
	# we cannot raise exception from except part, so wait for here
	raise Postgres.ERROR('Kind "{kind}" does not exists.'.format(kind = kindName),code = 10111)

@pytypes
def main(kindName,objectName):
	fname = kindName+"_del(text)"
	call(fname,objectName)

	jsn = dict()
	jsn["response"] = "deleteObject"
	jsn["kindName"] = kindName
	jsn["objectName"] = objectName
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.objectData(kindName text, objectName text)
RETURNS text
AS
$$
import Postgres
import json

def error_json(jsn,typ,message):
	err = dict()
	err["type"] = typ
	err["message"] = message
	jsn["dbException"] = err
	return json.dumps(jsn)

@pytypes
def main(kindName,objectName):
	jsn = dict()
	jsn["response"] = "objectData"
	jsn["objectName"] = objectName
	jsn["kindName"] = kindName

	try:
		with xact():
			plan = prepare("SELECT * FROM {0}_get_data($1)".format(kindName))
			data = plan(objectName)
	except Postgres.Exception as dberr:
		if dberr.pg_errordata.code == "42883":
			return error_json(jsn,"ServerError",'Kind "{0}" does not exists.'.format(kindName))
		return error_json(jsn,"ServerError",dberr.pg_errordata.message)

	data = [str(x) for x in data[0]]
	res = dict(zip(plan.column_names,data))
	jsn["objectData"] = res
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.dataDifference(a text, b text)
RETURNS text
AS
$$
import Postgres
import json

def error_json(jsn,typ,message):
	err = dict()
	err["type"] = typ
	err["message"] = message
	jsn["dbException"] = err
	return json.dumps(jsn)

def kinds():
	return list(["vendor","hardware","host","interface"])

def oneKindDiff(kindName,a,b):
	with xact():
		init = proc(kindName + "_init_diff(bigint,bigint)")
		terminate = proc(kindName + "_terminate_diff()")
		created = prepare("SELECT * FROM " + kindName + "_diff_created()")
		setattr = prepare("SELECT * FROM " + kindName + "_diff_set_attributes($1,$2)")
		deleted = prepare("SELECT * FROM " + kindName + "_diff_deleted()")
		revision2num = proc("revision2num(text)")
		
		#get changeset ids first
		a = revision2num(a)
		b = revision2num(b)

		res = list()
		
		init(a,b)
		for line in created():
			obj = dict()
			obj["command"] = "createObject"
			obj["kindName"] = kindName
			obj["objecName"] = str(line[0])
			res.append(obj)
		for line in setattr(a,b):
			obj = dict()
			obj["command"] = "setAttribute"
			obj["kindName"] = kindName
			obj["objecName"] = str(line[0])
			obj["attributeName"] = str(line[1])
			obj["oldValue"] = str(line[2])
			obj["newValue"] = str(line[3])
			res.append(obj)
		for line in deleted():
			obj = dict()
			obj["command"] = "deleteObject"
			obj["kindName"] = kindName
			obj["objecName"] = str(line[0])
			res.append(obj)
		terminate()

	return res

@pytypes
def main(a,b):
	jsn = dict()
	jsn["response"] = "dataDifference"
	jsn["a"] = a
	jsn["b"] = b
	
	res = list()
	try:
		for kindName in kinds():
			res.extend(oneKindDiff(kindName,a,b))
	except Postgres.Exception as dberr:
		#if dberr.pg_errordata.code == "42883":
		#	return error_json(jsn,"ServerError",'Kind "{0}" does not exists.'.format(kindName))
		return error_json(jsn,"ServerError",dberr.pg_errordata.message)

	jsn["dataDifference"] = res
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

