SET search_path TO api,deska;

CREATE OR REPLACE FUNCTION jsn.setAttribute(tag text, kindName text, objectName text, attributeName text, attributeData text)
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(tag,kindName,objectName,attributeName,attributeData):
	name = "setAttribute"
	jsn = dutil.jsn(name,tag)
	
	fname = kindName+"_set_"+attributeName+"(text,text)"
	try:
		dutil.fcall(fname,objectName,attributeData)
	except dutil.DeskaException as err:
		return err.json(name,jsn)

	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.renameObject(tag text, kindName text, oldName text, newName text)
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(tag,kindName,oldName,newName):
	name = "renameObject"
	jsn = dutil.jsn(name,tag)

	fname = kindName+"_set_name(text,text)"
	try:
		dutil.fcall(fname,oldName,newName)
	except dutil.DeskaException as err:
		return err.json(name,jsn)

	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.createObject(tag text, kindName text, objectName text)
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(tag,kindName,objectName):
	name = "createObject"
	jsn = dutil.jsn(name,tag)

	fname = kindName+"_add(text)"
	try:
		dutil.fcall(fname,objectName)
	except dutil.DeskaException as err:
		return err.json(name,jsn)

	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.deleteObject(tag text, kindName text, objectName text)
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(tag,kindName,objectName):
	name = "deleteObject"
	jsn = dutil.jsn(name,tag)

	fname = kindName+"_del(text)"
	try:
		dutil.fcall(fname,objectName)
	except dutil.DeskaException as err:
		return err.json(name)

	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.restoreDeletedObject(tag text, kindName text, objectName text)
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(tag,kindName,objectName):
	name = "restoreDeletedObject"
	jsn = dutil.jsn(name,tag)

	fname = kindName+"_undel(text)"
	try:
		dutil.fcall(fname,objectName)
	except dutil.DeskaException as err:
		return err.json(name)

	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.objectData(tag text, kindName text, objectName text, revision text)
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(tag,kindName,objectName,revision):
	name = "objectData"
	jsn = dutil.jsn(name,tag)

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

CREATE OR REPLACE FUNCTION jsn.dataDifference(tag text, a text, b text)
RETURNS text
AS
$$
import Postgres
import json
import dutil
from dutil import mystr,kinds,oneKindDiff

@pytypes
def main(tag,a,b):
	name = "dataDifference"
	jsn = dutil.jsn(name,tag)
	
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

CREATE OR REPLACE FUNCTION jsn.dataDifferenceInTemporaryChangeset(tag text)
RETURNS text
AS
$$
import Postgres
import json
import dutil
from dutil import mystr,kinds,oneKindDiff

@pytypes
def main(tag):
	name = "dataDifferenceInTemporaryChangeset"
	jsn = dutil.jsn(name,tag)
	
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

