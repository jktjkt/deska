
CREATE OR REPLACE FUNCTION jsn.kindInstances(tag text, kindName text, revision text, filter text = NULL)
RETURNS text
AS
$$
import dutil
from filter import Filter
import json

@pytypes
def main(tag,kindName,revision,filter):
	name = "kindInstances"
	jsn = dutil.jsn(name,tag)

	# check kind name
	if kindName not in dutil.generated.kinds():
		return dutil.errorJson(name,tag,"InvalidKindError","{0} is not valid kind.".format(kindName))
	
	embed = dutil.generated.embedNames()
	if kindName in embed.values():
		'''We need this for else branch'''
		for relName in embed:
			if embed[relName] == kindName:
				'''Here we hope that there is only one of these ->FIXME'''
				#FIXME: propagate delimiter constant here,or drop this argument
				refTbl = dutil.generated.relToTbl(relName)
				refCol = dutil.generated.relFromCol(relName)
				columns = "join_with_delim({ref}_get_name({kind}.{col}, $1), {kind}.name, '->') AS name".format(ref = refTbl, kind = kindName, col = refCol)
	else:
		columns = "{0}.name".format(kindName)

	try:
		# set start to 2, $1 - version is set
		filter = Filter(filter,2)
		where, values = filter.getWhere()
		select = dutil.getSelect(kindName, name, columns, filter.getJoin(kindName), where)
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
		res.append(str(line[0]))

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

	atts, specialTypeCols = dutil.getAtts(dutil.generated.atts(kindName),kindName)
	embed = dutil.generated.embedNames()
	for relName in embed:
		if embed[relName] == kindName:
			del atts[dutil.generated.relFromCol(relName)]
	cols = ",".join(atts.values())

	select = dutil.getSelect(kindName, name, cols)
	try:
		revisionNumber = dutil.fcall("revision2num(text)",revision)
		colnames, data = dutil.getdata(select,objectName,revisionNumber)
	except dutil.DeskaException as dberr:
		return dberr.json(name,jsn)

	if dutil.generated.atts(kindName) == {}:
		res = {}
	else:
		specialTypeCols = dutil.getColumnIndexes(colnames,specialTypeCols)
		data = dutil.pytypes(data[0],specialTypeCols)
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
	
	atts, specialTypeCols = dutil.getAtts(dutil.generated.atts(kindName),kindName)
	embed = dutil.generated.embedNames()
	for relName in embed:
		if embed[relName] == kindName:
			del atts[dutil.generated.relFromCol(relName)]
	cols = ",".join(atts.values())

	select = dutil.getSelect(kindName, name, cols)
	try:
		revisionNumber = dutil.fcall("revision2num(text)",revision)
		colnames, data = dutil.getdata(select,objectName,revisionNumber)
	except dutil.DeskaException as dberr:
		return dberr.json(name,jsn)

	if dutil.generated.atts(kindName) == {}:
		res = {}
	else:
		specialTypeCols = dutil.getColumnIndexes(colnames,specialTypeCols)
		data = dutil.pytypes(data[0],specialTypeCols)
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

	attributes = dutil.generated.atts(kindName)

	embed = dutil.generated.embedNames()
	for relName in embed:
		if embed[relName] == kindName:
			del attributes[dutil.generated.relFromCol(relName)]
	atts = dict()
	for att in attributes:
		if dutil.hasTemplate(kindName):
			if not re.match("template_",att):
				atts[att+"_templ"] = "text"
		atts[att] = attributes[att]
	atts, specialTypeCols = dutil.getAtts(atts,kindName)
	cols = ",".join(atts.values())

	select = dutil.getSelect(kindName, name, cols)
	try:
		revisionNumber = dutil.fcall("revision2num(text)",revision)
		colnames, data = dutil.getdata(select,objectName,revisionNumber)
	except dutil.DeskaException as dberr:
		return dberr.json(name,jsn)

	if dutil.generated.atts(kindName) == {}:
		data = {}
	else:
		specialTypeCols = dutil.getColumnIndexes(colnames,specialTypeCols)
		data = dutil.pytypes(data[0],specialTypeCols)
		data = dict(zip(colnames,data))
		if dutil.hasTemplate(kindName):
			'''Only if kindName has template'''
			data = dutil.collectOriginColumns(data,objectName)
		else:
			'''Fake origin columns'''
			data = dutil.fakeOriginColumns(data,objectName)
	jsn[name] = data
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

