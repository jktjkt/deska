import os
import json

class DeskaRunner():
	def __init__(self):
		self.stdin, self.stdout = os.popen2("python deska_server.py")
	
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
		return self.data[key]

	def response(self):
		return self["response"]

	def command(self):
		return self["command"]

	def result(self):
		return self[self.response()]

	def error(self):
		return self["dbException"]
	
	def all(self):
		return str(self.data)


js = JsonBuilder()
tr = DeskaRunner()

testlist = [
[js.startChangeset,JsonParser.result],
[js.startChangeset,JsonParser.error]
]

tests = 0
bad = 0
for test in testlist:
	cmd = test[0]()
	res = tr.command(cmd)
	jp = JsonParser(res)
	tests = tests + 1
	try:
		print str(cmd) + "\t\t\t" + str(test[1](jp))
	except:
		print str(cmd) + "\t\t\tFAILED!!!"
		bad = bad + 1
	
if bad == 0:
	print "PASSED."
else:
	print "{bad} tests FAILED (out of {tests}).".format(bad = bad, tests = tests)
