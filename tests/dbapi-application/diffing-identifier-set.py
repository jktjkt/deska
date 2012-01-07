from apiUtils import *

def imperative(r):
    # Redmine #411, modifing an object reports an extra diff in its identifier_set

    # initialize data
    tmp1 = r.c(startChangeset())
    r.c(createObject("service", "a"))
    r.c(createObject("host", "h"))
    r.cvoid(setAttribute("host", "h", "service", ["a"]))
    r.c(commitChangeset("."))
    tmp2 = r.c(startChangeset())

    # perform an unrelated change
    r.cvoid(setAttribute("host", "h", "note_host", "x"))
    expectedDiff = [
        {'command': 'setAttribute', 'kindName': 'host', 'objectName': 'h', "attributeName": "note_host",
         "attributeData": "x", "oldAttributeData": None},
    ]
    # FIXME: Redmine#411, the output from inside a changeset contains this extra items:
    #    {'command': 'setAttribute', 'kindName': 'host', 'objectName': 'h', "attributeName": "service",
    #     "attributeData": None, "oldAttributeData": ["a"]},
    #r.assertEquals(r.c(dataDifferenceInTemporaryChangeset(tmp2)), expectedDiff)
    r.assertEquals(r.c(commitChangeset(".")), "r3")
    r.assertEquals(r.c(dataDifference("r2", "r3")), expectedDiff)
