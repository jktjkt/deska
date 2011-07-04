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
	"identifier": "identifier",
	"bpchar": "string",
	"date": "string",
	"macaddr": "string",
	"ipv4": "string",
	"ipv6": "string",
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

class RelationList(list):
	def addRelation(self,type,kind,source):
		if kind in source:
			self.append({"relation": type, "target": source[kind]})

@pytypes
def main(tag,kindName):
	name = "kindRelations"
	jsn = dutil.jsn(name,tag)

	# check kind name
	if kindName not in dutil.generated.kinds():
		return dutil.errorJson(name,tag,"InvalidKindError","{0} is not valid kind.".format(kindName))

	res = RelationList()
	res.addRelation("EMBED_INTO",kindName,dutil.generated.embed())
	res.addRelation("MERGE_WITH",kindName,dutil.generated.merge())
	templates = dutil.generated.template()
	# revert dict
	revtemplates = {v:k for k, v in templates.items()}
	res.addRelation("TEMPLATIZED",kindName,templates)
	res.addRelation("IS_TEMPLATE",kindName,revtemplates)
	res.addRelation("REFERS_TO",kindName,dutil.generated.refs())
	
	jsn[name] = res
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

