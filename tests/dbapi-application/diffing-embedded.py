from apiUtils import *

def imperative(r):
    changeset = r.c(startChangeset())
    r.c(createObject("host", "h1"))
    r.c(createObject("interface", "h1->i1"))

    diffInChangeset = r.c(dataDifferenceInTemporaryChangeset(changeset))
    expectedDiff = [
        {'command': 'createObject', 'kindName': 'host', 'objectName': 'h1'},
        {'command': 'createObject', 'kindName': 'interface', 'objectName': 'h1->i1'}
    ]
    r.assertEquals(diffInChangeset, expectedDiff)
    revB = r.c(commitChangeset("."))
    diffInRevision = r.c(dataDifference("r1", revB))
    r.assertEquals(diffInRevision, expectedDiff)

    changeset = r.c(startChangeset())
    r.cvoid(setAttribute("interface", "h1->i1", "note", "foo"))
    expectedDiff = [
        {'command': 'setAttribute', 'kindName': 'interface', 'objectName': 'h1->i1',
         'attributeName': 'note', 'attributeData': 'foo', 'oldAttributeData': None}
    ]
    diffInChangeset = r.c(dataDifferenceInTemporaryChangeset(changeset))
    r.assertEquals(diffInChangeset, expectedDiff)
    revC = r.c(commitChangeset("."))
    diffInRevision = r.c(dataDifference(revB, revC))
    r.assertEquals(diffInRevision, expectedDiff)
