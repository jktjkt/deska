'''Test createObject, deleteObject, restoreDeletedObject with pre-existing data'''

from apiUtils import *

expectedHardwareData = {"vendor": "vendor1", "warranty": "2010-10-10", "purchase": "2008-10-10", "cpu_num": None, "cpu_ht": None, "ram": None, "note_hardware": None, "template_hardware": None, "host": None, "hepspec": None}
expectedInterfaceData = {"ip4": None, "ip6": None, "mac": "01:23:45:67:89:ab", "note": "host1->eth0 note", "template_interface": None, "network": None}

def imperative(r):
    doStuff(r)
    doStuff_embed(r)

def doStuff(r):
    vendorNames = set(["vendor1"])
    presentVendors = set(r.c(kindInstances("vendor")))
    vendorNames = vendorNames - presentVendors

    r.c(startChangeset())
    for obj in vendorNames:
        r.assertEqual(r.c(createObject("vendor", obj)), obj)

    hardwareNames = set(["hw1"])
    presentHW = set(r.c(kindInstances("hardware")))
    hardwareNames = hardwareNames - presentHW

    for obj in hardwareNames:
        r.assertEqual(r.c(createObject("hardware", obj)), obj)

    r.cvoid(setAttribute("hardware", "hw1", "vendor", "vendor1"))
    r.cvoid(setAttribute("hardware", "hw1", "purchase", "2008-10-10"))
    r.cvoid(setAttribute("hardware", "hw1", "warranty", "2010-10-10"))
    rev2 = r.c(commitChangeset("test"))

    hardwareData = r.c(objectData("hardware", "hw1"))
    r.assertEqual(hardwareData, expectedHardwareData)

    r.c(startChangeset())
    r.cvoid(deleteObject("hardware", "hw1"))
    r.cfail(objectData("hardware", "hw1"),NotFoundError())
    r.cfail(createObject("hardware", "hw1"),ReCreateObjectError())
    r.cvoid(restoreDeletedObject("hardware", "hw1"))
    hardwareData = r.c(objectData("hardware", "hw1"))
    r.assertEqual(hardwareData, expectedHardwareData)
    rev3 = r.c(commitChangeset("test2"))

    hardwareData = r.c(objectData("hardware", "hw1"))
    r.assertEqual(hardwareData, expectedHardwareData)

    # Check what happens when we try to rename an object to something which got
    # deleted in the current changeset
    changeset = r.c(startChangeset())
    r.c(createObject("vendor", "v2"))
    r.c(createObject("vendor", "v3"))
    rev4 = r.c(commitChangeset("."))
    changeset = r.c(startChangeset())
    r.cvoid(deleteObject("vendor", "v2"))
    r.cfail(renameObject("vendor", "v3", "v2"), exception=ReCreateObjectError())
    expectedDiff = [
        {'command': 'deleteObject', 'kindName': 'vendor', 'objectName': 'v2'},
    ]
    r.assertEqual(r.c(dataDifferenceInTemporaryChangeset(changeset)), expectedDiff)
    rev5 = r.c(commitChangeset("."))
    r.assertEqual(r.c(dataDifference(rev4, rev5)), expectedDiff)

    # Check what happens when setting attributes which belong to a just-deleted host
    # Set up the data
    r.c(startChangeset())
    r.c(createObject("host", "h1"))
    r.c(createObject("interface", "h1->i1"))
    rev6 = r.c(commitChangeset("added host and interface"))

    # Check how it works for a host
    changeset = r.c(startChangeset())
    r.cvoid(deleteObject("host", "h1"))
    r.cvoid(setAttribute("host", "h1", "note_host", "pwn"))
    r.assertEqual(r.c(dataDifferenceInTemporaryChangeset(changeset)),
                  [{'command': 'deleteObject', 'kindName': 'host', 'objectName': 'h1'}])
    r.cvoid(abortCurrentChangeset())

    # See how the embedded objects behave
    changeset = r.c(startChangeset())
    r.cvoid(deleteObject("interface", "h1->i1"))
    r.cvoid(setAttribute("interface", "h1->i1", "note", "pwn"))
    r.assertEqual(r.c(dataDifferenceInTemporaryChangeset(changeset)),
                  [{'command': 'deleteObject', 'kindName': 'interface', 'objectName': 'h1->i1'}])
    r.cvoid(abortCurrentChangeset())

    # Now try the absolutely same thing but with calling applyBatchedChanges()
    # instead of plain old setAttribute
    changeset = r.c(startChangeset())
    r.cvoid(deleteObject("interface", "h1->i1"))
    r.cvoid(applyBatchedChanges([{'attributeName': 'note', 'kindName': 'interface', 'command': 'setAttribute',
                                  'attributeData': 'pwn', 'objectName': 'h1->i1'}]))
    r.assertEqual(r.c(dataDifferenceInTemporaryChangeset(changeset)),
                  [{'command': 'deleteObject', 'kindName': 'interface', 'objectName': 'h1->i1'}])
    r.cvoid(abortCurrentChangeset())

def doStuff_embed(r):
    hostNames = set(["host1"])
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

