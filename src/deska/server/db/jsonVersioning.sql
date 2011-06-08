SET search_path TO deska,api;

CREATE OR REPLACE FUNCTION jsn.startChangeset()
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main():
	name = "startChangeset"
	jsn = dict()
	jsn["response"] = name

	fname = 'api.'+ name + "()"
	try:
		ver = dutil.fcall(fname)
	except dutil.DeskaException as err:
		return err.json(name,jsn)

	jsn[name] =str(ver)
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.abortCurrentChangeset()
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main():
	name = "abortCurrentChangeset"
	jsn = dict()
	jsn["response"] = name

	fname = 'api.'+ name + "()"
	try:
		ver = dutil.fcall(fname)
	except dutil.DeskaException as err:
		return err.json(name,jsn)

	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.resumeChangeset(revision text)
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(revision):
	name = "resumeChangeset"
	jsn = dict()
	jsn["response"] = name
	jsn["changeset"] = revision

	fname = 'api.'+ name + "(text)"
	try:
		ver = dutil.fcall(fname,revision)
	except dutil.DeskaException as err:
		return err.json(name,jsn)

	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.commitChangeset(commitMessage text)
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(commitMessage):
	name = "commitChangeset"
	jsn = dict()
	jsn["response"] = name
	jsn["commitMessage"] = commitMessage

	fname = 'api.'+ name + "(text)"
	try:
		ver = dutil.fcall(fname,commitMessage)
	except dutil.DeskaException as err:
		return err.json(name,jsn)
	jsn[name] =str(ver)
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.detachFromCurrentChangeset(message text)
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(message):
	name = "detachFromCurrentChangeset"
	jsn = dict()
	jsn["response"] = name
	jsn["message"] = message

	fname = 'api.'+ name + "(text)"
	try:
		ver = dutil.fcall(fname,message)
	except dutil.DeskaException as err:
		return err.json(name,jsn)

	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.pendingChangesets(filter text = NULL)
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(filter):
	name = "pendingChangesets"
	jsn = dict()
	jsn["response"] = name

	filter = dutil.Filter(filter)
	select = "SELECT id2changeset(metadata.id),metadata.author,metadata.status,num2revision(id2num(metadata.parentRevision)),metadata.timestamp,metadata.message FROM changeset AS metadata " + filter.getJoin("metadata") + filter.getWhere()
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

CREATE OR REPLACE FUNCTION jsn.listRevisions(filter text)
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(filter):
	name = "listRevisions"
	jsn = dict()
	jsn["response"] = name

	filter = dutil.Filter(filter)
	select = "SELECT num2revision(metadata.num),metadata.author,metadata.timestamp,metadata.message FROM version AS metadata " + filter.getJoin("metadata") + filter.getWhere()
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
