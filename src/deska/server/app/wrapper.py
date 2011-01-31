#!/usr/bin/python
import db

#
# wrapper for
# 
# /** @short Set an attribute that belongs to some object to a new value */
# virtual void setAttribute( const Identifier &kindName, const Identifier &objectName,
# const Identifier &attributeName, const Value &value ) = 0;

# strings
set_string = "{0}_set_{1}"
fn_string = "{0}_{1}"

def getfn(fnType, kindName,attributeName = ""):
	if fnType == "set":
		fname = set_string.format(kindName,attributeName)	
	else:
		fname = fn_string.format(kindName,fnType)
	fn = getattr(db,fname)
	return fn

def setAttribute(kindName,objectName,attributeName,value):
	fn= getfn("set",kindName,attributeName)
	return fn(objectName, value)

def changeObjectName(kindName,oldName,newName):
	fn = getfn("set",kindName,"name")
	return fn(oldName, newName)

def createObject(kindName,objectName):
	fn = getfn("add",kindName)
	return fn(objectName)

def deleteObject(kindName,objectName):
	fn = getfn("del",kindName)
	return fn(objectName)

print setAttribute("vendor","DELL","note","this is note")
print changeObjectName("vendor","DELL","Dell")
print createObject("vendor","DELL")
print deleteObject("vendor","DELL")

