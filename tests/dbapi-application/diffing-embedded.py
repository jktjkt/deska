from apiUtils import *

def imperative(r):
    changeset = r.c(startChangeset())
    r.c(createObject("host", "h1"))
    r.c(createObject("interface", "h1->i1"))

    diffInChangeset = r.c(dataDifferenceInTemporaryChangeset(changeset))
    expectedDiff = [
        {'command': 'createObject', 'kindName': 'host', 'objectName': 'h1'},
        # FIXME: there should be just a createObject(interface, h1->i1) instead of
        # a combination of createObject and setAttribute
        #{'command': 'createObject', 'kindName': 'interface', 'objectName': 'h1->i1'},
        # Redmine #300
        {'command': 'createObject', 'kindName': 'interface', 'objectName': 'i1'},
        {'command': 'setAttribute', 'kindName': 'interface', 'objectName': 'i1',
         'attributeName': 'host', 'attributeData': 'h1', 'oldAttributeData': None},
    ]
    r.assertEquals(diffInChangeset, expectedDiff)
    revB = r.c(commitChangeset("."))
    diffInRevision = r.c(dataDifference("r1", revB))
    r.assertEquals(diffInRevision, expectedDiff)

    changeset = r.c(startChangeset())
    r.cvoid(setAttribute("interface", "h1->i1", "note", "foo"))
    expectedDiff = [
        # FIXME: Redmine #300
        #{'command': 'setAttribute', 'kindName': 'interface', 'objectName': 'h1->i1',
        {'command': 'setAttribute', 'kindName': 'interface', 'objectName': 'i1',
         'attributeName': 'note', 'attributeData': 'foo', 'oldAttributeData': None}
    ]
    diffInChangeset = r.c(dataDifferenceInTemporaryChangeset(changeset))
    r.assertEquals(diffInChangeset, expectedDiff)
    revC = r.c(commitChangeset("."))
    diffInRevision = r.c(dataDifference(revB, revC))
    r.assertEquals(diffInRevision, expectedDiff)
