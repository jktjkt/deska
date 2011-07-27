import json
import unittest
import sys
import os

class DeskaRunner():
	def __init__(self):
		runstr = "python %s -d %s" % (sys.argv[1], os.environ["DESKA_DB"])
		self.stdin, self.stdout = os.popen2(runstr)

	def command(self,cmd):
		self.stdin.write(cmd)
		self.stdin.write("\n")
		self.stdin.flush()
		return self.stdout.readline()

class JsonBuilder():
	def __init__(self):
		return

	def command(self,name,**args):
		ret = {"command": name}
		for key in args:
			ret[key] = args[key]
		return json.dumps(ret)

	def startChangeset(self):
		return self.command("startChangeset")

	def commitChangeset(self,message):
		return self.command("commitChangeset",**{"commitMessage": message})

	def detachFromCurrentChangeset(self,message):
		return self.command("detachFromCurrentChangeset",**{"message": message})

	def abortCurrentChangeset(self):
		return self.command("abortCurrentChangeset")

	def resumeChangeset(self,chid):
		return self.command("resumeChangeset",**{"changeset": chid})

	def listRevisions(self,filter = None):
		if filter is None:
			return self.command("listRevisions")
		else:
			return self.command("listRevisions",**{"filter": filter})

	def pendingChangesets(self,filter = None):
		if filter is None:
			return self.command("pendingChangesets")
		else:
			return self.command("pendingChangesets",**{"filter": filter})

	def kindNames(self):
		return self.command("kindNames")

	def kindAttributes(self,kind):
		return self.command("kindAttributes",**{"kindName": kind})

	def kindRelations(self,kind):
		return self.command("kindRelations",**{"kindName": kind})

	def kindInstances(self,kind,revision = None):
		if revision is None:
			return self.command("kindInstances",**{"kindName": kind})
		else:
			return self.command("kindInstances",**{"kindName": kind, "revision": revision})

	def createObject(self,kind,name):
		return self.command("createObject",**{"kindName": kind,"objectName": name})

	def deleteObject(self,kind,name):
		return self.command("deleteObject",**{"kindName": kind,"objectName": name})

	def objectData(self,kind,name,revision = None):
		if revision is None:
			return self.command("objectData",**{"kindName": kind,"objectName": name})
		else:
			return self.command("objectData",**{"kindName": kind,"objectName": name, "revision": revision})

	def setAttribute(self,kind,name,att,data):
		return self.command("setAttribute",**{"kindName": kind,"objectName": name, "attributeName": att, "attributeData":data})

	def dataDifferenceInTemporaryChangeset(self):
		return self.command("dataDifferenceInTemporaryChangeset")

	def dataDifference(self,a,b):
		return self.command("dataDifference",**{"revisionA": a,"revisionB": b})


def deunicodeify(stuff):
	"""Convert a dict or stuff like that into a dict with all strings changed into unicode"""
	# courtesy of http://stackoverflow.com/questions/1254454/fastest-way-to-convert-a-dicts-keys-values-from-unicode-to-str
	if isinstance(stuff, unicode):
		return stuff.encode("utf-8")
	elif isinstance(stuff, dict):
		return dict(map(deunicodeify, stuff.iteritems()))
	elif isinstance(stuff, (list, tuple)):
		return type(stuff)(map(deunicodeify, stuff))
	else:
		return stuff


class JsonParser():
	def __init__(self,jsn):
		self.data = deunicodeify(json.loads(jsn))

	def __contains__(self,key):
		return key in self.data

	def __getitem__(self,key):
		return str(self.data[key])

	def response(self):
		return str(self["response"])

	def command(self):
		return str(self["command"])

	def result(self):
		return self.data[self.response()]

	def error(self,errorType):
		return str(self.data["dbException"]["type"]) == errorType

	def changesetOpen(self):
		return self.error("ChangesetAlreadyOpenError")

	def noChangeset(self):
		return self.error("NoChangesetError")

	def notFound(self):
		return self.error("NotFoundError")

	def otherError(self):
		return self.error("ServerError")

	def all(self):
		return self.data

	def OK(self):
		return not "dbException" in self

def updateRev(revision,update):
	'''updates revisionID'''
	return "r%d" % (int(revision[1:len(revision)])+update)

tr = DeskaRunner()
js = JsonBuilder()
