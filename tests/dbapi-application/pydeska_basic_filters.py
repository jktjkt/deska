'''Test basic features of the deska module'''

from apiUtils import *

myHw = {}

def imperative(r):
    prepareObjects(r)
    doTests(r)

def prepareObjects(r):
    '''Seed the DB with some data'''

    myVendors = ["HP", "IBM", "SGI"]
    r.c(startChangeset())
    for x in range(10):
        objname = "x%d" % x
        r.cvoid(createObject("host", objname))
        r.cvoid(setAttribute("host", objname, "note", "ahoj"))
    for vendor in myVendors:
        r.cvoid(createObject("vendor", vendor))
    for x in range(5):
        objname = "hw_%d" % x
        myDate = x+1
        myDateStr = "2011-04-%02d" % myDate
        myVendor = myVendors[x % len(myVendors)]
        myCPUs = str((x / 4) + 1)
        myRAM = str((x / 2) + 2)
        myHw[objname] = {"date": myDate, "vendor": myVendor, "cpu": myCPUs,
                         "ram": myRAM}
        r.cvoid(createObject("hardware", objname))
        r.cvoid(setAttribute("hardware", objname, "warranty", myDateStr))
        r.cvoid(setAttribute("hardware", objname, "purchase", myDateStr))
        r.cvoid(setAttribute("hardware", objname, "vendor", myVendor))
        r.cvoid(setAttribute("hardware", objname, "cpu_num", myCPUs))
        r.cvoid(setAttribute("hardware", objname, "ram", myRAM))
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

    # filter by date
    matching = deska.hardware[deska.hardware.warranty >= '2011-04-03']
