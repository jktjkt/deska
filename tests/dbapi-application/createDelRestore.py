'''Test createObject, deleteObject, restoreDeletedObject with pre-existing data'''

from apiUtils import *

expectedHardwareData = {"vendor": "vendor1", "warranty": "2010-10-10", "purchase": "2008-10-10", "cpu_num": None, "ram": None, "hardware_note": None, "template": None, "host": None}
expectedInterfaceData = {"ip4": None, "ip6": None, "mac": "01:23:45:67:89:ab", "note": "host1->eth0 note", "template": None}

def imperative(r):
    doStuff(r)
    doStuff_embed(r)

def doStuff(r):
    vendorNames = set(["vendor1"])
    presentVendors = set(r.c(kindInstances("vendor")))
    vendorNames = vendorNames - presentVendors

    r.c(startChangeset())
    for obj in vendorNames:
        r.cvoid(createObject("vendor", obj))

    hardwareNames = set(["hw1"])
    presentHW = set(r.c(kindInstances("hardware")))
    hardwareNames = hardwareNames - presentHW

    for obj in hardwareNames:
        r.cvoid(createObject("hardware", obj))

    r.cvoid(setAttribute("hardware", "hw1", "vendor", "vendor1"))
    r.cvoid(setAttribute("hardware", "hw1", "purchase", "2008-10-10"))
    r.cvoid(setAttribute("hardware", "hw1", "warranty", "2010-10-10"))
    r.c(commitChangeset("test"))

    hardwareData = r.c(objectData("hardware", "hw1"))
    r.assertEqual(hardwareData, expectedHardwareData)

    r.c(startChangeset())
    r.cvoid(deleteObject("hardware", "hw1"))
    r.cfail(objectData("hardware", "hw1"),NotFoundError())
    r.cfail(createObject("hardware", "hw1"),ReCreateObjectError())
    r.cvoid(restoreDeletedObject("hardware", "hw1"))
    hardwareData = r.c(objectData("hardware", "hw1"))
    r.assertEqual(hardwareData, expectedHardwareData)    
    r.c(commitChangeset("test2"))

    hardwareData = r.c(objectData("hardware", "hw1"))
    r.assertEqual(hardwareData, expectedHardwareData)

def doStuff_embed(r):
    hostNames = set(["host1"])
    presentHosts = set(r.c(kindInstances("host")))
    hostNames = hostNames - presentHosts

    r.c(startChangeset())
    for obj in hostNames:
        r.cvoid(createObject("host", obj))

    interfaceNames = set(["host1->eth0"])
    presentInterfaces = set(r.c(kindInstances("interface")))
    interfaceNames = interfaceNames - presentInterfaces

    for obj in interfaceNames:
        r.cvoid(createObject("interface", obj))

    r.cvoid(setAttribute("interface", "host1->eth0", "note", "host1->eth0 note"))
    r.cvoid(setAttribute("interface", "host1->eth0", "mac", "01:23:45:67:89:ab"))
    r.c(commitChangeset("test3"))

    interfaceData = r.c(objectData("interface", "host1->eth0"))
    r.assertEqual(interfaceData, expectedInterfaceData)

    r.c(startChangeset())
    r.cvoid(deleteObject("interface", "host1->eth0"))
    r.cfail(objectData("interface", "host1->eth0"), NotFoundError())
    r.cfail(createObject("interface", "host1->eth0"),ReCreateObjectError())
    r.cvoid(restoreDeletedObject("interface", "host1->eth0"))
  
    interfaceData = r.c(objectData("interface", "host1->eth0"))
    r.assertEqual(interfaceData, expectedInterfaceData)
    r.c(commitChangeset("test4"))

    interfaceData = r.c(objectData("interface", "host1->eth0"))
    r.assertEqual(interfaceData, expectedInterfaceData)

