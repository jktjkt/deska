CREATE OR REPLACE FUNCTION jsn.multipleObjectData(tag text, kindName text, revision text, filter text = NULL)
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(tag,kindName,revision,filter):
	name = "multipleObjectData"
	jsn = dutil.jsn(name,tag)

	# check kind name
	if kindName not in dutil.generated.kinds():
		return dutil.errorJson(name,tag,"InvalidKindError","{0} is not valid kind.".format(kindName))
	
	atts = dutil.generated.atts(kindName)
	atts["name"] = "identifier"
	# dot the atts with kindName
	new_atts = dict()
	for att in atts:
		new_atts[att] = "{0}.{1}".format(kindName,att)
	atts = new_atts

	embed = dutil.generated.embed()
	refs = dutil.generated.refs()
	if kindName in embed:
		#FIXME: propagate delimiter constant here,or drop this argument
		coldef = "join_with_delim({ref}_get_name({kind}.{ref}, $1), {kind}.name, '->') AS name".format(ref = embed[kindName], kind = kindName)
		atts["name"] = coldef
		# delete embed attribute
		del atts[embed[kindName]]
	if kindName in refs:
		for kind in refs[kindName]:
			# FIXME: this will have to change for the tripple relations
			if dutil.generated.atts(kindName)[kind] == "identifier_set":
				#"inner_host_service_multiRef_get_set"
				coldef = "inner_{0}_{1}_multiRef_get_set({0}.uid, $1)".format(kindName, kind)
			else:
				coldef = "{0}_get_name({1},$1)".format(kind,atts[kind])
			atts[kind] = coldef
	columns = ",".join(atts.values())

	try:
		# set start to 2, $1 - version is set
		filter = dutil.Filter(filter,2)
		where, values = filter.getWhere()
		select = 'SELECT '+ columns +' FROM {0}_data_version($1) AS {0} ' + filter.getJoin(kindName) + where
		select = select.format(kindName)
	except dutil.DutilException as err:
		return err.json(name,jsn)

	try:
		revisionNumber = dutil.fcall("revision2num(text)",revision)
		args = [revisionNumber]+values
		colnames, cur = dutil.getdata(select,*args)
	except dutil.DeskaException as err:
		return err.json(name,jsn)
	
	res = dict()
	for line in cur:
		data = [dutil.mystr(x) for x in line]
		data = dict(zip(atts.keys(),data))

		#FIXME? this shoud be slower, but its in protocol spec.
		objectName = data['name']
		del data['name']
		res[objectName] = data


	jsn[name] = res
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.multipleResolvedObjectData(tag text, kindName text, revision text, filter text = NULL)
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(tag,kindName,revision,filter):
	name = "multipleResolvedObjectData"
	jsn = dutil.jsn(name,tag)

	# check kind name
	if kindName not in dutil.generated.kinds():
		return dutil.errorJson(name,tag,"InvalidKindError","{0} is not valid kind.".format(kindName))
	
	atts = dutil.generated.atts(kindName)
	atts["name"] = "identifier"
	# dot the atts with kindName
	new_atts = dict()
	for att in atts:
		new_atts[att] = "{0}.{1}".format(kindName,att)
	atts = new_atts

	embed = dutil.generated.embed()
	refs = dutil.generated.refs()
	if kindName in embed:
		#FIXME: propagate delimiter constant here,or drop this argument
		coldef = "join_with_delim({ref}_get_name({kind}.{ref}, $1), {kind}.name, '->') AS name".format(ref = embed[kindName], kind = kindName)
		atts["name"] = coldef
		# delete embed attribute
		del atts[embed[kindName]]
	if kindName in refs:
		for kind in refs[kindName]:
			coldef = "{0}_get_name({1},$1)".format(kind,atts[kind])
			atts[kind] = coldef
	columns = ",".join(atts.values())

	try:
		# set start to 2, $1 - version is set
		filter = dutil.Filter(filter,2)
		where, values = filter.getWhere()
		select = 'SELECT '+ columns +' FROM {0}_resolved_data($1) AS {0} ' + filter.getJoin(kindName) + where
		select = select.format(kindName)
	except dutil.DutilException as err:
		return err.json(name,jsn)

	try:
		revisionNumber = dutil.fcall("revision2num(text)",revision)
		args = [revisionNumber]+values
		colnames, cur = dutil.getdata(select,*args)
	except dutil.DeskaException as err:
		return err.json(name,jsn)
	
	res = dict()
	for line in cur:
		data = [dutil.mystr(x) for x in line]
		data = dict(zip(atts.keys(),data))

		#FIXME? this shoud be slower, but its in protocol spec.
		objectName = data['name']
		del data['name']
		res[objectName] = data


	jsn[name] = res
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.multipleResolvedObjectDataWithOrigin(tag text, kindName text, revision text, filter text = NULL)
RETURNS text
AS
$$
import dutil
import json
import re

@pytypes
def main(tag,kindName,revision,filter):
	name = "multipleResolvedObjectDataWithOrigin"
	jsn = dutil.jsn(name,tag)

	# check kind name
	if kindName not in dutil.generated.kinds():
		return dutil.errorJson(name,tag,"InvalidKindError","{0} is not valid kind.".format(kindName))
	
	attributes = dutil.generated.atts(kindName)
	atts = dict()
	for att in attributes:
		if not re.match("template_",att):
			atts[att+"_templ"] = attributes[att]
		atts[att] = attributes[att]
	atts["name"] = "identifier"
	# dot the atts with kindName
	new_atts = dict()
	for att in atts:
		new_atts[att] = "{0}.{1}".format(kindName,att)
	atts = new_atts

	embed = dutil.generated.embed()
	refs = dutil.generated.refs()
	if kindName in embed:
		#FIXME: propagate delimiter constant here,or drop this argument
		coldef = "join_with_delim({ref}_get_name({kind}.{ref}, $1), {kind}.name, '->') AS name".format(ref = embed[kindName], kind = kindName)
		atts["name"] = coldef
		# delete embed attribute
		del atts[embed[kindName]]
	if kindName in refs:
		for kind in refs[kindName]:
			coldef = "{0}_get_name({1},$1)".format(kind,atts[kind])
			atts[kind] = coldef
	columns = ",".join(atts.values())

	try:
		# set start to 2, $1 - version is set
		filter = dutil.Filter(filter,2)
		where, values = filter.getWhere()
		select = 'SELECT '+ columns +' FROM {0}_resolved_data_template_info($1) AS {0} ' + filter.getJoin(kindName) + where
		select = select.format(kindName)
	except dutil.DutilException as err:
		return err.json(name,jsn)

	try:
		revisionNumber = dutil.fcall("revision2num(text)",revision)
		args = [revisionNumber]+values
		colnames, cur = dutil.getdata(select,*args)
	except dutil.DeskaException as err:
		return err.json(name,jsn)
	
	res = dict()
	for line in cur:
		data = [dutil.mystr(x) for x in line]
		data = dict(zip(atts.keys(),data))

		#FIXME? this shoud be slower, but its in protocol spec.
		objectName = data['name']
		del data['name']
		data = dutil.collectOriginColumns(data)
		res[objectName] = data


	jsn[name] = res
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;
