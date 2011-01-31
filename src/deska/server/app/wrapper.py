#!/usr/bin/python

# import api for generated stored procedures
import db

#
# auxiliary functions ... 
#

# strings
fn3_string = "{0}_{1}_{2}"
fn_string = "{0}_{1}"

def getfn(fnType, kindName,attributeName = ""):
	if (fnType == "set") or  (fnType == "rem") :
		fname = fn3_string.format(kindName,fnType,attributeName)	
	else:
		fname = fn_string.format(kindName,fnType)
	fn = getattr(db,fname)
	return fn

# 
# functions implementing API
#
def setAttribute(kindName,objectName,attributeName,value):
	fn= getfn("set",kindName,attributeName)
	return fn(objectName, value)

def removeAttribute(kindName,objectName,attributeName,value):
	fn= getfn("rem",kindName,attributeName)
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

# pre-test ...
print setAttribute("vendor","DELL","note","this is note")
print changeObjectName("vendor","DELL","Dell")
print createObject("vendor","DELL")
print deleteObject("vendor","DELL")

