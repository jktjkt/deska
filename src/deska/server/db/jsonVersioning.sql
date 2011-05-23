SET search_path TO deska,api;

CREATE OR REPLACE FUNCTION jsn.startChangeset()
RETURNS text
AS
$$
import Postgres
import json

@pytypes
def main():
	name = "startChangeset"
	fname = 'api.'+ name + "()"
	func = proc(fname)
	ver = func()

	jsn = dict()
	jsn["response"] = name
	jsn[name] =str(ver)
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.abortCurrentChangeset()
RETURNS text
AS
$$
import Postgres
import json

@pytypes
def main():
	name = "abortCurrentChangeset"
	fname = 'api.'+ name + "()"
	func = proc(fname)
	ver = func()

	jsn = dict()
	jsn["response"] = name
	jsn[name] =str(ver)
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.resumeChangeset(id text)
RETURNS text
AS
$$
import Postgres
import json

@pytypes
def main(id):
	name = "resumeChangeset"
	fname = 'api.'+ name + "(text)"
	func = proc(fname)
	ver = func(id)

	jsn = dict()
	jsn["response"] = name
	jsn["id"] = id
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.commitChangeset(commitMessage text)
RETURNS text
AS
$$
import Postgres
import json

@pytypes
def main(commitMessage):
	name = "commitChangeset"
	fname = 'api.'+ name + "(text)"
	func = proc(fname)
	ver = func(commitMessage)

	jsn = dict()
	jsn["response"] = name
	jsn[name] =str(ver)
	jsn["commitMessage"] = commitMessage
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.detachFromCurrentChangeset(message text)
RETURNS text
AS
$$
import Postgres
import json

@pytypes
def main(message):
	name = "detachFromCurrentChangeset"
	fname = 'api.'+ name + "(text)"
	func = proc(fname)
	ver = func(message)

	jsn = dict()
	jsn["response"] = name
	jsn["message"] = message
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.pendingChangesets()
RETURNS text
AS
$$
import Postgres
import json

@pytypes
def main():
	name = "pendingChangesets"
	plan = prepare("SELECT id2changeset(id),author,status,num2revision(id2num(parentRevision)),timestamp,message FROM changeset")
	a = plan()

	res = list()
	for line in a:
		ver = dict()
		ver["changeset"] = str(line[0])
		ver["author"] = str(line[1])
		ver["status"] = str(line[2])
		ver["parentRevision"] = str(line[3])
		ver["timestamp"] = str(line[4])
		ver["message"] = str(line[5])
		res.append(ver)

	jsn = dict()
	jsn["response"] = name
	jsn[name] = res
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.listRevisions()
RETURNS text
AS
$$
import Postgres
import json

@pytypes
def main():
	name = "listRevisions"
	plan = prepare("SELECT num2revision(id),author,timestamp,message FROM version")
	a = plan()

	res = list()
	for line in a:
		ver = dict()
		ver["version"] = str(line[0])
		ver["author"] = str(line[1])
		ver["timestamp"] = str(line[2])
		ver["message"] = str(line[3])
		res.append(ver)

	jsn = dict()
	jsn["response"] = name
	jsn[name] = res
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;
