from testutils import js
from deskatest import DeskaTest

class metadataTest(DeskaTest):
	'''test metadata functions'''
	def test_001_kindNames(self):
		'''test kindNames returns list'''
		res = self.command(js.kindNames)
		self.OK(res.OK)
		self.assertTrue(type(res.result()) == list)

	def all_kindInstances(self):
		'''helper kindInstances returns list'''
		# get kind names at first
		res = self.command(js.kindNames)
		self.OK(res.OK)
		kindNames = res.result()
		# test all kindInstances
		for kind in kindNames:
			res = self.command(js.kindInstances,kind)
			self.OK(res.OK)
			self.assertTrue(type(res.result()) == list)

		# crash on bad kindName
		res = self.command(js.kindInstances,"error_kind_name")
		self.OK(res.otherError)

	def test_008a_kindInstances(self):
		'''test with changeset assigned'''
		# create changeset
		res = self.command(js.startChangeset)
		self.OK(res.OK)
		self.all_kindInstances()
		res = self.command(js.abortCurrentChangeset)
		self.OK(res.OK)

	def test_008b_kindInstances(self):
		'''test with changeset assigned'''
		self.all_kindInstances()

	def test_009_kindRelations(self):
		'''test kindRelations for all kinds'''
		# get kind names at first
		res = self.command(js.kindNames)
		self.OK(res.OK)
		kindNames = res.result()
		# test all kindInstances
		for kind in kindNames:
			res = self.command(js.kindRelations,kind)
			self.OK(res.OK)
			self.assertTrue(type(res.result()) == list)

		# crash on bad kindName
		res = self.command(js.kindRelations,"error_kind_name")
		#rem self.OK(res.otherError)

	def test_010_kindAttributes(self):
		'''test kindAttributes for all kinds'''
		# get kind names at first
		res = self.command(js.kindNames)
		self.OK(res.OK)
		kindNames = res.result()
		# test all kindInstances
		for kind in kindNames:
			res = self.command(js.kindAttributes,kind)
			self.OK(res.OK)
			self.assertTrue(type(res.result()) == dict)

		# crash on bad kindName
		res = self.command(js.kindAttributes,"error_kind_name")
		#rem self.OK(res.otherError)

