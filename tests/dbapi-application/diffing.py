from apiUtils import *

def imperative(r):
    changeset = r.c(startChangeset())
    revA = r.c(pendingChangesets(filter={"condition": "columnEq", "metadata":
                                         "changeset", "value": changeset})
              )[0]["parentRevision"]

    cmdlist1 = [
        createObject("vendor", "v1"),
        createObject("vendor", "v2"),
        createObject("hardware", "hw1")
    ]
    cmdlist2 = [
        setAttribute("hardware", "hw1", "vendor", "v1"),
        setAttribute("hardware", "hw1", "purchase", "2011-01-01"),
        setAttribute("hardware", "hw1", "warranty", "2011-01-01")
    ]
    for x in cmdlist1:
        r.c(x)
    for x in cmdlist2:
        r.cvoid(x)

    # FIXME: redmine #277
    #diff_in_changeset = r.c(dataDifferenceInTemporaryChangeset(changeset))

    revB = r.c(commitChangeset("test diff"))
    r.assertTrue(revA < revB)

    reportedDiff = r.c(dataDifference(revA, revB))
    expectedDiff = [
        {"command": "createObject", "kindName": "vendor", "objectName": "v1"},
        {"command": "createObject", "kindName": "vendor", "objectName": "v2"},
        {"command": "createObject", "kindName": "hardware", "objectName": "hw1"},
        {"command": "setAttribute", "kindName": "hardware", "objectName": "hw1", "attributeName": "vendor", "oldValue": None, "newValue": "v1"},
        {"command": "setAttribute", "kindName": "hardware", "objectName": "hw1", "attributeName": "warranty", "oldValue": None, "newValue": "2011-01-01"},
        {"command": "setAttribute", "kindName": "hardware", "objectName": "hw1", "attributeName": "purchase", "oldValue": None, "newValue": "2011-01-01"}
    ]
    r.assertEquals(reportedDiff, expectedDiff)

    # FIXME: redmine #277
    #r.assertEquals(sorted(reportedDiff), sorted(diff_in_changeset))

    changeset = r.c(startChangeset())
    cmdlist3 = [
        setAttribute("hardware", "hw1", "vendor", "v2"),
    ]
    for x in cmdlist3:
        r.cvoid(x)

    # FIXME: redmine #277
    #diff_in_changeset = r.c(dataDifferenceInTemporaryChangeset(changeset))

    revC = r.c(commitChangeset("changed one attribute"))
    reportedDiff = r.c(dataDifference(revB, revC))
    expectedDiff = [
        {"command": "setAttribute", "kindName": "hardware", "objectName": "hw1", "attributeName": "vendor", "oldValue": "v1", "newValue": "v2"},
    ]
    r.assertEquals(reportedDiff, expectedDiff)
    # FIXME: redmine #277
    #r.assertEquals(sorted(reportedDiff), sorted(diff_in_changeset))

    reportedDiff = r.c(dataDifference(revA, revC))
    expectedDiff = [
        {"command": "createObject", "kindName": "vendor", "objectName": "v1"},
        {"command": "createObject", "kindName": "vendor", "objectName": "v2"},
        {"command": "createObject", "kindName": "hardware", "objectName": "hw1"},
        # this is not present, as it's overridden by the next line:
        # {"command": "setAttribute", "kindName": "hardware", "objectName": "hw1", "attributeName": "vendor", "oldValue": None, "newValue": "v1"},
        {"command": "setAttribute", "kindName": "hardware", "objectName": "hw1", "attributeName": "vendor", "oldValue": None, "newValue": "v2"},
        {"command": "setAttribute", "kindName": "hardware", "objectName": "hw1", "attributeName": "warranty", "oldValue": None, "newValue": "2011-01-01"},
        {"command": "setAttribute", "kindName": "hardware", "objectName": "hw1", "attributeName": "purchase", "oldValue": None, "newValue": "2011-01-01"}
    ]

    # now be careful, we have to filter out the modification to hw1.vendor
    r.assertEquals(reportedDiff, expectedDiff)

    # try renaming an object
    changeset = r.c(startChangeset())
    cmdlist4 = [ renameObject("vendor", "v1", "v1a") ]
    for x in cmdlist4:
        r.cvoid(x)
    # FIXME: redmine #277
    #diff_in_changeset = r.c(dataDifferenceInTemporaryChangeset(changeset))
    revD = r.c(commitChangeset("renaming stuff"))
    reportedDiff = r.c(dataDifference(revC, revD))
    # FIXME: redmine #286, renames are reported back as setAttribute calls :(
    #r.assertEquals(sorted(helper_diff_2_cmdlist(reportedDiff)),
    #               sorted(helper_extract_commands(cmdlist4)))
    # FIXME: redmine #277
    #r.assertEquals(sorted(reportedDiff), sorted(diff_in_changeset))

    # now let's remove what we've added
    changeset = r.c(startChangeset())
    cmdlist4 = [
        deleteObject("vendor", "v2"),
        deleteObject("hardware", "hw1"),
        deleteObject("vendor", "v1a"),
    ]
    for x in cmdlist4:
        r.cvoid(x)

    # FIXME: redmine #277
    #diff_in_changeset = r.c(dataDifferenceInTemporaryChangeset(changeset))

    revE = r.c(commitChangeset("removed stuff"))
    # try with both of them
    for rev in (revB, revC):
        reportedDiff = r.c(dataDifference(rev, revE))
        expectedDiff = [
            {"command": "deleteObject", "kindName": "vendor", "objectName": "v1"},
            {"command": "deleteObject", "kindName": "vendor", "objectName": "v2"},
            {"command": "deleteObject", "kindName": "hardware", "objectName": "hw1"},
        ]
        r.assertEquals(reportedDiff, expectedDiff)
        # FIXME: redmine #277
        #r.assertEquals(sorted(reportedDiff), sorted(diff_in_changeset))

    reportedDiff = r.c(dataDifference(revD, revE))
    expectedDiff = [
        {"command": "deleteObject", "kindName": "vendor", "objectName": "v1a"},
        {"command": "deleteObject", "kindName": "vendor", "objectName": "v2"},
        {"command": "deleteObject", "kindName": "hardware", "objectName": "hw1"},
    ]
    r.assertEquals(reportedDiff, expectedDiff)
    # FIXME: redmine #277
    #r.assertEquals(sorted(reportedDiff), sorted(diff_in_changeset))

    # finally, there should be absolutely no difference in here
    # FIXME: Redmine #284, this one is broken
    #r.assertEquals(r.c(dataDifference(revA, revE)), [])
