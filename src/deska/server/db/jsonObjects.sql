SET search_path TO api,deska;

CREATE OR REPLACE FUNCTION jsn.setAttribute(kindName text, objectName text, attributeName text, attributeData text)
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(kindName,objectName,attributeName,attributeData):
	name = "setAttribute"
	jsn = dict()
	jsn["response"] = name
	jsn["kindName"] = kindName
	jsn["objectName"] = objectName
	jsn["attributeName"] = attributeName
	jsn["attributeData"] = attributeData
	fname = kindName+"_set_"+attributeName+"(text,text)"
	try:
		dutil.fcall(fname,objectName,attributeData)
	except dutil.DeskaException as err:
		return err.json(name,jsn)

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
	name = "renameObject"
	jsn = dict()
	jsn["response"] = name
	jsn["kindName"] = kindName
	jsn["oldObjectName"] = oldName
	jsn["newObjectName"] = newName

	fname = kindName+"_set_name(text,text)"
	try:
		dutil.fcall(fname,oldName,newName)
	except dutil.DeskaException as err:
		return err.json(name,jsn)

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
	name = "createObject"
	jsn = dict()
	jsn["response"] = name
	jsn["kindName"] = kindName
	jsn["objectName"] = objectName

	fname = kindName+"_add(text)"
	try:
		dutil.fcall(fname,objectName)
	except dutil.DeskaException as err:
		return err.json(name,jsn)

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
	name = "deleteObject"
	jsn = dict()
	jsn["response"] = name
	jsn["kindName"] = kindName
	jsn["objectName"] = objectName

	fname = kindName+"_del(text)"
	try:
		dutil.fcall(fname,objectName)
	except dutil.DeskaException as err:
		return err.json(name)

	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.restoreDeletedObject(kindName text, objectName text)
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(kindName,objectName):
	name = "restoreDeletedObject"
	jsn = dict()
	jsn["response"] = name
	jsn["kindName"] = kindName
	jsn["objectName"] = objectName

	fname = kindName+"_undel(text)"
	try:
		dutil.fcall(fname,objectName)
	except dutil.DeskaException as err:
		return err.json(name)

	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.objectData(kindName text, objectName text, revision text)
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(kindName,objectName,revision):
	jsn = dict()
	name = "objectData"
	jsn["response"] = name
	jsn["objectName"] = objectName
	jsn["kindName"] = kindName

	select = "SELECT * FROM {0}_get_data($1,$2)".format(kindName)
	try:
		revisionNumber = dutil.fcall("revision2num(text)",revision)
		colnames, data = dutil.getdata(select,objectName,revisionNumber)
	except dutil.DeskaException as dberr:
		return dberr.json(name,jsn)

	data = [dutil.mystr(x) for x in data[0]]
	res = dict(zip(colnames,data))
	jsn[name] = res
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
from dutil import mystr,kinds,oneKindDiff

@pytypes
def main(a,b):
	jsn = dict()
	name = "dataDifference"
	jsn["response"] = name
	jsn["revisionA"] = a
	jsn["revisionB"] = b
	
	res = list()
	try:
		for kindName in kinds():
			res.extend(oneKindDiff(kindName,a,b))
	except Postgres.Exception as dberr:
		err = dutil.DeskaException(dberr)
		return err.json(name,jsn)

	jsn[name] = res
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.dataDifferenceInTemporaryChangeset()
RETURNS text
AS
$$
import Postgres
import json
import dutil
from dutil import mystr,kinds,oneKindDiff

@pytypes
def main():
	jsn = dict()
	name = "dataDifferenceInTemporaryChangeset"
	jsn["response"] = name
	
	res = list()
	try:
		for kindName in kinds():
			res.extend(oneKindDiff(kindName))
	except Postgres.Exception as dberr:
		err = dutil.DeskaException(dberr)
		return err.json(name,jsn)

	jsn[name] = res
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

