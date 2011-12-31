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
    r.c(createObject("service", "even"))
    r.c(createObject("service", "odd"))
    r.c(createObject("service", "first_half"))
    r.c(createObject("service", "second_half"))
    for x in range(10):
        objname = "x%d" % x
        r.assertEqual(r.c(createObject("host", objname)), objname)
        r.cvoid(setAttribute("host", objname, "note_host", "ahoj"))
        if x < 5:
            r.cvoid(setAttributeInsert("host", "x%d" % x, "service", "first_half"))
        else:
            r.cvoid(setAttributeInsert("host", "x%d" % x, "service", "second_half"))
        if x % 2:
            r.cvoid(setAttributeInsert("host", "x%d" % x, "service", "even"))
        else:
            r.cvoid(setAttributeInsert("host", "x%d" % x, "service", "odd"))

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

    r.c(createObject("network", "nat"))
    r.cvoid(setAttribute("network", "nat", "ip4", "172.16.0.0"))
    r.cvoid(setAttribute("network", "nat", "cidr4", 16))
    r.cvoid(setAttribute("network", "nat", "vlan", 172))
    r.c(createObject("network", "public"))
    r.cvoid(setAttribute("network", "public", "ip4", "147.231.25.0"))
    r.cvoid(setAttribute("network", "public", "cidr4", 24))
    r.cvoid(setAttribute("network", "public", "vlan", 25))

    for x in range(0, 4):
        hostname = "x%d" % x
        iface = "%s->eth0" % hostname
        r.c(createObject("interface", iface))
        r.cvoid(setAttribute("interface", iface, "ip4", "147.231.25.%d" % (x*3+1)))
        r.cvoid(setAttribute("interface", iface, "network", "public"))
    for x in range(4, 9):
        hostname = "x%d" % x
        iface = "%s->eth0" % hostname
        r.c(createObject("interface", iface))
        r.cvoid(setAttribute("interface", iface, "ip4", "172.16.0.%d" % (x*3+1)))
        r.cvoid(setAttribute("interface", iface, "network", "nat"))
    for x in [9]:
        hostname = "x%d" % x
        iface = "%s->eth0" % hostname
        r.c(createObject("interface", iface))
        r.cvoid(setAttribute("interface", iface, "ip4", "192.168.12.%d" % (x*3+1)))
    r.c(createObject("interface", "x0->eth1"))


    r.c(commitChangeset("objects set up"))

def doTests(r):
    '''Test that we get reasonable data back'''
    import deska
    import libLowLevelPyDeska as _l
    args = _l.std_vector_string()
    args.append(r.path_deska_server_bin)
    deska.init(_l.Connection(args))

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
    matching = deska.vendor._all()
    r.assertEqual(sorted(matching.keys()), sorted(["HP", "IBM", "SGI"]))

    # Now try to perform that we can search from the other way round
    matching = deska.vendor[deska.hardware.ram == 3]
    # The Python code has to eliminate None objects from the list, though,
    # because from the DBAPI point of view it arguably doesn't make sense to
    # return null objects in this context.  There is simply no allowed way for
    # the DBAPI protocol to return nulls in this context.
    r.assertEqual(sorted(matching.keys()),
                  sorted([v["vendor"] for (k, v) in myHw.iteritems()
                          if v["ram"] == "3" and v["vendor"] is not None]))

    # Indirect check: check for all interface in a network with particular VLAN
    matching = deska.interface[deska.network.vlan == 25]
    r.assertEqual(sorted(matching.keys()), sorted(["x%d->eth0" % x for x in range(0,4)]))

    # Indirect check combined with another condition
    matching = deska.interface[(deska.interface.ip4 != None) &
                               (deska.network.vlan == 25)]
    r.assertEqual(sorted(matching.keys()), sorted(["x%d->eth0" % x for x in range(0,4)]))

    # Indirect and non-equivalence
    matching = deska.interface[deska.network.vlan != 25]
    r.assertEqual(sorted(matching.keys()), sorted(["x%d->eth0" % x for x in range(4,10)] + ["x0->eth1"]))

    matching = deska.interface[deska.host.name == "x0"]
    r.assertEqual(sorted(matching.keys()), ["x0->eth0", "x0->eth1"])

    # Indirect query combined with an == on interface name
    matching = deska.interface[(deska.host.name == "x0") &
                               (deska.interface.name == "eth0")]
    r.assertEqual(sorted(matching.keys()), ["x0->eth0"])

    # Indirect query combined with an != on interface name
    # FIXME: fails, produces invalid data: Redmine#399, Redmine#400
    matching = deska.interface[(deska.host.name == "x0") &
                               (deska.interface.name != "eth0")]
    r.assertEqual(sorted(matching.keys()), ["x0->eth1"])

    # FIXME: fails, Redmine#400
    # Check for all other interfaces of a given host
    #matching = deska.interface[(deska.interface.host == "x0") &
    #                           (deska.interface.name != "x0->eth0")]
    #r.assertEqual(matching.keys(), ["x0->eth1"])
    # FIXME: fails, Redmine#399
    matching = deska.interface[(deska.interface.host == "x0") &
                               (deska.interface.name != "eth0")]
    r.assertEqual(matching.keys(), ["x0->eth1"])

    hosts_even = sorted(deska.host[deska.host.service.contains("even")].keys())
    r.assertEqual(hosts_even, ["x%d" % d for d in (1,3,5,7,9)])
    hosts_odd = sorted(deska.host[deska.host.service.contains("odd")].keys())
    r.assertEqual(hosts_odd, ["x%d" % d for d in (0,2,4,6,8)])
    hosts_first_half = sorted(deska.host[deska.host.service.contains("first_half")].keys())
    r.assertEqual(hosts_first_half, ["x%d" % d for d in range(5)])
    hosts_second_half = sorted(deska.host[deska.host.service.contains("second_half")].keys())
    r.assertEqual(hosts_second_half, ["x%d" % d for d in range(5, 10)])

    matching = deska.host[deska.host.service.contains("even") & deska.host.service.contains("odd")].keys()
    r.assertEqual(matching, [])
    matching = deska.host[deska.host.service.contains("even") | deska.host.service.contains("odd")].keys()
    r.assertEqual(sorted(matching), ["x%d" % x for x in range(10)])

    matching = deska.host[deska.host.service.contains("even") & deska.host.service.contains("first_half")].keys()
    r.assertEqual(sorted(matching), [x for x in hosts_even if x in hosts_first_half])
    matching = deska.host[deska.host.service.contains("even") | deska.host.service.contains("first_half")].keys()
    r.assertEqual(sorted(matching), sorted(list(set(hosts_even + hosts_first_half))))
