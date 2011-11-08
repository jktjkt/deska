'''Test multipleObjectData with revisions with pre-existing data'''

from apiUtils import *

expectedHardwareData2 = {
    "hw3": {"vendor": "vendor2", "warranty": "2010-10-10", "purchase": "2012-10-10", "cpu_num": None, "cpu_ht": None, "ram": None, "note_hardware": None, "template_hardware": None, "host" : None},
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

hw3_1 = {
    'cpu_ht': [None, None],
    'cpu_num': [None, None],
    'host': [None, None],
    'note_hardware': [None, None],
    'purchase': ['hw3', '2010-10-10'],
    'ram': [None, None],
    # FIXME: Redmine #294
    #'template_hardware': [None, None],
    'template_hardware': None,
    'vendor': ['hw3', 'vendor2'],
    'warranty': ['hw3', '2012-10-10']
}

def strip_origin(x):
    # FIXME: simplify this to a one-liner when #294 is fixed
    res = {}
    for k,v in x.iteritems():
        if v is None:
            res[k] = v
        else:
            res[k] = v[1]
    return res

def imperative(r):
    r.c(startChangeset())
    for obj in ["vendor1", "vendor2"]:
        r.assertEqual(r.c(createObject("vendor", obj)), obj)

    r.assertEqual(r.c(createObject("hardware", "hw3")), "hw3")
    r.cvoid(setAttribute("hardware", "hw3", "vendor", "vendor2"))
    r.cvoid(setAttribute("hardware", "hw3", "purchase", "2010-10-10"))
    r.cvoid(setAttribute("hardware", "hw3", "warranty", "2012-10-10"))

    # check it before commit
    r.assertEqual(r.c(resolvedObjectData("hardware", "hw3")), strip_origin(hw3_1))
    r.assertEqual(r.c(resolvedObjectDataWithOrigin("hardware", "hw3")), hw3_1)
    r.assertEqual(r.c(multipleResolvedObjectData("hardware")), {"hw3": strip_origin(hw3_1)})
    # FIXME: fails, Redmine #295
    #r.assertEqual(r.c(multipleResolvedObjectDataWithOrigin("hardware")), {"hw3": hw3_1})

    r.c(commitChangeset("test2"))

    # now repeat the checks after a commit
    r.assertEqual(r.c(resolvedObjectData("hardware", "hw3")), strip_origin(hw3_1))
    r.assertEqual(r.c(resolvedObjectDataWithOrigin("hardware", "hw3")), hw3_1)
    r.assertEqual(r.c(multipleResolvedObjectData("hardware")), {"hw3": strip_origin(hw3_1)})
    # FIXME: fails, Redmine #295
    #r.assertEqual(r.c(multipleResolvedObjectDataWithOrigin("hardware")), {"hw3": hw3_1})

    # now let's see how templates come into play here

    # FIXME: write more code


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

