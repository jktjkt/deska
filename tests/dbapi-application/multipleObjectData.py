'''Test multipleObjectData with revisions with pre-existing data'''

from apiUtils import *

expectedHardwareData = {
    "hw1": {"vendor": "vendor1", "warranty": "2010-10-10", "purchase": "2008-10-10", "cpu_num": None, "cpu_ht": None, "ram": None, "note_hardware": None, "template_hardware": None, "host" : None},
    "hw2": {"vendor": "vendor1", "warranty": "2010-06-06", "purchase": "2006-06-06", "cpu_num": None, "cpu_ht": None, "ram": None, "note_hardware": None, "template_hardware": None, "host" : None}
}
expectedHardwareData2 = {
    "hw1": {"vendor": "vendor1", "warranty": "2010-10-10", "purchase": "2008-10-10", "cpu_num": None, "cpu_ht": None, "ram": None, "note_hardware": None, "template_hardware": None, "host" : None},
    "hw2": {"vendor": "vendor1", "warranty": "2006-06-06", "purchase": "2010-06-06", "cpu_num": None, "cpu_ht": None, "ram": None, "note_hardware": None, "template_hardware": None, "host" : None},
    "hw3": {"vendor": "vendor2", "warranty": "2010-10-10", "purchase": "2012-10-10", "cpu_num": None, "cpu_ht": None, "ram": None, "note_hardware": None, "template_hardware": None, "host" : None},
    "hw4": {"vendor": "vendor2", "warranty": "2010-10-10", "purchase": "2012-10-10", "cpu_num": None, "cpu_ht": None, "ram": None, "note_hardware": None, "template_hardware": None, "host" : None}
}

expectedInterfaceData = {
    "host1->eth0": {"note": "some note", "mac": "01:23:45:67:89:aa", "ip6": None, "ip4": "192.168.0.1", "template_interface": None},
    "host2->eth0": {"note": "another note", "mac": "01:23:45:67:89:bb", "ip6": None, "ip4": None, "template_interface": None}
}

expectedInterfaceData2 = {
    "host1->eth0": {"note": "some note", "mac": "01:23:45:67:89:ab", "ip6": None, "ip4": "192.168.0.1", "template_interface": None},
    "host2->eth0": {"note": "another note", "mac": "01:23:45:67:89:bb", "ip6": None, "ip4": None, "template_interface": None},
    "host1->eth1": {"note": None, "mac": "01:23:45:67:89:ba", "ip6": None, "ip4": None, "template_interface": None}
}

def imperative(r):
    doStuff(r)
    doStuff_embed(r)

