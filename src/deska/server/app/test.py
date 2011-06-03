import os
import json
import unittest
import sys

class DeskaRunner():
	def __init__(self):
		#runstr = "python {server} {db}".format(server=sys.argv[1], db=sys.argv[2])
		runstr = "python deska_server.py -d deska_dev"
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
		#FIXME: pass changeset is right
		return self.command("resumeChangeset",**{"revision": chid})
		return self.command("resumeChangeset",**{"changeset": chid})

	def listRevisions(self,filter = ''):
		return self.command("listRevisions",**{"filter": filter})

	def pendingChangesets(self,filter = ''):
		return self.command("pendingChangesets",**{"filter": filter})

	def kindNames(self):
		return self.command("kindNames")

	def kindAttributes(self,kind):
		return self.command("kindAttributes",**{"kindName": kind})

	def kindRelations(self,kind):
		return self.command("kindRelations",**{"kindName": kind})

	def kindInstances(self,kind):
		return self.command("kindInstances",**{"kindName": kind})

	def createObject(self,kind,name):
		return self.command("createObject",**{"kindName": kind,"objectName": name})

	def deleteObject(self,kind,name):
		return self.command("deleteObject",**{"kindName": kind,"objectName": name})

	def objectData(self,kind,name):
		return self.command("objectData",**{"kindName": kind,"objectName": name})

	def setAttribute(self,kind,name,att,data):
		return self.command("setAttribute",**{"kindName": kind,"objectName": name, "attributeName": att, "attributeData":data})

	def dataDifference(self,a,b):
		return self.command("dataDifference",**{"a": a,"b": b})


class JsonParser():
	def __init__(self,jsn):
		self.data = json.loads(jsn)

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


js = JsonBuilder()
tr = DeskaRunner()

class DeskaTest(unittest.TestCase):
	def command(self,cmd,*args):
		jsn = cmd(*args)
		cmd = JsonParser(jsn)
		res = tr.command(jsn)
		jp = JsonParser(res)
		print jsn
		print res
		self.assertEqual(jp["response"], cmd["command"])
		return jp

	def OK(self,func):
		self.assertTrue(func())

	def test_001_startChangeset(self):
		res = self.command(js.startChangeset)
		self.OK(res.OK)
		# second time we expect exception
		res = self.command(js.startChangeset)
		self.OK(res.changesetOpen)

	def test_002_commitChangeset(self):
		# FIXME: no changeset should raise exception
		res = self.command(js.commitChangeset,"message")
		self.OK(res.OK)
		# second time we expect exception
		res = self.command(js.commitChangeset,"message")
		self.OK(res.noChangeset)
		
	def test_003_resumeChangeset(self):
		# create changeset
		res = self.command(js.startChangeset)
		self.OK(res.OK)
		chid = res.result()
		# you have already assigned one
		res = self.command(js.resumeChangeset,chid)
		self.OK(res.changesetOpen)
		# detaching
		res = self.command(js.detachFromCurrentChangeset,"test")
		self.OK(res.OK)
		# bad id
		res = self.command(js.resumeChangeset,"r12")
		self.OK(res.otherError)


	def test_004_detachChangeset(self):
		# create changeset
		res = self.command(js.startChangeset)
		self.OK(res.OK)
		# detach
		res = self.command(js.detachFromCurrentChangeset,"test")
		self.OK(res.OK)
		# 2nd times it crashes
		res = self.command(js.detachFromCurrentChangeset,"test")
		self.OK(res.noChangeset)
	
	def test_005_pendingChangeset(self):
		res = self.command(js.pendingChangesets)
		self.OK(res.OK)
		lines = len(res.result())
		# create another one
		res = self.command(js.startChangeset)
		self.OK(res.OK)
		res = self.command(js.pendingChangesets)
		self.OK(res.OK)
		self.assertEqual(len(res.result()),lines + 1)

	def test_006_abortCurrentChangeset(self):
		res = self.command(js.abortCurrentChangeset)
		self.OK(res.OK)
		# second times it crashes
		res = self.command(js.abortCurrentChangeset)
		self.OK(res.noChangeset)

	def test_007_kindInstances(self):
		res = self.command(js.kindNames)
		self.OK(res.OK)

	def _kindInstances(self):
		# get kind names at first
		res = self.command(js.kindNames)
		self.OK(res.OK)
		kindNames = res.result()
		# test all kindInstances
		for kind in kindNames:
			res = self.command(js.kindInstances,kind)
			self.OK(res.OK)

		# crash on bad kindName
		res = self.command(js.kindInstances,"error_kind_name")
		self.OK(res.otherError)

	def test_008a_kindInstances(self):
		# test with changeset assigned
		# create changeset
		res = self.command(js.startChangeset)
		self.OK(res.OK)
		self._kindInstances()

	def test_008b_kindInstances(self):
		# test with no changeset assigned
		self._kindInstances()

	def test_009_kindRelations(self):
		# get kind names at first
		res = self.command(js.kindNames)
		self.OK(res.OK)
		kindNames = res.result()
		# test all kindInstances
		for kind in kindNames:
			res = self.command(js.kindRelations,kind)
			self.OK(res.OK)

		# crash on bad kindName
		res = self.command(js.kindRelations,"error_kind_name")
		#rem self.OK(res.otherError)

	def test_010_kindAttributes(self):
		# get kind names at first
		res = self.command(js.kindNames)
		self.OK(res.OK)
		kindNames = res.result()
		# test all kindInstances
		for kind in kindNames:
			res = self.command(js.kindAttributes,kind)
			self.OK(res.OK)

		# crash on bad kindName
		res = self.command(js.kindAttributes,"error_kind_name")
		#rem self.OK(res.otherError)

	def test_011_filterTest(self):
		# r1 always exists
		filter = {"condition": "columnEq", "column": "revision", "value": "r1"}
		res = self.command(js.listRevisions,filter)
		self.OK(res.OK)
		# number of revisions - 1
		filter = {"condition": "columnNe", "column": "revision", "value": "r1"}
		res = self.command(js.listRevisions,filter)
		self.OK(res.OK)
		revisions1 = res.result()
		# number of revisions - 1 / other method
		filter = {"condition": "columnGt", "column": "revision", "value": "r1"}
		res = self.command(js.listRevisions,filter)
		self.OK(res.OK)
		revisions2 = res.result()
		# compare 2 results
		self.assertEqual(revisions1,revisions2)

		# 2 revisions
		filter = {"operator": "or", "operands": [
			{"condition": "columnEq", "column": "revision", "value": "r1"},
			{"condition": "columnEq", "column": "revision", "value": "r2"},
			{"condition": "columnEq", "column": "revision", "value": "r3"}
			]}
		res = self.command(js.listRevisions,filter)
		self.OK(res.OK)
		revisions1 = res.result()
		filter = {"operator": "and", "operands": [
			{"condition": "columnGe", "column": "revision", "value": "r1"},
			{"condition": "columnLe", "column": "revision", "value": "r3"}
			]}
		res = self.command(js.listRevisions,filter)
		self.OK(res.OK)
		revisions2 = res.result()
		self.assertEqual(revisions1,revisions2)

		# bad syntax of filter
		filter = {"xxx": "columnGt", "column": "revision", "value": "r1"}
		res = self.command(js.listRevisions,filter)
		self.OK(res.otherError)

		# test all conditions
		for cond in ["columnGe","columnLe","columnGt","columnLt","columnNe","columnEq"]:
			filter = {"condition": cond, "column": "revision", "value": "r1"}
			res = self.command(js.listRevisions,filter)
			self.OK(res.OK)





unittest.main()

