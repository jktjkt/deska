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
	"float4": "double",
	"text": "string",
	"identifier": "identifier",\
	"identifier_set": "identifier_set",
	"bpchar": "string",
	"date": "date",
	"timestamp": "timestamp",
	"macaddr": "macaddress",
	"ipv4": "ipv4address",
	"ipv6": "ipv6address",
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
	for (attr_name, attr_type) in atts.items():
		if attr_type in type_dict:
			atts[attr_name] = type_dict[attr_type]
		else:
			atts[att_name] = "string"

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
	def addRelation(self,reltype,kind,source):
		if kind in source:
			if (type(source[kind]) == list):
				for tbl in source[kind]:
					self.append({"relation": reltype, "target": tbl})
			else:
				self.append({"relation": reltype, "target": source[kind]})

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
	res.addRelation("REFERS_TO",kindName,dutil.generated.refs())
	res.addRelation("REFERS_TO_SET",kindName,dutil.generated.refs_set())
	
	jsn[name] = res
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

