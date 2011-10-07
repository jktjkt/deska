from apiUtils import *

def imperative(r):
    myFilter = {"condition": "columnEq", "metadata": "revision", "value": "r1"}

    # r1 should be there
    revisions = r.c(listRevisions(myFilter))
    r.assertEqual(revisions[0]["revision"], "r1")

    # but no other revisions
    for operator in ("columnGt", "columnNe", "columnLt"):
        myFilter["condition"] = operator
        r.assertEqual(r.c(listRevisions(myFilter)), [])

    # let's create some, then!
    for x in range(2):
        r.c(startChangeset())
        r.c(commitChangeset("blah %d" % x))

    filters = [
        {"operator": "or", "operands": [
			{"condition": "columnEq", "metadata": "revision", "value": "r1"},
			{"condition": "columnEq", "metadata": "revision", "value": "r2"},
			{"condition": "columnEq", "metadata": "revision", "value": "r3"}
			]
        },
        {"condition": "columnGe", "metadata": "revision", "value": "r1"},
        {"condition": "columnLt", "metadata": "revision", "value": "r666"},
        {"operator": "and", "operands": [
			{"condition": "columnGe", "metadata": "revision", "value": "r1"},
			{"condition": "columnLe", "metadata": "revision", "value": "r3"},
			]
        }
    ]
    for f in filters:
        r.assertEqual(r.c(listRevisions()), r.c(listRevisions(filter=f)))

    # create two more
    for x in range(2):
        r.c(startChangeset())
        r.c(commitChangeset("blah %d" % x))

    # try all operators
    for (op, key, res) in [("Ge", 4, [4, 5]), ("Gt", 2, [3, 4, 5]),
                           ("Lt", 4, [1, 2, 3]), ("Le", 4, [1, 2, 3, 4]),
                           ("Ne", 3, [1, 2, 4, 5]), ("Eq", 2, [2])]:
        myFilter = {"condition": "column%s" % op, "metadata": "revision", "value": "r%d" % key}
        r.assertEqual([x["revision"] for x in r.c(listRevisions(filter=myFilter))],
                     ["r%d" % x for x in res])

    # test a wrong filter
    r.cfail(listRevisions(filter={"xxx": "columnGt"}))


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

