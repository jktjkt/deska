from apiUtils import *

def imperative(r):
    # Redmine #441, modifing an object reports an extra diff in its identifier_set

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
    # FIXME: Redmine#441, the output from inside a changeset contains this extra items:
    #    {'command': 'setAttribute', 'kindName': 'host', 'objectName': 'h', "attributeName": "service",
    #     "attributeData": None, "oldAttributeData": ["a"]},
    #r.assertEquals(r.c(dataDifferenceInTemporaryChangeset(tmp2)), expectedDiff)
    r.assertEquals(r.c(commitChangeset(".")), "r3")
    r.assertEquals(r.c(dataDifference("r2", "r3")), expectedDiff)

    # Try to remove this host nad make sure that the commit succeeds. Previously, this would fail
    # because of a conflict in the ordering which would get caught by the constraint checks.
    tmp3 = r.c(startChangeset())
    r.cvoid(deleteObject("host", "h"))
    expectedDiff = [
        {'command': 'deleteObject', 'kindName': 'host', 'objectName': 'h'},
    ]
    # FIXME: another manifestation of Redmine#441
    #r.assertEquals(r.c(dataDifferenceInTemporaryChangeset(tmp3)), expectedDiff)
    r.assertEquals(r.c(commitChangeset(".")), "r4")
    r.assertEquals(r.c(dataDifference("r3", "r4")), expectedDiff)

    # Verify deleting service the other way round
    # Start with some object initialization
    tmp4 = r.c(startChangeset())
    r.c(createObject("service", "b"))
    r.c(createObject("host", "x"))
    r.cvoid(setAttribute("host", "x", "service", ["a"]))
    r.c(createObject("host", "y"))
    r.cvoid(setAttribute("host", "y", "service", ["b"]))
    r.c(createObject("host", "z"))
    r.cvoid(setAttribute("host", "z", "service", ["a", "b"]))
    expectedDiff = [
        {'command': 'createObject', 'kindName': 'service', 'objectName': 'b'},
        {'command': 'createObject', 'kindName': 'host', 'objectName': 'x'},
        {'command': 'createObject', 'kindName': 'host', 'objectName': 'y'},
        {'command': 'createObject', 'kindName': 'host', 'objectName': 'z'},
        {'command': 'setAttribute', 'kindName': 'host', 'objectName': 'x', "attributeName": "service",
         "attributeData": ["a"], "oldAttributeData": None},
        {'command': 'setAttribute', 'kindName': 'host', 'objectName': 'y', "attributeName": "service",
         "attributeData": ["b"], "oldAttributeData": None},
        {'command': 'setAttribute', 'kindName': 'host', 'objectName': 'z', "attributeName": "service",
         "attributeData": ["a", "b"], "oldAttributeData": None},
    ]
    r.assertEquals(r.c(dataDifferenceInTemporaryChangeset(tmp4)), expectedDiff)
    r.assertEquals(r.c(commitChangeset(".")), "r5")
    r.assertEquals(r.c(dataDifference("r4", "r5")), expectedDiff)
    # Try deleting stuff
    tmp5 = r.c(startChangeset())
    r.cvoid(deleteObject("service", "b"))
    # We've just deleted one object, nothing else. There are dangling references, but this behavior is consistent with
    # what happens when we delete a vendor that is referenced from a hardware -- we get a constraint error.
    r.assertEquals(r.c(dataDifferenceInTemporaryChangeset(tmp5)),
        [{'command': 'deleteObject', 'kindName': 'service', 'objectName': 'b'}])
    # FIXME: shall raise proper exception...
    #r.cfail(commitChangeset("."), exception=ConstraintError())
    r.cfail(commitChangeset("."))
    # so we delete these extra service references by hand
    r.cvoid(setAttributeRemove("host", "y", "service", "b"))
    r.cvoid(setAttributeRemove("host", "z", "service", "b"))
    # FIXME: Redmine#441, "y" reports the old data as [] instead of ['b']
    # and "z" says that the old data are ['a', 'None'] instead of ['a', 'b']
    expectedDiff = [
        {'command': 'deleteObject', 'kindName': 'service', 'objectName': 'b'},
        {'command': 'setAttribute', 'kindName': 'host', 'objectName': 'y', "attributeName": "service",
         "attributeData": [],
         #"oldAttributeData": ["b"]
         "oldAttributeData": []
        },
        {'command': 'setAttribute', 'kindName': 'host', 'objectName': 'z', "attributeName": "service",
         "attributeData": ["a"],
         #"oldAttributeData": ["a", "b"]
         "oldAttributeData": ["a", 'None']
        },
    ]
    r.assertEquals(r.c(dataDifferenceInTemporaryChangeset(tmp5)), expectedDiff)
    r.assertEquals(r.c(commitChangeset(".")), "r6")
    r.assertEquals(r.c(dataDifference("r5", "r6")), expectedDiff)


    # Test references becoming undefined
    tmp6 = r.c(startChangeset())
    r.c(createObject("vendor", "v"))
    r.c(createObject("hardware", "dummy"))
    # note: this one is different, it gets linked to the host
    r.c(createObject("hardware", "z"))
    r.cvoid(setAttribute("hardware", "z", "vendor", "v"))
    r.cvoid(setAttribute("hardware", "dummy", "vendor", "v"))
    expectedDiff = [
        {'command': 'createObject', 'kindName': 'vendor', 'objectName': 'v'},
        {'command': 'createObject', 'kindName': 'hardware', 'objectName': 'dummy'},
        {'command': 'createObject', 'kindName': 'hardware', 'objectName': 'z'},
        {'command': 'setAttribute', 'kindName': 'hardware', 'objectName': 'dummy', "attributeName": "vendor",
         "attributeData": "v", "oldAttributeData": None},
        {'command': 'setAttribute', 'kindName': 'hardware', 'objectName': 'z', "attributeName": "host",
         "attributeData": "z", "oldAttributeData": None},
        {'command': 'setAttribute', 'kindName': 'hardware', 'objectName': 'z', "attributeName": "vendor",
         "attributeData": "v", "oldAttributeData": None},
        {'command': 'setAttribute', 'kindName': 'host', 'objectName': 'z', "attributeName": "hardware",
         "attributeData": "z", "oldAttributeData": None},
        # ...FIXME: #441, this one is *not* expected here
        {'command': 'setAttribute', 'kindName': 'host', 'objectName': 'z', "attributeName": "service",
         "attributeData": None, "oldAttributeData": ['a']},
    ]
    r.assertEquals(r.c(dataDifferenceInTemporaryChangeset(tmp6)), expectedDiff)
    r.assertEquals(r.c(commitChangeset(".")), "r7")
    r.assertEquals(r.c(dataDifference("r6", "r7")), expectedDiff)
