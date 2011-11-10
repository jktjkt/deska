'''Test multipleObjectData with revisions with pre-existing data'''

import copy
from apiUtils import *

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
        if not isinstance(v, list):
            res[k] = v
        else:
            res[k] = v[1]
    return res

def imperative(r):
    do_hardware(r)
    do_host(r)

def do_hardware(r):
    # Start with creating some objects
    r.c(startChangeset())
    for obj in ["vendor1", "vendor2"]:
        r.assertEqual(r.c(createObject("vendor", obj)), obj)

    r.assertEqual(r.c(createObject("hardware", "hw3")), "hw3")
    r.cvoid(setAttribute("hardware", "hw3", "vendor", "vendor2"))
    r.cvoid(setAttribute("hardware", "hw3", "purchase", "2010-10-10"))
    r.cvoid(setAttribute("hardware", "hw3", "warranty", "2012-10-10"))

    # Make sure that their data is correct
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

    # Now let's see how templates come into play. Let's inherit three attributes.
    r.c(startChangeset())
    r.assertEqual(r.c(createObject("hardware_template", "t1")), "t1")
    r.cvoid(setAttribute("hardware_template", "t1", "cpu_num", 666))
    r.cvoid(setAttribute("hardware_template", "t1", "cpu_ht", True))
    r.cvoid(setAttribute("hardware_template", "t1", "ram", 333))
    r.cvoid(setAttribute("hardware", "hw3", "template_hardware", "t1"))

    hw3_2 = copy.deepcopy(hw3_1)
    hw3_2["template_hardware"] = "t1"
    hw3_2["cpu_num"] = ["t1", 666]
    hw3_2["cpu_ht"] = ["t1", True]
    hw3_2["ram"] = ["t1", 333]

    r.assertEqual(r.c(resolvedObjectData("hardware", "hw3")), strip_origin(hw3_2))
    r.assertEqual(r.c(resolvedObjectDataWithOrigin("hardware", "hw3")), hw3_2)
    # FIXME: Redmine #296, the value is reported as an integer, not as a full name
    hw3_2["template_hardware"] = 1
    r.assertEqual(r.c(multipleResolvedObjectData("hardware")), {"hw3": strip_origin(hw3_2)})
    # FIXME: Redmine #296, got to restore it back
    hw3_2["template_hardware"] = "t1"
    # FIXME: fails, Redmine #295
    #r.assertEqual(r.c(multipleResolvedObjectDataWithOrigin("hardware")), {"hw3": hw3_2})

    r.c(commitChangeset("test2"))
    # and test after a commit again
    r.assertEqual(r.c(resolvedObjectData("hardware", "hw3")), strip_origin(hw3_2))
    r.assertEqual(r.c(resolvedObjectDataWithOrigin("hardware", "hw3")), hw3_2)
    # FIXME: Redmine #296, the value is reported as an integer, not as a full name
    hw3_2["template_hardware"] = 1
    r.assertEqual(r.c(multipleResolvedObjectData("hardware")), {"hw3": strip_origin(hw3_2)})
    # FIXME: Redmine #296, got to restore it back
    hw3_2["template_hardware"] = "t1"
    # FIXME: fails, Redmine #295
    #r.assertEqual(r.c(multipleResolvedObjectDataWithOrigin("hardware")), {"hw3": hw3_2})

    # Let's see what happens when we override an inherited attribute
    r.c(startChangeset())
    r.cvoid(setAttribute("hardware", "hw3", "cpu_ht", False))
    hw3_3 = copy.deepcopy(hw3_2)
    hw3_3["cpu_ht"] = ["hw3", False]
    r.assertEqual(r.c(resolvedObjectData("hardware", "hw3")), strip_origin(hw3_3))
    r.assertEqual(r.c(resolvedObjectDataWithOrigin("hardware", "hw3")), hw3_3)
    # FIXME: Redmine #296, the value is reported as an integer, not as a full name
    hw3_3["template_hardware"] = 1
    r.assertEqual(r.c(multipleResolvedObjectData("hardware")), {"hw3": strip_origin(hw3_3)})
    # FIXME: Redmine #296, got to restore it back
    hw3_3["template_hardware"] = "t1"
    # FIXME: fails, Redmine #295
    #r.assertEqual(r.c(multipleResolvedObjectDataWithOrigin("hardware")), {"hw3": hw3_3})

    r.c(commitChangeset("test2"))
    # and test after a commit again
    r.assertEqual(r.c(resolvedObjectData("hardware", "hw3")), strip_origin(hw3_3))
    r.assertEqual(r.c(resolvedObjectDataWithOrigin("hardware", "hw3")), hw3_3)
    # FIXME: Redmine #296, the value is reported as an integer, not as a full name
    hw3_3["template_hardware"] = 1
    r.assertEqual(r.c(multipleResolvedObjectData("hardware")), {"hw3": strip_origin(hw3_3)})
    # FIXME: Redmine #296, got to restore it back
    hw3_3["template_hardware"] = "t1"
    # FIXME: fails, Redmine #295
    #r.assertEqual(r.c(multipleResolvedObjectDataWithOrigin("hardware")), {"hw3": hw3_3})


    # See what happens when we switch it to derive from another template
    r.c(startChangeset())
    r.assertEqual(r.c(createObject("hardware_template", "t2")), "t2")
    r.cvoid(setAttribute("hardware_template", "t2", "cpu_num", 4))
    r.cvoid(setAttribute("hardware_template", "t2", "ram", 1024))
    r.cvoid(setAttribute("hardware", "hw3", "template_hardware", "t2"))
    hw3_4 = copy.deepcopy(hw3_1)
    # this is our overriden attribute
    hw3_4["cpu_ht"] = ["hw3", False]
    hw3_4["cpu_num"] = ["t2", 4]
    hw3_4["ram"] = ["t2", 1024]
    hw3_4["template_hardware"] = "t2"
    r.assertEqual(r.c(resolvedObjectData("hardware", "hw3")), strip_origin(hw3_4))
    r.assertEqual(r.c(resolvedObjectDataWithOrigin("hardware", "hw3")), hw3_4)
    # FIXME: Redmine #296, the value is reported as an integer, not as a full name
    hw3_4["template_hardware"] = 2
    r.assertEqual(r.c(multipleResolvedObjectData("hardware")), {"hw3": strip_origin(hw3_4)})
    # FIXME: Redmine #296, got to restore it back
    hw3_4["template_hardware"] = "t2"
    # FIXME: fails, Redmine #295
    #r.assertEqual(r.c(multipleResolvedObjectDataWithOrigin("hardware")), {"hw3": hw3_4})
    r.c(commitChangeset("test2"))
    # and test after a commit again
    r.assertEqual(r.c(resolvedObjectData("hardware", "hw3")), strip_origin(hw3_4))
    r.assertEqual(r.c(resolvedObjectDataWithOrigin("hardware", "hw3")), hw3_4)
    # FIXME: Redmine #296, the value is reported as an integer, not as a full name
    hw3_4["template_hardware"] = 2
    r.assertEqual(r.c(multipleResolvedObjectData("hardware")), {"hw3": strip_origin(hw3_4)})
    # FIXME: Redmine #296, got to restore it back
    hw3_4["template_hardware"] = "t2"
    # FIXME: fails, Redmine #295
    #r.assertEqual(r.c(multipleResolvedObjectDataWithOrigin("hardware")), {"hw3": hw3_4})

    # Let's play with a chain of inheritance
    r.c(startChangeset())
    r.cvoid(setAttribute("hardware_template", "t2", "template_hardware", "t1"))
    hw3_5 = copy.deepcopy(hw3_3)
    hw3_5["cpu_num"] = ["t2", 4]
    hw3_5["ram"] = ["t2", 1024]
    hw3_5["template_hardware"] = "t2"
    r.assertEqual(r.c(resolvedObjectData("hardware", "hw3")), strip_origin(hw3_5))
    r.assertEqual(r.c(resolvedObjectDataWithOrigin("hardware", "hw3")), hw3_5)
    # FIXME: Redmine #296, the value is reported as an integer, not as a full name
    hw3_5["template_hardware"] = 2
    r.assertEqual(r.c(multipleResolvedObjectData("hardware")), {"hw3": strip_origin(hw3_5)})
    # FIXME: Redmine #296, got to restore it back
    hw3_5["template_hardware"] = "t2"
    # FIXME: fails, Redmine #295
    #r.assertEqual(r.c(multipleResolvedObjectDataWithOrigin("hardware")), {"hw3": hw3_5})
    r.c(commitChangeset("test2"))
    # and test after a commit again
    r.assertEqual(r.c(resolvedObjectData("hardware", "hw3")), strip_origin(hw3_5))
    r.assertEqual(r.c(resolvedObjectDataWithOrigin("hardware", "hw3")), hw3_5)
    # FIXME: Redmine #296, the value is reported as an integer, not as a full name
    hw3_5["template_hardware"] = 2
    r.assertEqual(r.c(multipleResolvedObjectData("hardware")), {"hw3": strip_origin(hw3_5)})
    # FIXME: Redmine #296, got to restore it back
    hw3_5["template_hardware"] = "t2"
    # FIXME: fails, Redmine #295
    #r.assertEqual(r.c(multipleResolvedObjectDataWithOrigin("hardware")), {"hw3": hw3_5})

    # See what happens when we break the chain -- that should be the same as hw3_4
    r.c(startChangeset())
    r.cvoid(setAttribute("hardware_template", "t2", "template_hardware", None))
    r.assertEqual(r.c(resolvedObjectData("hardware", "hw3")), strip_origin(hw3_4))
    r.assertEqual(r.c(resolvedObjectDataWithOrigin("hardware", "hw3")), hw3_4)
    # FIXME: Redmine #296, the value is reported as an integer, not as a full name
    hw3_4["template_hardware"] = 2
    r.assertEqual(r.c(multipleResolvedObjectData("hardware")), {"hw3": strip_origin(hw3_4)})
    # FIXME: Redmine #296, got to restore it back
    hw3_4["template_hardware"] = "t2"
    # FIXME: fails, Redmine #295
    #r.assertEqual(r.c(multipleResolvedObjectDataWithOrigin("hardware")), {"hw3": hw3_4})
    r.c(commitChangeset("test2"))
    # and test after a commit again
    r.assertEqual(r.c(resolvedObjectData("hardware", "hw3")), strip_origin(hw3_4))
    r.assertEqual(r.c(resolvedObjectDataWithOrigin("hardware", "hw3")), hw3_4)
    # FIXME: Redmine #296, the value is reported as an integer, not as a full name
    hw3_4["template_hardware"] = 2
    r.assertEqual(r.c(multipleResolvedObjectData("hardware")), {"hw3": strip_origin(hw3_4)})
    # FIXME: Redmine #296, got to restore it back
    hw3_4["template_hardware"] = "t2"
    # FIXME: fails, Redmine #295
    #r.assertEqual(r.c(multipleResolvedObjectDataWithOrigin("hardware")), {"hw3": hw3_4})

