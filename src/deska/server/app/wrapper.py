

#
# wrapper for
# 
# /** @short Set an attribute that belongs to some object to a new value */
# virtual void setAttribute( const Identifier &kindName, const Identifier &objectName,
# const Identifier &attributeName, const Value &value ) = 0;

# strings
set_string = "{0}_set_{1}"
fn_string = "{0}_{1}"

def getfname(fnType, kindName):
	fn = fn_string.format(kindName,fnType)	
	return fn

def getfname(fnType, kindName,attributeName = ""):
	if fnType == "set":
		fn = set_string.format(kindName,attributeName)	
	else:
		# maybe we dont need this
		fn = fn_string.format(kindName,fnType)
	return fn

def vendor_add(objectName):
	return "vendor_add({0})".format(objectName)

def vendor_del(objectName):
	return "vendor_del({0})".format(objectName)

def vendor_set_note(objectName, value):
	return "vendor_set_note({0},{1})".format(objectName, value)

def vendor_set_name(objectName, value):
	return "vendor_set_name({0},{1})".format(objectName, value)

def setAttribute(kindName,objectName,attributeName,value):
	fname = getfname("set",kindName,attributeName)
	fn = globals()[fname]
	return fn(objectName, value)

def changeObjectName(kindName,oldName,newName):
	fname = getfname("set",kindName,"name")
	fn = globals()[fname]
	return fn(oldName, newName)

def createObject(kindName,objectName):
	fname = getfname("add",kindName)
	fn = globals()[fname]
	return fn(objectName)

def deleteObject(kindName,objectName):
	fname = getfname("del",kindName)
	fn = globals()[fname]
	return fn(objectName)

print setAttribute("vendor","DELL","note","this is note")
print changeObjectName("vendor","DELL","Dell")
print createObject("vendor","DELL")
print deleteObject("vendor","DELL")
