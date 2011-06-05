from testutils import js
from deskatest import DeskaTest

class filterTest(DeskaTest):
	def test_011a_filterTest(self):
		'''test filter for listRevisions'''
		# r1 always exists
		filter = {"condition": "columnEq", "column": "revision", "value": "r1", "kind": "metadata"}
		res = self.command(js.listRevisions,filter)
		self.OK(res.OK)
		# number of revisions - 1
		filter = {"condition": "columnNe", "column": "revision", "value": "r1", "kind": "metadata"}
		res = self.command(js.listRevisions,filter)
		self.OK(res.OK)
		revisions1 = res.result()
		# number of revisions - 1 / other method
		filter = {"condition": "columnGt", "column": "revision", "value": "r1", "kind": "metadata"}
		res = self.command(js.listRevisions,filter)
		self.OK(res.OK)
		revisions2 = res.result()
		# compare 2 results
		self.assertEqual(revisions1,revisions2)

		# 2 revisions
		filter = {"operator": "or", "operands": [
			{"condition": "columnEq", "column": "revision", "value": "r1", "kind": "metadata"},
			{"condition": "columnEq", "column": "revision", "value": "r2", "kind": "metadata"},
			{"condition": "columnEq", "column": "revision", "value": "r3", "kind": "metadata"}
			]}
		res = self.command(js.listRevisions,filter)
		self.OK(res.OK)
		revisions1 = res.result()
		filter = {"operator": "and", "operands": [
			{"condition": "columnGe", "column": "revision", "value": "r1", "kind": "metadata"},
			{"condition": "columnLe", "column": "revision", "value": "r3", "kind": "metadata"}
			]}
		res = self.command(js.listRevisions,filter)
		self.OK(res.OK)
		revisions2 = res.result()
		self.assertEqual(revisions1,revisions2)

		# bad syntax of filter
		filter = {"xxx": "columnGt", "column": "revision", "value": "r1", "kind": "metadata"}
		res = self.command(js.listRevisions,filter)
		self.OK(res.otherError)

		# test all conditions
		for cond in ["columnGe","columnLe","columnGt","columnLt","columnNe","columnEq"]:
			filter = {"condition": cond, "column": "revision", "value": "r1", "kind": "metadata"}
			res = self.command(js.listRevisions,filter)
			self.OK(res.OK)

	def test_011b_filterTest(self):
		'''test filter for pendingChangeset'''
		# tmp1 always exists
		filter = {"condition": "columnEq", "column": "changeset", "value": "tmp1", "kind": "metadata"}
		res = self.command(js.pendingChangesets,filter)
		self.OK(res.OK)
		# number of changesets - 1
		filter = {"condition": "columnNe", "column": "changeset", "value": "tmp1", "kind": "metadata"}
		res = self.command(js.pendingChangesets,filter)
		self.OK(res.OK)
		changesets1 = res.result()
		# number of changesets - 1 / other method
		filter = {"condition": "columnGt", "column": "changeset", "value": "tmp1", "kind": "metadata"}
		res = self.command(js.pendingChangesets,filter)
		self.OK(res.OK)
		changesets2 = res.result()
		# compare 2 results
		self.assertEqual(changesets1,changesets2)

		# 2 changesets
		filter = {"operator": "or", "operands": [
			{"condition": "columnEq", "column": "changeset", "value": "tmp1", "kind": "metadata"},
			{"condition": "columnEq", "column": "changeset", "value": "tmp2", "kind": "metadata"},
			{"condition": "columnEq", "column": "changeset", "value": "tmp3", "kind": "metadata"}
			]}
		res = self.command(js.pendingChangesets,filter)
		self.OK(res.OK)
		changesets1 = res.result()
		filter = {"operator": "and", "operands": [
			{"condition": "columnGe", "column": "changeset", "value": "tmp1", "kind": "metadata"},
			{"condition": "columnLe", "column": "changeset", "value": "tmp3", "kind": "metadata"}
			]}
		res = self.command(js.pendingChangesets,filter)
		self.OK(res.OK)
		changesets2 = res.result()
		self.assertEqual(changesets1,changesets2)

		# bad syntax of filter
		filter = {"xxx": "columnGt", "column": "changeset", "value": "tmp1", "kind": "metadata"}
		res = self.command(js.pendingChangesets,filter)
		self.OK(res.otherError)

		# test all conditions
		for cond in ["columnGe","columnLe","columnGt","columnLt","columnNe","columnEq"]:
			filter = {"condition": cond, "column": "changeset", "value": "tmp1", "kind": "metadata"}
			res = self.command(js.pendingChangesets,filter)
			self.OK(res.OK)

	def test_012_filterJoinTest(self):
		'''test joining for listRevisions and pendingChangeset'''
		# join listRevisions with hardware
		filter = {"condition": "columnEq", "column": "name", "value": "hp2", "kind": "hardware"}
		res = self.command(js.listRevisions,filter)
		self.OK(res.OK)
		# join pendingChangeset with hardware
		filter = {"condition": "columnEq", "column": "name", "value": "hp2", "kind": "hardware"}
		res = self.command(js.pendingChangesets,filter)
		self.OK(res.OK)

