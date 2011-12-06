'''Test createObject, setAttribute, objectData, objectData with revisions with pre-existing data'''

from apiUtils import *

expectedHardwareData = {"vendor": "vendor1", "warranty": "2010-10-10", "purchase": "2008-10-10", "cpu_num": None, "cpu_ht": True,
                        "hepspec": None, "ram": None, "note_hardware": None, "template_hardware": None, "host": None}
expectedHardwareData2 = {"vendor": "vendor2", "warranty": "2010-10-10","purchase": "2008-10-10", "cpu_num": None, "cpu_ht": False,
                         "hepspec": 1.0/4, "ram": None, "note_hardware": None, "template_hardware": None, "host": None}
expectedInterfaceData = {"ip4": None, "ip6": None, "mac": "01:23:45:67:89:ab", "note": "host1->eth0 note", "template_interface": None, "network": None}
expectedInterfaceData2 = {"ip4": None, "ip6": None, "mac": "01:23:45:67:89:aa", "note": "host1->eth0 note", "template_interface": None, "network": None}

def imperative(r):
    doStuff(r)
    doStuff_embed(r)

def doStuff(r):
    vendorNames = set(["vendor1", "vendor2"])

    r.c(startChangeset())
    for obj in vendorNames:
        r.assertEqual(r.c(createObject("vendor", obj)), obj)

    hardwareNames = set(["hw1"])

    for obj in hardwareNames:
        r.assertEqual(r.c(createObject("hardware", obj)), obj)

    r.cvoid(setAttribute("hardware", "hw1", "vendor", "vendor1"))
    r.cvoid(setAttribute("hardware", "hw1", "purchase", "2008-10-10"))
    r.cvoid(setAttribute("hardware", "hw1", "warranty", "2010-10-10"))
    r.cvoid(setAttribute("hardware", "hw1", "cpu_ht", True))

    hardwareData = r.c(objectData("hardware", "hw1"))
    r.assertEqual(hardwareData, expectedHardwareData)
    revision = r.c(commitChangeset("test"))

    hardwareData = r.c(objectData("hardware", "hw1", revision))
    r.assertEqual(hardwareData, expectedHardwareData)
    r.assertTrue("hwNoneExisting" not in r.c(kindInstances("hardware")))
    r.cfail(objectData("hardware", "hwNoneExisting"), NotFoundError())

    r.c(startChangeset())
    r.cvoid(setAttribute("hardware", "hw1", "vendor", "vendor2"))
    r.cvoid(setAttribute("hardware", "hw1", "hepspec", 1.0/4))
    r.cvoid(setAttribute("hardware", "hw1", "cpu_ht", False))
    hardwareData = r.c(objectData("hardware", "hw1"))
    r.assertEqual(hardwareData, expectedHardwareData2)
    hardwareData = r.c(objectData("hardware", "hw1", revision))
    r.assertEqual(hardwareData, expectedHardwareData)
    r.c(commitChangeset("test2"))

    hardwareData = r.c(objectData("hardware", "hw1", revision))
    r.assertEqual(hardwareData, expectedHardwareData)
    hardwareData = r.c(objectData("hardware", "hw1"))
    r.assertEqual(hardwareData, expectedHardwareData2)

    r.c(startChangeset())
    r.cvoid(renameObject("hardware", "hw1", "hwNewName"))
    hardwareData = r.c(objectData("hardware", "hwNewName"))
    r.assertEqual(hardwareData, expectedHardwareData2)
    r.c(commitChangeset("test"))
    hardwareData = r.c(objectData("hardware", "hwNewName"))
    r.assertEqual(hardwareData, expectedHardwareData2)
    r.cfail(objectData("hardware", "hw1"), NotFoundError())

    r.c(startChangeset())
    r.c(createObject("host", "dummy"))
    r.cfail(setAttribute("host", "dummy", "service", 123), NotASetError())
    r.cfail(setAttribute("host", "dummy", "note_host", [123]), NotASetError())
    r.cfail(setAttributeInsert("host", "dummy", "note_host", [123]), NotASetError())
    r.cfail(setAttributeRemove("host", "dummy", "note_host", [123]), NotASetError())
    r.cfail(setAttributeInsert("host", "dummy", "service", [123]), NotASetError())
    r.cfail(setAttributeRemove("host", "dummy", "service", [123]), NotASetError())
    r.cvoid(abortCurrentChangeset())



def doStuff_embed(r):
    hostNames = set(["host1", "host2"])
    presentHosts = set(r.c(kindInstances("host")))
    hostNames = hostNames - presentHosts

    r.c(startChangeset())
    for obj in hostNames:
        r.assertEqual(r.c(createObject("host", obj)), obj)

    interfaceNames = set(["host1->eth0"])
    presentInterfaces = set(r.c(kindInstances("interface")))
    interfaceNames = interfaceNames - presentInterfaces

    for obj in interfaceNames:
        r.assertEqual(r.c(createObject("interface", obj)), obj)

    r.cvoid(setAttribute("interface", "host1->eth0", "note", "host1->eth0 note"))
    r.cvoid(setAttribute("interface", "host1->eth0", "mac", "01:23:45:67:89:ab"))

    interfaceData = r.c(objectData("interface", "host1->eth0"))
    r.assertEqual(interfaceData, expectedInterfaceData)
    revision = r.c(commitChangeset("test3"))

    interfaceData = r.c(objectData("interface", "host1->eth0", revision))
    r.assertEqual(interfaceData, expectedInterfaceData)
    if "host1->interfaceNoneExisting" not in r.c(kindInstances("interface")):
        r.cfail(objectData("interface", "host1->interfaceNoneExisting"), NotFoundError())

    r.c(startChangeset())
    r.cvoid(setAttribute("interface", "host1->eth0", "mac", "01:23:45:67:89:aa"))
    interfaceData = r.c(objectData("interface", "host1->eth0"))
    r.assertEqual(interfaceData, expectedInterfaceData2)
    interfaceData = r.c(objectData("interface", "host1->eth0", revision))
    r.assertEqual(interfaceData, expectedInterfaceData)
    r.c(commitChangeset("test4"))

    interfaceData = r.c(objectData("interface", "host1->eth0", revision))
    r.assertEqual(interfaceData, expectedInterfaceData)
    interfaceData = r.c(objectData("interface", "host1->eth0"))
    r.assertEqual(interfaceData, expectedInterfaceData2)

    r.c(startChangeset())
    r.cvoid(renameObject("interface", "host1->eth0", "host2->eth1"))
    interfaceData = r.c(objectData("interface", "host2->eth1"))
    r.assertEqual(interfaceData, expectedInterfaceData2)
    r.c(commitChangeset("test"))

    interfaceData = r.c(objectData("interface", "host2->eth1"))
    r.assertEqual(interfaceData, expectedInterfaceData2)
    r.cfail(objectData("interface", "host1->eth0"), NotFoundError())

