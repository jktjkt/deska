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

	jsn[name] =str(ver)
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
	jsn[name] =str(ver)
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
import json

@pytypes
def main(tag,filter):
	name = "pendingChangesets"
	jsn = dutil.jsn(name,tag)

	filter = dutil.Filter(filter)
	select = "SELECT id2changeset(metadata.id),metadata.author,metadata.status,num2revision(id2num(metadata.parentRevision)),metadata.timestamp,metadata.message FROM changeset AS metadata " + filter.getJoin("metadata") + filter.getWhere() + "ORDER BY metadata.id"
	try:
		colnames,data = dutil.getdata(select)
	except dutil.DeskaException as err:
		return err.json(name,jsn)

	res = list()
	for line in data:
		ver = dict()
		ver["changeset"] = str(line[0])
		ver["author"] = str(line[1])
		ver["status"] = str(line[2])
		ver["parentRevision"] = str(line[3])
		ver["timestamp"] = str(line[4])
		ver["message"] = str(line[5])
		res.append(ver)
	jsn[name] = res
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.listRevisions(tag text, filter text)
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(tag,filter):
	name = "listRevisions"
	jsn = dutil.jsn(name,tag)

	filter = dutil.Filter(filter)
	select = "SELECT num2revision(metadata.num),metadata.author,metadata.timestamp,metadata.message FROM version AS metadata " + filter.getJoin("metadata") + filter.getWhere() + "ORDER BY metadata.num"
	try:
		colnames,data = dutil.getdata(select)
	except dutil.DeskaException as err:
		return err.json(name,jsn)

	res = list()
	for line in data:
		ver = dict()
		ver["version"] = str(line[0])
		ver["author"] = str(line[1])
		ver["timestamp"] = str(line[2])
		ver["message"] = str(line[3])
		res.append(ver)

	jsn[name] = res
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;
