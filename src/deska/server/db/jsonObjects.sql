SET search_path TO api,deska;

CREATE OR REPLACE FUNCTION jsn.setAttribute(kindName text, objectName text, attributeName text, attributeData text)
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(kindName,objectName,attributeName,attributeData):
	fname = kindName+"_set_"+attributeName+"(text,text)"
	try:
		dutil.fcall(fname,objectName,attributeData)
	except dutil.DeskaException as err:
		return err.json("setAttribute")

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
import dutil
import json

@pytypes
def main(kindName,oldName,newName):
	fname = kindName+"_set_name(text,text)"
	try:
		dutil.fcall(fname,oldName,newName)
	except dutil.DeskaException as err:
		return err.json("renameObject")

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
import dutil
import json

@pytypes
def main(kindName,objectName):
	fname = kindName+"_add(text)"
	try:
		dutil.fcall(fname,objectName)
	except dutil.DeskaException as err:
		return err.json("createObject")

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
import dutil
import json

@pytypes
def main(kindName,objectName):
	fname = kindName+"_del(text)"
	try:
		dutil.fcall(fname,objectName)
	except dutil.DeskaException as err:
		return err.json("deleteObject")

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

def mystr(s):
	if s is None:
		return s
	return str(s)

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
		return error_json(jsn,"ServerError",dberr.pg_errordata.message)

	data = [mystr(x) for x in data[0]]
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
import dutil

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
		err = dutil.DeskaException(dberr)
		return err.json("dataDifference")

	jsn["dataDifference"] = res
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

