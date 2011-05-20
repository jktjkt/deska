SET search_path TO deska,api,genproc,history,versioning,production;
DROP SCHEMA jsn CASCADE;
CREATE SCHEMA jsn;

CREATE OR REPLACE FUNCTION jsn.listVersions()
RETURNS text
AS
$$
import Postgres
import json

@pytypes
def main():
	plan = prepare("SELECT * FROM api.listVersions()")
	a = plan()

	slist = list()
	for line in a:
		res = dict()
		res["author"] = str(line["author"])
		res["timestamp"] = str(line["timestamp"])
		res["message"] = str(line["message"])
		res["version"] = str(line["version"])
		slist.append(res)

	jsn = dict()
	jsn["response"] = "listVersions"
	jsn["listVersions"] = slist
	return json.dumps(jsn)
$$
LANGUAGE python;

CREATE OR REPLACE FUNCTION jsn.kindInstances(kindName text)
RETURNS text
AS
$$
import Postgres
import json

@pytypes
def main(kindName):
	plan = prepare('SELECT * FROM api.kindinstances($1)')
	cur = plan(kindName)
	
	res = list()
	for line in cur:
		res.append(str(line[0]))

	jsn = dict()
	jsn["response"] = "kindInstances"
	jsn["kindName"] = kindName
	jsn["kindInstances"] = res
	return json.dumps(jsn)
$$
LANGUAGE python;

