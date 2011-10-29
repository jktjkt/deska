SET search_path TO deska,api;

CREATE OR REPLACE FUNCTION jsn.startChangeset(tag text)
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(tag):
	name = "startChangeset"
	jsn = dutil.jsn(name,tag)

	fname = 'api.'+ name + "()"
	try:
		ver = dutil.fcall(fname)
	except dutil.DeskaException as err:
		return err.json(name,jsn)

	jsn[name] = dutil.mystr(ver)
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.abortCurrentChangeset(tag text)
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(tag):
	name = "abortCurrentChangeset"
	jsn = dutil.jsn(name,tag)

	fname = 'api.'+ name + "()"
	try:
		ver = dutil.fcall(fname)
	except dutil.DeskaException as err:
		return err.json(name,jsn)

	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.resumeChangeset(tag text, revision text)
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(tag,revision):
	name = "resumeChangeset"
	jsn = dutil.jsn(name,tag)

	fname = 'api.'+ name + "(text)"
	try:
		ver = dutil.fcall(fname,revision)
	except dutil.DeskaException as err:
		return err.json(name,jsn)

	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.commitChangeset(tag text, commitMessage text)
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(tag,commitMessage):
	name = "commitChangeset"
	jsn = dutil.jsn(name,tag)

	fname = 'api.'+ name + "(text)"
	try:
		ver = dutil.fcall(fname,commitMessage)
	except dutil.DeskaException as err:
		return err.json(name,jsn)
	jsn[name] = dutil.mystr(ver)
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.detachFromCurrentChangeset(tag text, message text)
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(tag,message):
	name = "detachFromCurrentChangeset"
	jsn = dutil.jsn(name,tag)

	fname = 'api.'+ name + "(text)"
	try:
		ver = dutil.fcall(fname,message)
	except dutil.DeskaException as err:
		return err.json(name,jsn)

	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.pendingChangesets(tag text, filter text = NULL)
RETURNS text
AS
$$
import dutil
from dutil import mystr
from filter import Filter
import json

@pytypes
def main(tag,filter):
	name = "pendingChangesets"
	jsn = dutil.jsn(name,tag)

	try:
		filter = Filter(filter,1)
		where, values = filter.getWhere()
		select = "SELECT id2changeset(metadata.id),metadata.author,metadata.status,num2revision(id2num(metadata.parentRevision)),metadata.timestamp,metadata.message FROM changeset AS metadata " + filter.getJoin("metadata") + where + " ORDER BY metadata.id"
	except dutil.DutilException as err:
		return err.json(name,jsn)

	try:
		colnames,data = dutil.getdata(select,*values)
	except dutil.DeskaException as err:
		return err.json(name,jsn)

	res = list()
	for line in data:
		ver = dict()
		ver["changeset"] = mystr(line[0])
		ver["author"] = mystr(line[1])
		ver["status"] = mystr(line[2])
		ver["parentRevision"] = mystr(line[3])
		ver["timestamp"] = mystr(line[4])
		ver["message"] = mystr(line[5])
		res.append(ver)
	jsn[name] = res
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.listRevisions(tag text, filter text = NULL)
RETURNS text
AS
$$
import dutil
from dutil import mystr
import json

@pytypes
def main(tag,filter):
	name = "listRevisions"
	jsn = dutil.jsn(name,tag)

	try:
		filter = dutil.Filter(filter,1)
		where, values = filter.getWhere()
		select = "SELECT num2revision(metadata.num),metadata.author,metadata.timestamp,metadata.message FROM version AS metadata " + filter.getJoin("metadata") + where + " ORDER BY metadata.num"
	except dutil.DutilException as err:
		return err.json(name,jsn)
	
	try:
		colnames,data = dutil.getdata(select,*values)
	except dutil.DeskaException as err:
		return err.json(name,jsn)

	res = list()
	for line in data:
		ver = dict()
		ver["revision"] = mystr(line[0])
		ver["author"] = mystr(line[1])
		ver["timestamp"] = mystr(line[2])
		ver["commitMessage"] = mystr(line[3])
		res.append(ver)

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
from dutil import oneKindDiff

@pytypes
def main(tag,a,b):
	name = "dataDifference"
	jsn = dutil.jsn(name,tag)

	res = list()
	try:
		for kindName in dutil.generated.kinds():
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
from dutil import oneKindDiff

@pytypes
def main(tag):
	name = "dataDifferenceInTemporaryChangeset"
	jsn = dutil.jsn(name,tag)

	res = list()
	try:
		for kindName in dutil.generated.kinds():
			res.extend(oneKindDiff(kindName))
	except Postgres.Exception as dberr:
		err = dutil.DeskaException(dberr)
		return err.json(name,jsn)

	jsn[name] = res
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.resolvedDataDifference(tag text, a text, b text)
RETURNS text
AS
$$
import Postgres
import json
import dutil
from dutil import oneKindDiff

@pytypes
def main(tag,a,b):
	name = "resolvedDataDifference"
	jsn = dutil.jsn(name,tag)

	res = list()
	try:
		for kindName in dutil.generated.kinds():
			res.extend(oneResolvedKindDiff(kindName,a,b))
	except Postgres.Exception as dberr:
		err = dutil.DeskaException(dberr)
		return err.json(name,jsn)

	jsn[name] = res
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.resolvedDataDifferenceInTemporaryChangeset(tag text)
RETURNS text
AS
$$
import Postgres
import json
import dutil
from dutil import oneKindDiff

@pytypes
def main(tag):
	name = "resolvedDataDifferenceInTemporaryChangeset"
	jsn = dutil.jsn(name,tag)

	res = list()
	try:
		for kindName in dutil.generated.kinds():
			res.extend(oneResolvedKindDiff(kindName))
	except Postgres.Exception as dberr:
		err = dutil.DeskaException(dberr)
		return err.json(name,jsn)

	jsn[name] = res
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