def doStuff(r):
    vendorNames = set(["vendor1", "vendor2"])
    presentVendors = set(r.c(kindInstances("vendor")))
    vendorNames = vendorNames - presentVendors

    r.c(startChangeset())
    for obj in vendorNames:
        r.assertEqual(r.c(createObject("vendor", obj)), obj)

    hardwareNames = set(["hw1", "hw2"])
    presentHW = set(r.c(kindInstances("hardware")))
    hardwareNames = hardwareNames - presentHW

    for obj in hardwareNames:
        r.assertEqual(r.c(createObject("hardware", obj)), obj)

    r.cvoid(setAttribute("hardware", "hw1", "vendor", "vendor1"))
    r.cvoid(setAttribute("hardware", "hw1", "purchase", "2008-10-10"))
    r.cvoid(setAttribute("hardware", "hw1", "warranty", "2010-10-10"))
    r.cvoid(setAttribute("hardware", "hw2", "vendor", "vendor1"))
    r.cvoid(setAttribute("hardware", "hw2", "purchase", "2006-06-06"))
    r.cvoid(setAttribute("hardware", "hw2", "warranty", "2010-06-06"))

    hardwareData = r.c(multipleObjectData("hardware"))
    for key in hardwareData:
        r.assertEqual(hardwareData[key], expectedHardwareData[key])
    revision = r.c(commitChangeset("test"))

    hardwareData = r.c(multipleObjectData("hardware", revision))
    for key in hardwareData:
        r.assertEqual(hardwareData[key], expectedHardwareData[key])

    hardwareNames = set(["hw3","hw4"])
    presentHW = set(r.c(kindInstances("hardware")))
    hardwareNames = hardwareNames - presentHW

    r.c(startChangeset())

    for obj in hardwareNames:
        r.assertEqual(r.c(createObject("hardware", obj)), obj)

    r.cvoid(setAttribute("hardware", "hw3", "vendor", "vendor2"))
    r.cvoid(setAttribute("hardware", "hw3", "purchase", "2010-10-10"))
    r.cvoid(setAttribute("hardware", "hw3", "warranty", "2012-10-10"))
    r.cvoid(setAttribute("hardware", "hw4", "vendor", "vendor2"))
    r.cvoid(setAttribute("hardware", "hw4", "purchase", "2010-10-10"))
    r.cvoid(setAttribute("hardware", "hw4", "warranty", "2012-10-10"))
    r.c(commitChangeset("test2"))

    resolvedDataWithOrigin = r.c(resolvedObjectDataWithOrigin("hardware", "hw3"))
    r.assertEqual(resolvedDataWithOrigin, {
        'cpu_ht': [None, None],
        'cpu_num': [None, None],
        'host': [None, None],
        'note_hardware': [None, None],
        'purchase': ['hw3', '2010-10-10'],
        'ram': [None, None],
        # FIXed: Redmine #294
        'template_hardware': [None, None],
        'vendor': ['hw3', 'vendor2'],
        'warranty': ['hw3', '2012-10-10']
    })

def doStuff_embed(r):
    hostNames = set(["host1", "host2"])
    presentHosts = set(r.c(kindInstances("host")))
    hostNames = hostNames - presentHosts

    r.c(startChangeset())
    for obj in hostNames:
        r.assertEqual(r.c(createObject("host", obj)), obj)

    interfaceNames = set(["host1->eth0", "host2->eth0"])
    presentInterfaces = set(r.c(kindInstances("interface")))
    interfaceNames = interfaceNames - presentInterfaces

    for obj in interfaceNames:
        r.assertEqual(r.c(createObject("interface", obj)), obj)

    r.cvoid(setAttribute("interface", "host1->eth0", "mac", "01:23:45:67:89:aa"))
    r.cvoid(setAttribute("interface", "host1->eth0", "ip4", "192.168.0.1"))
    r.cvoid(setAttribute("interface", "host1->eth0", "note", "some note"))
    r.cvoid(setAttribute("interface", "host2->eth0", "mac", "01:23:45:67:89:bb"))
    r.cvoid(setAttribute("interface", "host2->eth0", "note", "another note"))

    interfaceData = r.c(multipleObjectData("interface"))
    for key in interfaceData:
        r.assertEqual(interfaceData[key], expectedInterfaceData[key])
    revision = r.c(commitChangeset("test"))

    interfaceData = r.c(multipleObjectData("interface", revision))
    for key in interfaceData:
        r.assertEqual(interfaceData[key], expectedInterfaceData[key])

    interfaceNames = set(["host1->eth1"])
    presentInterfaces = set(r.c(kindInstances("interface")))
    interfaceNames = interfaceNames - presentInterfaces

    r.c(startChangeset())
    for obj in interfaceNames:
        r.assertEqual(r.c(createObject("interface", obj)), obj)

    r.cvoid(setAttribute("interface", "host1->eth0", "mac", "01:23:45:67:89:ab"))
    r.cvoid(setAttribute("interface", "host1->eth1", "mac", "01:23:45:67:89:ba"))
    interfaceData = r.c(multipleObjectData("interface"))
    for key in interfaceData:
        r.assertEqual(interfaceData[key], expectedInterfaceData2[key])
    r.c(commitChangeset("test"))

    interfaceData = r.c(multipleObjectData("interface", revision))
    for key in interfaceData:
        r.assertEqual(interfaceData[key], expectedInterfaceData[key])

