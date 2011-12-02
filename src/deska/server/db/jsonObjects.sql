SET search_path TO api,deska;

CREATE OR REPLACE FUNCTION jsn.setAttribute(tag text, kindName text, objectName text, attributeName text, attributeData text)
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(tag,kindName,objectName,attributeName,attributeData):
	name = "setAttribute"
	jsn = dutil.jsn(name,tag)

	# check kind name
	if kindName not in dutil.generated.kinds():
		return dutil.errorJson(name,tag,"InvalidKindError","{0} is not valid kind.".format(kindName))
	# check attribute 
	if attributeName not in dutil.generated.atts(kindName):
		return dutil.errorJson(name,tag,"InvalidAttributeError","{0} has no attribute {1}.".format(kindName,attributeName))

	fname = kindName+"_set_"+attributeName+"(text,text)"
	try:
		# check attribute is not set
		if dutil.generated.atts(kindName)[attributeName] == "identifier_set":
			raise dutil.DutilException("NotASetError","Attribute '{0}' type is identifier_set, but data type is not.".format(attributeName))
		dutil.fcall(fname,objectName,attributeData)
	except dutil.DeskaException as err:
		return err.json(name,jsn)

	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.setAttribute(tag text, kindName text, objectName text, attributeName text, attributeData text[])
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(tag,kindName,objectName,attributeName,attributeData):
	name = "setAttribute"
	jsn = dutil.jsn(name,tag)

	# check kind name
	if kindName not in dutil.generated.kinds():
		return dutil.errorJson(name,tag,"InvalidKindError","{0} is not valid kind.".format(kindName))
	# check attribute 
	if attributeName not in dutil.generated.atts(kindName):
		return dutil.errorJson(name,tag,"InvalidAttributeError","{0} has no attribute {1}.".format(kindName,attributeName))

	fname = kindName+"_set_"+attributeName+"(text,text[])"
	try:
		# check attribute is set
		if dutil.generated.atts(kindName)[attributeName] != "identifier_set":
			raise dutil.DutilException("NotASetError","Attribute '{0}' type is not identifier_set, but data type is.".format(attributeName))
		dutil.fcall(fname,objectName,attributeData)
	except dutil.DeskaException as err:
		return err.json(name,jsn)

	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

