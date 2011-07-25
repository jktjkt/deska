
CREATE OR REPLACE FUNCTION jsn.kindInstances(tag text, kindName text, revision text, filter text = NULL)
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(tag,kindName,revision,filter):
	name = "kindInstances"
	jsn = dutil.jsn(name,tag)

	# check kind name
	if kindName not in dutil.generated.kinds():
		return dutil.errorJson(name,tag,"InvalidKindError","{0} is not valid kind.".format(kindName))
	
	embed = dutil.generated.embed()
	if kindName in embed:
		#FIXME: propagate delimiter constant here,or drop this argument
		columns = "join_with_delim({ref}_get_name({kind}.{ref}, $1), {kind}.name, '->')".format(ref = embed[kindName], kind = kindName)
	else:
		columns = "{0}.name"

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
	
	res = list()
	for line in cur:
		res.append(dutil.mystr(line[0]))

	jsn[name] = res
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.objectData(tag text, kindName text, objectName text, revision text)
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(tag,kindName,objectName,revision):
	name = "objectData"
	jsn = dutil.jsn(name,tag)

	# check kind name
	if kindName not in dutil.generated.kinds():
		return dutil.errorJson(name,tag,"InvalidKindError","{0} is not valid kind.".format(kindName))

	select = "SELECT * FROM {0}_get_data($1,$2)".format(kindName)
	try:
		revisionNumber = dutil.fcall("revision2num(text)",revision)
		colnames, data = dutil.getdata(select,objectName,revisionNumber)
	except dutil.DeskaException as dberr:
		return dberr.json(name,jsn)

	data = [dutil.mystr(x) for x in data[0]]
	res = dict(zip(colnames,data))
	jsn[name] = res
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

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
		data = dict(zip(colnames,data))

		#FIXME? this shoud be slower, but its in protocol spec.
		objectName = data['name']
		del data['name']
		res[objectName] = data


	jsn[name] = res
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

