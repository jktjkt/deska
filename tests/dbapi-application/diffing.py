from apiUtils import *

def imperative(r):
    changeset = r.c(startChangeset())
    revA = r.c(pendingChangesets(filter={"condition": "columnEq", "metadata":
                                         "changeset", "value": changeset})
              )[0]["parentRevision"]

    r.c(createObject("vendor", "v1"))
    r.c(createObject("vendor", "v2"))
    r.c(createObject("hardware", "hw1"))
    r.cvoid(setAttribute("hardware", "hw1", "vendor", "v1"))
    r.cvoid(setAttribute("hardware", "hw1", "purchase", "2011-01-01"))
    r.cvoid( setAttribute("hardware", "hw1", "warranty", "2011-01-01"))

    # FIXME: redmine #277
    #diff_in_changeset = r.c(dataDifferenceInTemporaryChangeset(changeset))

    revB = r.c(commitChangeset("test diff"))
    r.assertTrue(revA < revB)

    reportedDiff = r.c(dataDifference(revA, revB))
    expectedDiff = [
        {"command": "createObject", "kindName": "vendor", "objectName": "v1"},
        {"command": "createObject", "kindName": "vendor", "objectName": "v2"},
        {"command": "createObject", "kindName": "hardware", "objectName": "hw1"},
        {"command": "setAttribute", "kindName": "hardware", "objectName": "hw1", "attributeName": "vendor", "oldAttributeData": None, "attributeData": "v1"},
        {"command": "setAttribute", "kindName": "hardware", "objectName": "hw1", "attributeName": "warranty", "oldAttributeData": None, "attributeData": "2011-01-01"},
        {"command": "setAttribute", "kindName": "hardware", "objectName": "hw1", "attributeName": "purchase", "oldAttributeData": None, "attributeData": "2011-01-01"}
    ]
    r.assertEquals(reportedDiff, expectedDiff)

    # FIXME: redmine #277
    #r.assertEquals(sorted(reportedDiff), sorted(diff_in_changeset))

    changeset = r.c(startChangeset())
    r.cvoid(setAttribute("hardware", "hw1", "vendor", "v2"))

    # FIXME: redmine #277
    #diff_in_changeset = r.c(dataDifferenceInTemporaryChangeset(changeset))

    revC = r.c(commitChangeset("changed one attribute"))
    reportedDiff = r.c(dataDifference(revB, revC))
    expectedDiff = [
        {"command": "setAttribute", "kindName": "hardware", "objectName": "hw1", "attributeName": "vendor", "oldAttributeData": "v1", "attributeData": "v2"},
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
        # {"command": "setAttribute", "kindName": "hardware", "objectName": "hw1", "attributeName": "vendor", "oldAttributeData": None, "attributeData": "v1"},
        {"command": "setAttribute", "kindName": "hardware", "objectName": "hw1", "attributeName": "vendor", "oldAttributeData": None, "attributeData": "v2"},
        {"command": "setAttribute", "kindName": "hardware", "objectName": "hw1", "attributeName": "warranty", "oldAttributeData": None, "attributeData": "2011-01-01"},
        {"command": "setAttribute", "kindName": "hardware", "objectName": "hw1", "attributeName": "purchase", "oldAttributeData": None, "attributeData": "2011-01-01"}
    ]

    # now be careful, we have to filter out the modification to hw1.vendor
    r.assertEquals(reportedDiff, expectedDiff)

    # try renaming an object
    changeset = r.c(startChangeset())
    r.cvoid(renameObject("vendor", "v1", "v1a"))
    # FIXME: redmine #277
    #diff_in_changeset = r.c(dataDifferenceInTemporaryChangeset(changeset))
    revD = r.c(commitChangeset("renaming stuff"))
    reportedDiff = r.c(dataDifference(revC, revD))
    expectedDiff = [{"command": "renameObject", "kindName": "vendor", "oldObjectName": "v1", "newObjectName": "v1a"}]
    # FIXME: redmine #286, renames are reported back as setAttribute calls :(
    r.assertEquals(reportedDiff, expectedDiff)
    # FIXME: redmine #277
    #r.assertEquals(sorted(reportedDiff), sorted(diff_in_changeset))

    # now let's remove what we've added
    changeset = r.c(startChangeset())
    r.cvoid(deleteObject("vendor", "v2"))
    r.cvoid(deleteObject("hardware", "hw1"))
    r.cvoid(deleteObject("vendor", "v1a"))

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
    r.assertEquals(r.c(dataDifference(revA, revE)), [])