def do_host(r):
    r.c(startChangeset())
    for service in ["a", "b", "c"]:
        r.assertEqual(r.c(createObject("service", service)), service)
    r.c(createObject("host", "h"))
    r.c(createObject("host_template", "t1"))
    r.cvoid(setAttribute("host", "h", "template_host", "t1"))
    r.cvoid(setAttribute("host_template", "t1", "service", ["a"]))

    hdata = {
        "hardware": [None, None],
        "note_host": [None, None],
        # FIXME: Redmine #294
        #"template_host": ["h", "t1"],
        "template_host": "t1",
        "service": ["t1", ["a"]],
    }

    r.assertEqual(r.c(resolvedObjectData("host", "h")), strip_origin(hdata))
    r.assertEqual(r.c(resolvedObjectDataWithOrigin("host", "h")), hdata)
    # FIXME: Redmine #296, the value is reported as an integer, not as a full name
    hdata["template_host"] = 1
    # FIXME: Redmine #297, identifier_set values are not propagated at all
    # r.assertEqual(r.c(multipleResolvedObjectData("host")), {"h": strip_origin(hdata)})
    # FIXME: Redmine #296, change it back
    hdata["template_host"] = "t1"
    # FIXME: fails, Redmine #295
    #r.assertEqual(r.c(multipleResolvedObjectDataWithOrigin("host")), {"h": hdata})
    # test after a commit
    r.c(commitChangeset("."))
    r.assertEqual(r.c(resolvedObjectData("host", "h")), strip_origin(hdata))
    r.assertEqual(r.c(resolvedObjectDataWithOrigin("host", "h")), hdata)
    # FIXME: Redmine #296, the value is reported as an integer, not as a full name
    hdata["template_host"] = 1
    # FIXME: Redmine #297, identifier_set values are not propagated at all
    # r.assertEqual(r.c(multipleResolvedObjectData("host")), {"h": strip_origin(hdata)})
    # FIXME: Redmine #296, change it back
    hdata["template_host"] = "t1"
    # FIXME: fails, Redmine #295
    #r.assertEqual(r.c(multipleResolvedObjectDataWithOrigin("host")), {"h": hdata})
