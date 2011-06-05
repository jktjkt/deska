from testutils import js
from deskatest import DeskaTest

class Versioning(DeskaTest):
	'''Test versioning functions'''
	def test_001_startChangeset(self):
		'''test start changeset'''
		res = self.command(js.startChangeset)
		self.OK(res.OK)
		# second time we expect exception
		res = self.command(js.startChangeset)
		self.OK(res.changesetOpen)

	def test_002_commitChangeset(self):
		'''test commit changeset'''
		# FIXME: no changeset should raise exception
		res = self.command(js.commitChangeset,"message")
		self.OK(res.OK)
		# second time we expect exception
		res = self.command(js.commitChangeset,"message")
		self.OK(res.noChangeset)
		
	def test_003_resumeChangeset(self):
		'''test resume changeset'''
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
		'''test detach changeset'''
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
		'''test that create changeset adds one changeset in pendinch changeset'''
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
		'''test abort changeset'''
		res = self.command(js.abortCurrentChangeset)
		self.OK(res.OK)
		# second times it crashes
		res = self.command(js.abortCurrentChangeset)
		self.OK(res.noChangeset)

