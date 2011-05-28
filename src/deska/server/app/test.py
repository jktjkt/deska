import os
import json
import unittest
import sys

class DeskaRunner():
	def __init__(self):
		#runstr = "python {server} {db}".format(server=sys.argv[1], db=sys.argv[2])
		runstr = "python deska_server.py deska_dev"
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

	def resumeChangeset(self,chid):
		return self.command("resumeChangeset",**{"changeset": chid})

	def listRevisions(self):
		return self.command("listRevisions")

	def pendingChangesets(self):
		return self.command("pendingChangesets")

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
		return str(self[self.response()])

	def error(self):
		return str(self["dbException"])
	
	def all(self):
		return str(self.data)

	def OK(self):
		return not "dbException" in self


js = JsonBuilder()
tr = DeskaRunner()

class DeskaTest(unittest.TestCase):
	tmp = 'x'
	def command(self,cmd,*args):
		jsn = cmd(*args)
		cmd = JsonParser(jsn)
		res = tr.command(jsn)
		jp = JsonParser(res)
		self.assertEqual(jp["response"], cmd["command"])
		return jp

	def test_001_startChangeset(self):
		res = self.command(js.startChangeset)
		self.assertTrue(res.OK())
		# second time we expect error
		res = self.command(js.startChangeset)
		self.assertTrue(not res.OK())

	def test_002_commitChangeset(self):
		res = self.command(js.commitChangeset,"message")
		self.assertTrue(res.OK())
		# second time we expect error
		res = self.command(js.commitChangeset,"message")
		self.assertTrue(not res.OK())
		
	def test_003_resumeChangeset(self):
		res = self.command(js.startChangeset)
		self.assertTrue(res.OK())
		chid = res.result()
		res = self.command(js.resumeChangeset,chid)
		self.assertTrue(res.OK())
		# bad id
		res = self.command(js.resumeChangeset,"r12")
		self.assertTrue(not res.OK())
		
unittest.main()

