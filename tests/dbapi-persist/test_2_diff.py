from testutils import js
import testutils 
from deskatest import DeskaTest
import json

class diffTest(DeskaTest):
	'''test diffing'''
	def diff2cmdlist(self,diff):
		newlist = list()
		for oldcmd in diff:
			cmd = oldcmd.copy()
			if cmd["command"] == "setAttribute":
				del cmd["oldValue"]
				cmd["attributeData"] = cmd["newValue"]
				del cmd["newValue"]
			newlist.append(cmd)
		return newlist

	def revertDiff(self,diff):
		for cmd in diff:
			if cmd["command"] == "setAttribute":
				cmd["attributeData"] = cmd["oldValue"]
				if cmd["attributeData"] == 'None':
					print "HERE"
					cmd["attributeData"] = None
				del cmd["oldValue"]
				del cmd["newValue"]
			elif cmd["command"] == "createObject":
				cmd["command"] = "deleteObject"
			elif cmd["command"] == "deleteObject":
				cmd["command"] = "createObject"
		diff.reverse()
		return diff


	def test_001_dataDifference(self):
		'''test diff'''
		# start changeset
		res = self.command(js.startChangeset)
		self.OK(res.OK)
		cmdlist = list()
		# add commands into cmdlist and run
		cmdlist.append(json.loads(js.createObject("vendor","test vendor")))
		cmdlist.append(json.loads(js.createObject("vendor","test vendor 2")))
		cmdlist.append(json.loads(js.createObject("hardware","test hardware 1")))
		cmdlist.append(json.loads(js.setAttribute("hardware","test hardware 1","vendor","test vendor")))
		cmdlist.append(json.loads(js.setAttribute("hardware","test hardware 1","purchase","2011-01-01")))
		cmdlist.append(json.loads(js.setAttribute("hardware","test hardware 1","warranty","2011-01-01")))
		self.commandList(cmdlist)

		# commit
		res = self.command(js.commitChangeset,"test diff")
		self.OK(res.OK)
		revB = res.result()
		revA = testutils.updateRev(revB,-1)

		# diff is ok?
		res = self.command(js.dataDifference,revA,revB)
		self.OK(res.OK)
		diff = res.result()
		listToCompare = self.diff2cmdlist(diff)
		self.assertEqual(listToCompare.sort(),cmdlist.sort())

		# and undo changes
		# start changeset
		res = self.command(js.startChangeset)
		self.OK(res.OK)
		# revert diff and run
		cmdlist = self.revertDiff(diff)
		self.commandList(cmdlist)

		# commit
		res = self.command(js.commitChangeset,"test diff")
		self.OK(res.OK)
		revC = res.result()

		# diff should be empty
		res = self.command(js.dataDifference,revA,revC)
		self.OK(res.OK)
		self.assertTrue(res.result() == [])

		# diff is ok?
		res = self.command(js.dataDifference,revB,revC)
		self.OK(res.OK)
		diff = res.result()
		listToCompare = self.diff2cmdlist(diff)
		self.assertEqual(listToCompare.sort(),cmdlist.sort())

	def test_002_dataDifferenceTemp(self):
		'''test diff in changeset'''
		# start changeset
		res = self.command(js.startChangeset)
		self.OK(res.OK)
		cmdlist = list()
		# add commands into cmdlist and run
		cmdlist.append(json.loads(js.createObject("vendor","test vendor")))
		cmdlist.append(json.loads(js.createObject("vendor","test vendor 2")))
		cmdlist.append(json.loads(js.createObject("hardware","test hardware 1")))
		cmdlist.append(json.loads(js.setAttribute("hardware","test hardware 1","vendor","test vendor")))
		cmdlist.append(json.loads(js.setAttribute("hardware","test hardware 1","purchase","2011-01-01")))
		cmdlist.append(json.loads(js.setAttribute("hardware","test hardware 1","warranty","2011-01-01")))
		self.commandList(cmdlist)

		# diff is ok?
		res = self.command(js.dataDifferenceInTemporaryChangeset)
		self.OK(res.OK)
		diff = res.result()
		listToCompare = self.diff2cmdlist(diff)
		self.assertEqual(listToCompare.sort(),cmdlist.sort())

		# abort
		res = self.command(js.abortCurrentChangeset)
