
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
	
	embed = dutil.generated.embedNames()
	if kindName in embed:
		#FIXME: propagate delimiter constant here,or drop this argument
		refTbl = dutil.generated.relToTbl(embed[kindName])
		refCol = dutil.generated.relFromCol(embed[kindName])
		columns = "join_with_delim({ref}_get_name({kind}.{col}, $1), {kind}.name, '->')".format(ref = refTbl, kind = kindName, col = refCol)
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

CREATE OR REPLACE FUNCTION jsn.resolvedObjectData(tag text, kindName text, objectName text, revision text)
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(tag,kindName,objectName,revision):
	name = "resolvedObjectData"
	jsn = dutil.jsn(name,tag)

	# check kind name
	if kindName not in dutil.generated.kinds():
		return dutil.errorJson(name,tag,"InvalidKindError","{0} is not valid kind.".format(kindName))

	select = "SELECT * FROM {0}_resolved_object_data($1,$2)".format(kindName)
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

CREATE OR REPLACE FUNCTION jsn.resolvedObjectDataWithOrigin(tag text, kindName text, objectName text, revision text)
RETURNS text
AS
$$
import dutil
import json
import re

@pytypes
def main(tag,kindName,objectName,revision):
	name = "resolvedObjectDataWithOrigin"
	jsn = dutil.jsn(name,tag)

	# check kind name
	if kindName not in dutil.generated.kinds():
		return dutil.errorJson(name,tag,"InvalidKindError","{0} is not valid kind.".format(kindName))

	select = "SELECT * FROM {0}_resolved_object_data_template_info($1,$2)".format(kindName)
	try:
		revisionNumber = dutil.fcall("revision2num(text)",revision)
		colnames, data = dutil.getdata(select,objectName,revisionNumber)
	except dutil.DeskaException as dberr:
		return dberr.json(name,jsn)

	data = [dutil.mystr(x) for x in data[0]]
	res = dict(zip(colnames,data))
	res = dutil.collectOriginColumns(res)
	jsn[name] = res
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

