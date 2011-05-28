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
	fname = 'api.'+ name + "()"
	try:
		ver = dutil.fcall(fname)
	except dutil.DeskaException as err:
		return err.json(name)

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
import dutil
import json

@pytypes
def main():
	name = "abortCurrentChangeset"
	fname = 'api.'+ name + "()"
	try:
		ver = dutil.fcall(fname)
	except dutil.DeskaException as err:
		return err.json(name)

	jsn = dict()
	jsn["response"] = name
#	jsn[name] =str(ver)
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
	fname = 'api.'+ name + "(text)"
	try:
		ver = dutil.fcall(fname,revision)
	except dutil.DeskaException as err:
		return err.json(name)

	jsn = dict()
	jsn["response"] = name
	jsn["revision"] = revision
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
	fname = 'api.'+ name + "(text)"
	try:
		ver = dutil.fcall(fname,commitMessage)
	except dutil.DeskaException as err:
		return err.json(name)

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
import dutil
import json

@pytypes
def main(message):
	name = "detachFromCurrentChangeset"
	fname = 'api.'+ name + "(text)"
	try:
		ver = dutil.fcall(fname,message)
	except dutil.DeskaException as err:
		return err.json(name)

	jsn = dict()
	jsn["response"] = name
	jsn["message"] = message
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.pendingChangesets(filter text)
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(filter):
	name = "pendingChangesets"
	filter = dutil.Filter(filter)
	select = "SELECT id2changeset(id),author,status,num2revision(id2num(parentRevision)),timestamp,message FROM changeset "+filter.getWhere()
	try:
		colnames,data = dutil.getdata(select)
	except dutil.DeskaException as err:
		return err.json(name)

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

	jsn = dict()
	jsn["response"] = name
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
	filter = dutil.Filter(filter)
	select = "SELECT num2revision(num),author,timestamp,message FROM version "+filter.getWhere()
	try:
		colnames,data = dutil.getdata(select)
	except dutil.DeskaException as err:
		return err.json(name)

	res = list()
	for line in data:
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
