CREATE OR REPLACE FUNCTION jsn.multipleObjectData(tag text, kindName text, revision text, filter text = NULL)
RETURNS text
AS
$$
import dutil
from filter import Filter
import json

@pytypes
def main(tag,kindName,revision,filter):
	name = "multipleObjectData"
	jsn = dutil.jsn(name,tag)

	# check kind name
	if kindName not in dutil.generated.kinds():
		return dutil.errorJson(name,tag,"InvalidKindError","{0} is not valid kind.".format(kindName))
	
	atts, specialTypeCols = dutil.getAtts(dutil.generated.atts(kindName),kindName,True)

	embed = dutil.generated.embedNames()
	rels = dutil.generated.refNames()
	rels.update(dutil.generated.containsNames())
	rels.update(dutil.generated.containableNames())
	rels.update(dutil.generated.templateNames())
	for relName in embed:
		if embed[relName] == kindName:
			refTbl = dutil.generated.relToTbl(relName)
			refCol = dutil.generated.relFromCol(relName)
			coldef = "join_with_delim({ref}_get_name({kind}.{col}, $1), {kind}.name, '{delim}') AS name".format(ref = refTbl, kind = kindName, col = refCol, delim = dutil.generated.DELIMITER)
			atts["name"] = coldef
			# delete embed attribute
			del atts[refCol]
	for relName in rels:
		if rels[relName] == kindName:
			refTbl = dutil.generated.relToTbl(relName)
			refCol = dutil.generated.relFromCol(relName)
			if dutil.generated.atts(kindName)[refCol] == "identifier_set":
				#"inner_host_service_get_set"
				coldef = "inner_{0}_{1}_get_set({0}.uid, $1) AS {1}".format(kindName, refCol)
			else:
				coldef = "{0}_get_name({1}.{2},$1) AS {0}".format(refTbl,kindName,refCol)
			atts[refCol] = coldef

	columns = ",".join(atts.values())

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
	
	specialTypeCols = dutil.getColumnIndexes(colnames,specialTypeCols)
	res = dict()
	for line in cur:
		data = dutil.pytypes(line,specialTypeCols)
		data = dict(zip(atts.keys(),data))

		#this shoud be slower, but its in protocol spec.
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
from filter import Filter
import json

