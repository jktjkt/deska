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
def main(kindName,objectName,attributeName,data):
	fname = kindName+"_set_"+attributeName+"(text,text)"
	call(fname,objectName,data)

	jsn = dict()
	jsn["responce"] = "setAttribute"
	jsn["kindName"] = kindName
	jsn["objectName"] = objectName
	jsn["attributeName"] = attributeName
	jsn["data"] = data
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
	jsn["responce"] = "renameObject"
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
	jsn["responce"] = "createObject"
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
	jsn["responce"] = "deleteObject"
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
	jsn["responce"] = "objectData"
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