-- dummy function for proper returning NotASetError
CREATE OR REPLACE FUNCTION jsn.setAttributeInsert(tag text, kindName text, objectName text, attributeName text, attributeData text[])
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(tag,kindName,objectName,attributeName,attributeData):
	name = "setAttributeInsert"
	jsn = dutil.jsn(name,tag)
	try:
		raise dutil.DutilException("NotASetError","Attribute '{0}' type is not identifier_set.".format(attributeName))
	except dutil.DeskaException as err:
		return err.json(name,jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.setAttributeInsert(tag text, kindName text, objectName text, attributeName text, attributeData text)
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(tag,kindName,objectName,attributeName,attributeData):
	name = "setAttributeInsert"
	jsn = dutil.jsn(name,tag)

	# check kind name
	if kindName not in dutil.generated.kinds():
		return dutil.errorJson(name,tag,"InvalidKindError","{0} is not valid kind.".format(kindName))
	# check attribute 
	if attributeName not in dutil.generated.atts(kindName):
		return dutil.errorJson(name,tag,"InvalidAttributeError","{0} has no attribute {1}.".format(kindName,attributeName))

	fname = kindName+"_set_"+attributeName+"_insert(identifier,identifier)"
	try:
		# check attribute is set
		if dutil.generated.atts(kindName)[attributeName] != "identifier_set":
			raise dutil.DutilException("NotASetError","Attribute '{0}' type is not identifier_set.".format(attributeName))
		dutil.fcall(fname,objectName,attributeData)
	except dutil.DeskaException as err:
		return err.json(name,jsn)

	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

-- dummy function for proper returning NotASetError
CREATE OR REPLACE FUNCTION jsn.setAttributeRemove(tag text, kindName text, objectName text, attributeName text, attributeData text[])
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(tag,kindName,objectName,attributeName,attributeData):
	name = "setAttributeRemove"
	jsn = dutil.jsn(name,tag)
	try:
		raise dutil.DutilException("NotASetError","Attribute '{0}' type is not identifier_set.".format(attributeName))
	except dutil.DeskaException as err:
		return err.json(name,jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.setAttributeRemove(tag text, kindName text, objectName text, attributeName text, attributeData text)
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(tag,kindName,objectName,attributeName,attributeData):
	name = "setAttributeRemove"
	jsn = dutil.jsn(name,tag)

	# check kind name
	if kindName not in dutil.generated.kinds():
		return dutil.errorJson(name,tag,"InvalidKindError","{0} is not valid kind.".format(kindName))
	# check attribute 
	if attributeName not in dutil.generated.atts(kindName):
		return dutil.errorJson(name,tag,"InvalidAttributeError","{0} has no attribute {1}.".format(kindName,attributeName))

	fname = kindName+"_set_"+attributeName+"_remove(identifier,identifier)"
	try:
		# check attribute is set
		if dutil.generated.atts(kindName)[attributeName] != "identifier_set":
			raise dutil.DutilException("NotASetError","Attribute '{0}' type is not identifier_set.".format(attributeName))
		dutil.fcall(fname,objectName,attributeData)
	except dutil.DeskaException as err:
		return err.json(name,jsn)

	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.renameObject(tag text, kindName text, oldName text, newName text)
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(tag,kindName,oldName,newName):
	name = "renameObject"
	jsn = dutil.jsn(name,tag)

	# check kind name
	if kindName not in dutil.generated.kinds():
		return dutil.errorJson(name,tag,"InvalidKindError","{0} is not valid kind.".format(kindName))

	fname = kindName+"_set_name(text,text)"
	try:
		dutil.fcall(fname,oldName,newName)
	except dutil.DeskaException as err:
		return err.json(name,jsn)

	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.createObject(tag text, kindName text, objectName text)
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(tag,kindName,objectName):
	name = "createObject"
	jsn = dutil.jsn(name,tag)

	# check kind name
	if kindName not in dutil.generated.kinds():
		return dutil.errorJson(name,tag,"InvalidKindError","{0} is not valid kind.".format(kindName))

	fname = kindName+"_add(text)"
	try:
		assignedName = dutil.fcall(fname,objectName)
	except dutil.DeskaException as err:
		return err.json(name,jsn)

	jsn[name] = str(assignedName)
	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.deleteObject(tag text, kindName text, objectName text)
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(tag,kindName,objectName):
	name = "deleteObject"
	jsn = dutil.jsn(name,tag)

	# check kind name
	if kindName not in dutil.generated.kinds():
		return dutil.errorJson(name,tag,"InvalidKindError","{0} is not valid kind.".format(kindName))

	fname = kindName+"_del(text)"
	try:
		dutil.fcall(fname,objectName)
	except dutil.DeskaException as err:
		return err.json(name, jsn)

	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

CREATE OR REPLACE FUNCTION jsn.restoreDeletedObject(tag text, kindName text, objectName text)
RETURNS text
AS
$$
import dutil
import json

@pytypes
def main(tag,kindName,objectName):
	name = "restoreDeletedObject"
	jsn = dutil.jsn(name,tag)

	# check kind name
	if kindName not in dutil.generated.kinds():
		return dutil.errorJson(name,tag,"InvalidKindError","{0} is not valid kind.".format(kindName))

	fname = kindName+"_undel(text)"
	try:
		dutil.fcall(fname,objectName)
	except dutil.DeskaException as err:
		return err.json(name, jsn)

	return json.dumps(jsn)
$$
LANGUAGE python SECURITY DEFINER;

