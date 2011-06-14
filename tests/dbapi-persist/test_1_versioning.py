from testutils import js
from deskatest import DeskaTest

class Versioning(DeskaTest):
	'''Test versioning functions'''
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
