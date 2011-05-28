SET search_path TO deska,api;

CREATE OR REPLACE FUNCTION jsn.kindNames()
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main():
	name = "kindNames"
	select = 'SELECT * FROM api.kindNames()'
	try:
		colnames, cur = dutil.getdata(select)
	except dutil.DeskaException as err:
		return err.json(name)
	
	res = list()
	for line in cur:
		res.append(str(line[0]))

	jsn = dict()
	jsn["response"] = name
	jsn[name] = res
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.kindAttributes(kindName text)
RETURNS text
AS
$$
import dutil
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
	select = 'SELECT * FROM api.kindAttributes($1)'
	try:
		colnames, cur = dutil.getdata(select,kindName)
	except dutil.DeskaException as err:
		return err.json(name)
	
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
import dutil
import json

@pytypes
def main(kindName):
	name = "kindRelations"
	select = 'SELECT * FROM api.kindRelations($1)'
	try:
		colnames, cur = dutil.getdata(select,kindName)
	except dutil.DeskaException as err:
		return err.json(name)
	
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
import dutil
import json

@pytypes
def main(kindName):
	name = "kindInstances"
	select = 'SELECT * FROM api.kindInstances($1)'
	try:
		colnames, cur = dutil.getdata(select,kindName)
	except dutil.DeskaException as err:
		return err.json(name)
	
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

