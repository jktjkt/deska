SET search_path TO deska,api;

CREATE OR REPLACE FUNCTION jsn.kindNames(tag text)
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(tag):
	name = "kindNames"
	jsn = dutil.jsn(name,tag)

	jsn[name] = dutil.generated.kinds()
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.kindAttributes(tag text, kindName text)
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
def main(tag,kindName):
	name = "kindAttributes"
	jsn = dutil.jsn(name,tag)

	# check kind name
	if kindName not in dutil.generated.kinds():
		return dutil.errorJson(name,tag,"InvalidKindError","{0} is not valid kind.".format(kindName))

	atts = dutil.generated.atts(kindName)
	for att in atts:
		atts[att] = type_dict[atts[att]]

	jsn[name] = atts
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.kindRelations(tag text, kindName text)
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(tag,kindName):
	name = "kindRelations"
	jsn = dutil.jsn(name,tag)

	select = 'SELECT * FROM api.kindRelations($1)'
	try:
		colnames, cur = dutil.getdata(select,kindName)
	except dutil.DeskaException as err:
		return err.json(name,jsn)
	
	res = list()
	for line in cur:
		rel = dict()
		# FIXME: hotfix until rewrite relations get functions
		rel["relation"] = "EMBED_INTO"
		rel["target"] = str(line[1])
		res.append(rel)

	jsn[name] = res
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.kindInstances(tag text, kindName text, revision text)
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(tag,kindName,revision):
	name = "kindInstances"
	jsn = dutil.jsn(name,tag)

	select = 'SELECT * FROM {0}_names($1)'.format(kindName)
	try:
		revisionNumber = dutil.fcall("revision2num(text)",revision)
		colnames, cur = dutil.getdata(select,revisionNumber)
	except dutil.DeskaException as err:
		return err.json(name,jsn)
	
	res = list()
	for line in cur:
		res.append(str(line[0]))

	jsn[name] = res
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

