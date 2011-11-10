'''Test basic features of the deska module'''

from apiUtils import *
import datetime

myHw = {}

def imperative(r):
    prepareObjects(r)
    doTests(r)

def prepareObjects(r):
    '''Seed the DB with some data'''

    realVendors = ["HP", "IBM", "SGI"]
    myVendors = realVendors + [None]
    r.c(startChangeset())
    for x in range(10):
        objname = "x%d" % x
        r.assertEqual(r.c(createObject("host", objname)), objname)
        r.cvoid(setAttribute("host", objname, "note_host", "ahoj"))
    for vendor in realVendors:
        r.assertEqual(r.c(createObject("vendor", vendor)), vendor)
    for x in range(5):
        objname = "hw_%d" % x
        myDate = x+1
        myDateStr = "2011-04-%02d" % myDate
        myVendor = myVendors[x % len(myVendors)]
        myCPUs = str((x / 4) + 1)
        myRAM = str((x / 2) + 2)
        myHw[objname] = {"date": myDate, "vendor": myVendor, "cpu": myCPUs,
                         "ram": myRAM}
        r.assertEqual(r.c(createObject("hardware", objname)), objname)
        r.cvoid(setAttribute("hardware", objname, "warranty", myDateStr))
        r.cvoid(setAttribute("hardware", objname, "purchase", myDateStr))
        r.cvoid(setAttribute("hardware", objname, "vendor", myVendor))
        r.cvoid(setAttribute("hardware", objname, "cpu_num", myCPUs))
        r.cvoid(setAttribute("hardware", objname, "ram", myRAM))
    r.c(commitChangeset("objects set up"))

def doTests(r):
    '''Test that we get reasonable data back'''
    import deska
    deska.init()

    expectedNames = ["x%d" % x for x in range(10)]
    r.assertEqual(sorted(r.c(kindInstances("host"))), expectedNames)
    # the following should match all objects in the DB
    matching = deska.host[deska.host.note_host == "ahoj"]
    r.assertEqual(sorted(matching.keys()), expectedNames)
    # check the actual object data
    for x in matching.itervalues():
        r.assertEqual(x.hardware, None)
        r.assertEqual(x.note_host, "ahoj")
    # now ask for the same, but without an artificial filter
    matching = deska.host._all()
    r.assertEqual(len(matching), len(expectedNames))
    for x in matching.itervalues():
        r.assertEqual(x.hardware, None)
        r.assertEqual(x.note_host, "ahoj")

    # make sure that the following returns no results
    r.assertEqual(deska.host[deska.host.note_host != "ahoj"].keys(), [])
    r.assertEqual(deska.host[deska.host.note_host == "bla"].keys(), [])

    # filter by date and CMP_GE; the date is specified as string
    matching = deska.hardware[deska.hardware.warranty >= '2011-04-03']
    r.assertEqual(sorted(matching.keys()),
                  sorted([k for (k, v) in myHw.iteritems() if v["date"] >= 3]))
    # filter by date and CMP_GT; the date is now specified properly as a native type
    matching = deska.hardware[deska.hardware.warranty > datetime.date(2011, 4, 3)]
    r.assertEqual(sorted(matching.keys()),
                  sorted([k for (k, v) in myHw.iteritems() if v["date"] > 3]))
    # filter by date and CMP_LE; the date is specified as string
    matching = deska.hardware[deska.hardware.warranty <= '2011-04-03']
    r.assertEqual(sorted(matching.keys()),
                  sorted([k for (k, v) in myHw.iteritems() if v["date"] <= 3]))
    # filter by date and CMP_LT; the date is now specified properly as a native type
    matching = deska.hardware[deska.hardware.warranty < datetime.date(2011, 4, 3)]
    r.assertEqual(sorted(matching.keys()),
                  sorted([k for (k, v) in myHw.iteritems() if v["date"] < 3]))

    # ask for a specific vendor
    matching = deska.hardware[deska.hardware.vendor == "IBM"]
    r.assertEqual(sorted(matching.keys()),
                  sorted([k for (k, v) in myHw.iteritems() if v["vendor"] == "IBM"]))
    # ask for a specific vendor in an indirect way
    matching = deska.hardware[deska.vendor.name == "IBM"]
    r.assertEqual(sorted(matching.keys()),
                  sorted([k for (k, v) in myHw.iteritems() if v["vendor"] == "IBM"]))

    # ask for a different vendor
    matching = deska.hardware[deska.hardware.vendor != "IBM"]
    r.assertEqual(sorted(matching.keys()),
                  sorted([k for (k, v) in myHw.iteritems() if v["vendor"] != "IBM"]))

    # ask for a different vendor indirectly
    matching = deska.hardware[deska.vendor.name != "IBM"]
    r.assertEqual(sorted(matching.keys()),
                  sorted([k for (k, v) in myHw.iteritems() if v["vendor"] != "IBM"]))

    # ask for HW with known vendor
    matching = deska.hardware[deska.hardware.vendor != None]
    r.assertEqual(sorted(matching.keys()),
                  sorted([k for (k, v) in myHw.iteritems() if v["vendor"] is not None]))
    # Now the same, but indirectly
    matching = deska.hardware[deska.vendor.name != None]
    r.assertEqual(sorted(matching.keys()),
                  sorted([k for (k, v) in myHw.iteritems() if v["vendor"] is not None]))

    # ask for HW with UNKNOWN vendor
    matching = deska.hardware[deska.hardware.vendor == None]
    r.assertEqual(sorted(matching.keys()),
                  sorted([k for (k, v) in myHw.iteritems() if v["vendor"] is None]))
    # Now the same, but indirectly
    matching = deska.hardware[deska.vendor.name == None]
    r.assertEqual(sorted(matching.keys()),
                  sorted([k for (k, v) in myHw.iteritems() if v["vendor"] is None]))

    # Just enumerate all of the HW
    # FIXME: fails, Redmine #298
    #matching = deska.vendor._all()
    #r.assertEqual(sorted(matchig.keys()), sorted(["HP", "IBM", "SGI"]))
