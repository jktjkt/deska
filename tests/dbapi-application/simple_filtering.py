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
        # Cannot use non-existing revision here (Redmine #306), so let's check
        # against a date instead.
        {"condition": "columnLt", "metadata": "timestamp", "value": "3333-12-31 23:59:59"},
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
    numericData = [("Ge", 4, [4, 5]), ("Gt", 2, [3, 4, 5]),
                   ("Lt", 4, [1, 2, 3]), ("Le", 4, [1, 2, 3, 4]),
                   ("Ne", 3, [1, 2, 4, 5]), ("Eq", 2, [2])]
    for (op, key, res) in numericData:
        myFilter = {"condition": "column%s" % op, "metadata": "revision", "value": "r%d" % key}
        r.assertEqual([x["revision"] for x in r.c(listRevisions(filter=myFilter))],
                     ["r%d" % x for x in res])

    # test a wrong filter
    r.cfail(listRevisions(filter={"xxx": "columnGt"}))


    # now test a few pending changesets
    # situation is a bit more complicated, because the first changeset we create
    # is not really tmp1, so we will not filter against the key, but against the
    # "message"
    for x in range(1, 6):
        r.c(startChangeset())
        r.cvoid(detachFromCurrentChangeset("%d" % x))
    for (op, key, res) in numericData:
        myFilter = {"condition": "column%s" % op, "metadata": "message", "value": "%d" % key}
        r.assertEqual([x["message"] for x in r.c(pendingChangesets(filter=myFilter))],
                     ["%d" % x for x in res])

    # create some changesets with some useful data in them
    changesetA = r.c(startChangeset())
    r.c(createObject("hardware", "hw1"))
    r.cvoid(detachFromCurrentChangeset("."))
    changesetB = r.c(startChangeset())
    r.c(createObject("hardware", "hw2"))
    r.cvoid(detachFromCurrentChangeset("."))
    changesetC = r.c(startChangeset())
    r.c(createObject("hardware", "hw3"))

    # ask for all changesets which touch a particular object
    r.assertEqual([x["changeset"] for x in
                   r.c(pendingChangesets(filter={"condition": "columnEq", "attribute": "name", "value": "hw2", "kind":"hardware"}))
                  ], [changesetB])

    r.cvoid(detachFromCurrentChangeset(".."))

    # now commit these changes (cannot just commit the changesets, but got to
    # recreate the changes)
    r.c(startChangeset())
    helper_createHw(r, "hw1")
    revA = r.c(commitChangeset("."))
    r.c(startChangeset())
    helper_createHw(r, "hw2")
    revB = r.c(commitChangeset("."))
    r.c(startChangeset())
    helper_createHw(r, "hw3")
    r.cvoid(setAttribute("hardware", "hw2", "note_hardware", "foo"))
    revC = r.c(commitChangeset("."))
    r.c(startChangeset())
    helper_createHw(r, "hw4")
    revD = r.c(commitChangeset("."))


    # ask for all revisions which have touched a particular object
    r.assertEqual([x["revision"] for x in
                   r.c(listRevisions(filter={"condition": "columnEq", "attribute": "name", "value": "hw2", "kind":"hardware"}))
                  ], [revB, revC])

def helper_createHw(r, name):
    r.c(createObject("hardware", name))
    r.cvoid(setAttribute("hardware", name, "purchase", "2011-03-04"))
    r.cvoid(setAttribute("hardware", name, "warranty", "2011-03-04"))
