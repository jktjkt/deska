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
	
	atts = dutil.generated.atts(kindName)
	atts["name"] = "identifier"
	# dot the atts with kindName
	new_atts = dict()
	for att in atts:
		new_atts[att] = "{0}.{1}".format(kindName,att)
	atts = new_atts

	embed = dutil.generated.embedNames()
	refs = dutil.generated.refNames()
	for relName in embed:
		if embed[relName] == kindName:
			#FIXME: propagate delimiter constant here,or drop this argument
			refTbl = dutil.generated.relToTbl(relName)
			refCol = dutil.generated.relFromCol(relName)
			coldef = "join_with_delim({ref}_get_name({kind}.{col}, $1), {kind}.name, '->') AS name".format(ref = refTbl, kind = kindName, col = refCol)
			atts["name"] = coldef
			# delete embed attribute
			del atts[refCol]
	for relName in refs:
		if refs[relName] == kindName:
			refTbl = dutil.generated.relToTbl(relName)
			refCol = dutil.generated.relFromCol(relName)
			if dutil.generated.atts(kindName)[refTbl] == "identifier_set":
				#"inner_host_service_multiRef_get_set"
				coldef = "inner_{0}_{1}_multiRef_get_set({0}.uid, $1)".format(kindName, refTbl)
			else:
				coldef = "{0}_get_name({1},$1)".format(refTbl,refCol)
			atts[refCol] = coldef

	columns = ",".join(atts.values())

	try:
		# set start to 2, $1 - version is set
		filter = Filter(filter,2)
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
from filter import Filter
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

	embed = dutil.generated.embedNames()
	refs = dutil.generated.refNames()
	for relName in embed:
		if embed[relName] == kindName:
			#FIXME: propagate delimiter constant here,or drop this argument
			refTbl = dutil.generated.relToTbl(relName)
			refCol = dutil.generated.relFromCol(relName)
			coldef = "join_with_delim({ref}_get_name({kind}.{col}, $1), {kind}.name, '->') AS name".format(ref = refTbl, kind = kindName, col = refCol)
			atts["name"] = coldef
			# delete embed attribute
			del atts[refCol]
	for relName in refs:
		if refs[relName] == kindName:
			refTbl = dutil.generated.relToTbl(relName)
			refCol = dutil.generated.relFromCol(relName)
			if dutil.generated.atts(kindName)[refTbl] != "identifier_set":
				# no action for identifier_set - getting it from data function
				coldef = "{0}_get_name({1},$1)".format(refTbl,refCol)
				atts[refCol] = coldef

	columns = ",".join(atts.values())

	try:
		# set start to 2, $1 - version is set
		filter = Filter(filter,2)
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
		if not re.match("template_",att):
			atts[att+"_templ"] = attributes[att]
		atts[att] = attributes[att]
	atts["name"] = "identifier"
	# dot the atts with kindName
	new_atts = dict()
	for att in atts:
		new_atts[att] = "{0}.{1}".format(kindName,att)
	atts = new_atts

	embed = dutil.generated.embedNames()
	refs = dutil.generated.refNames()
	for relName in embed:
		if embed[relName] == kindName:
			#FIXME: propagate delimiter constant here,or drop this argument
			refTbl = dutil.generated.relToTbl(relName)
			refCol = dutil.generated.relFromCol(relName)
			coldef = "join_with_delim({ref}_get_name({kind}.{col}, $1), {kind}.name, '->') AS name".format(ref = refTbl, kind = kindName, col = refCol)
			atts["name"] = coldef
			# delete embed attribute
			del atts[refCol]
	for relName in refs:
		if refs[relName] == kindName:
			refTbl = dutil.generated.relToTbl(relName)
			refCol = dutil.generated.relFromCol(relName)
			if dutil.generated.atts(kindName)[refTbl] != "identifier_set":
				# No action for identifier_set required, getting from data function
				coldef = "{0}_get_name({1},$1)".format(refTbl,refCol)
				atts[refCol] = coldef

	columns = ",".join(atts.values())

	try:
		# set start to 2, $1 - version is set
		filter = Filter(filter,2)
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

