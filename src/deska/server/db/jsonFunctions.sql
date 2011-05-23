SET search_path TO deska,api;

CREATE OR REPLACE FUNCTION jsn.kindNames()
RETURNS text
AS
$$
import Postgres
import json

@pytypes
def main():
	name = "kindNames"
	plan = prepare('SELECT * FROM api.kindNames()')
	cur = plan()
	
	res = list()
	for line in cur:
		res.append(str(line[0]))

	jsn = dict()
	jsn["response"] = name
	jsn[name] = res
	jsn["response"] = "kindNames"
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.kindAttributes(kindName text)
RETURNS text
AS
$$
import Postgres
import json

type_dict = ({
	"int8": "int",
	"int4": "int",
	"text": "string",
	"bpchar": "string",
	"date": "string",
	"macaddr": "string",
	"inet": "string"
})

@pytypes
def main(kindName):
	name = "kindAttributes"
	plan = prepare('SELECT * FROM api.kindAttributes($1)')
	cur = plan(kindName)
	
	res = dict()
	for line in cur:
		res[str(line[0])] = type_dict[str(line[1])]

	jsn = dict()
	jsn["response"] = name
	jsn[name] = res
	jsn["kindName"] = kindName
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.kindRelations(kindName text)
RETURNS text
AS
$$
import Postgres
import json

@pytypes
def main(kindName):
	name = "kindRelations"
	plan = prepare('SELECT * FROM api.kindRelations($1)')
	cur = plan(kindName)
	
	res = list()
	for line in cur:
		rel = dict()
		# FIXME: hotfix until rewrite relations get functions
		rel["relation"] = "EMBED_INTO"
		rel["into"] = str(line[1])
		res.append(rel)

	jsn = dict()
	jsn["response"] = name
	jsn[name] = res
	jsn["kindName"] = kindName
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.kindInstances(kindName text)
RETURNS text
AS
$$
import Postgres
import json

@pytypes
def main(kindName):
	name = "kindInstances"
	plan = prepare('SELECT * FROM api.kindInstances($1)')
	cur = plan(kindName)
	
	res = list()
	for line in cur:
		res.append(str(line[0]))

	jsn = dict()
	jsn["response"] = name
	jsn[name] = res
	jsn["kindName"] = kindName
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

