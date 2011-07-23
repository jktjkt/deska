'''Test basic features of the deska module'''

from apiUtils import *

def imperative(r):
    prepareObjects(r)
    doTests(r)

def prepareObjects(r):
    '''Seed the DB with some data'''
    r.c(startChangeset())
    for x in range(10):
        objname = "x%d" % x
        r.cvoid(createObject("host", objname))
        r.cvoid(setAttribute("host", objname, "note", "ahoj"))
    r.c(commitChangeset("objects set up"))

def doTests(r):
    '''Test that we get reasonable data back'''
    import deska

    expectedNames = ["x%d" % x for x in range(10)]
    r.assertEqual(sorted(r.c(kindInstances("host"))), expectedNames)
    # the following should match all objects in the DB
    matching = deska.host[deska.host.note == "ahoj"]
    r.assertEqual(sorted(matching.keys()), expectedNames)
    # check the actual object data
    for x in matching.itervalues():
        r.assertEqual(x.hardware, None)
        r.assertEqual(x.note, "ahoj")

    # make sure that the following returns no results
    r.assertEqual(deska.host[deska.host.note != "ahoj"].keys(), [])
    r.assertEqual(deska.host[deska.host.note == "bla"].keys(), [])