@pytypes
def main(tag,kindName,revision,filter):
	name = "multipleResolvedObjectData"
	jsn = dutil.jsn(name,tag)

	#True if there is $1 - revision parameter
	revisionParameter = False

	# check kind name
	if kindName not in dutil.generated.kinds():
		return dutil.errorJson(name,tag,"InvalidKindError","{0} is not valid kind.".format(kindName))
	
	atts, specialTypeCols = dutil.getAtts(dutil.generated.atts(kindName),kindName,True)

	embed = dutil.generated.embedNames()
	rels = dutil.generated.refNames()
	rels.update(dutil.generated.containsNames())
	rels.update(dutil.generated.containableNames())
	rels.update(dutil.generated.templateNames())
	for relName in embed:
		if embed[relName] == kindName:
			refTbl = dutil.generated.relToTbl(relName)
			refCol = dutil.generated.relFromCol(relName)
			coldef = "join_with_delim({ref}_get_name({kind}.{col}, $1), {kind}.name, '{delim}') AS name".format(ref = refTbl, kind = kindName, col = refCol, delim = dutil.generated.DELIMITER)
			revisionParameter = True
			atts["name"] = coldef
			# delete embed attribute
			del atts[refCol]
	for relName in rels:
		if rels[relName] == kindName:
			refTbl = dutil.generated.relToTbl(relName)
			refCol = dutil.generated.relFromCol(relName)
			if dutil.generated.atts(kindName)[refCol] != "identifier_set":
				# no action for identifier_set - getting it from data function
				coldef = "{0}_get_name({1}.{2},$1) AS {0}".format(refTbl,kindName,refCol)
				revisionParameter = True
			else:
				if dutil.hasTemplate(kindName):
					coldef = "{0}_get_resolved_{1}({0}.uid, $1) AS {1}".format(kindName, refCol)
					revisionParameter = True
				else:
					coldef = "inner_{0}_{1}_get_set({0}.uid, $1) AS {1}".format(kindName, refCol)
					revisionParameter = True
			atts[refCol] = coldef

	columns = ",".join(atts.values())

	try:
		changeset = dutil.fcall("get_current_changeset_or_null()")
		if revision is None and changeset is None:
			'''Check if we are in changeset'''
			directAccess = True
		else:
			directAccess = False
		if revisionParameter:
			# set start to 2, $1 - version is set
			filter = Filter(filter,2,directAccess)
		else:
			filter = Filter(filter,1,directAccess)
		where, values = filter.getWhere()
		select = dutil.getSelect(kindName, name, columns, filter.getJoin(kindName), where, directAccess)
	except dutil.DutilException as err:
		return err.json(name,jsn)
	except dutil.DeskaException as err:
		return err.json(name,jsn)

	try:
		revisionNumber = dutil.fcall("revision2num(text)",revision)
		if not directAccess:
			args = [revisionNumber] + values
		else:
			if revisionParameter:
				args = [revisionNumber] + values
			else:
				args = values
		colnames, cur = dutil.getdata(select,*args)
	except dutil.DeskaException as err:
		return err.json(name,jsn)
	
	specialTypeCols = dutil.getColumnIndexes(colnames,specialTypeCols)
	res = dict()
	for line in cur:
		data = dutil.pytypes(line,specialTypeCols)
		data = dict(zip(atts.keys(),data))

		# this shoud be slower, but its in protocol spec.
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
from filter import Filter
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
		if dutil.hasTemplate(kindName):
			'''Only if kindName has template'''
			if not re.match("template_",att):
				atts[att+"_templ"] = "text"
		atts[att] = attributes[att]
	atts, specialTypeCols = dutil.getAtts(atts,kindName,True)

	embed = dutil.generated.embedNames()
	rels = dutil.generated.refNames()
	rels.update(dutil.generated.containsNames())
	rels.update(dutil.generated.containableNames())
	rels.update(dutil.generated.templateNames())
	for relName in embed:
		if embed[relName] == kindName:
			refTbl = dutil.generated.relToTbl(relName)
			refCol = dutil.generated.relFromCol(relName)
			coldef = "join_with_delim({ref}_get_name({kind}.{col}, $1), {kind}.name, '{delim}') AS name".format(ref = refTbl, kind = kindName, col = refCol, delim = dutil.generated.DELIMITER)
			atts["name"] = coldef
			# delete embed attribute
			del atts[refCol]
			del atts[refCol+"_templ"]
	for relName in rels:
		if rels[relName] == kindName:
			refTbl = dutil.generated.relToTbl(relName)
			refCol = dutil.generated.relFromCol(relName)
			if dutil.generated.atts(kindName)[refCol] != "identifier_set":
				# No action for identifier_set required, getting from data function
				coldef = "{0}_get_name({1}.{2},$1) AS {0}".format(refTbl,kindName,refCol)
				atts[refCol] = coldef

	columns = ",".join(atts.values())

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
	
	specialTypeCols = dutil.getColumnIndexes(colnames,specialTypeCols)
	res = dict()
	for line in cur:
		data = dutil.pytypes(line,specialTypeCols)
		data = dict(zip(atts.keys(),data))

		# this shoud be slower, but its in protocol spec.
		objectName = data['name']
		del data['name']
		if dutil.hasTemplate(kindName):
			'''Only if kindName has template'''
			data = dutil.collectOriginColumns(data,objectName)
		else:
			'''Fake origin columns'''
			data = dutil.fakeOriginColumns(data,objectName)
		res[objectName] = data


	jsn[name] = res
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

