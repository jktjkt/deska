
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
		columns = "join_with_delim({ref}_get_name({ref}, $1), name, '->')".format(ref = embed[kindName])
	else:
		columns = "{0}.name"
	select = 'SELECT '+ columns +' FROM {0}_data_version($1) AS {0} '
	select = select.format(kindName)

	try:
		revisionNumber = dutil.fcall("revision2num(text)",revision)
		colnames, cur = dutil.getdata(select,revisionNumber)
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
