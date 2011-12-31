'''Test multipleObjectData with revisions with pre-existing data'''

import copy
from apiUtils import *

def strip_origin(x):
    return dict((k, v[1]) for (k, v) in x.iteritems())

def imperative(r):
    r.maxDiff = None
    do_host(r)

def helper_check_host(r, expected):
    for name in expected.keys():
        r.assertEqual(r.c(resolvedObjectData("host", name)), strip_origin(expected[name]))
        r.assertEqual(r.c(resolvedObjectDataWithOrigin("host", name)), expected[name])
    r.assertEqual(r.c(multipleResolvedObjectData("host")), dict((k,strip_origin(v)) for (k,v) in expected.iteritems()))
    r.assertEqual(r.c(multipleResolvedObjectDataWithOrigin("host")), expected)

def do_host(r):
    changeset = r.c(startChangeset())
    r.c(createObject("virtual_hardware", "h"))
    r.c(commitChangeset("virt hw"))

    changeset = r.c(startChangeset())
    for service in ["a", "b", "c"]:
        r.assertEqual(r.c(createObject("service", service)), service)
    r.c(commitChangeset("."))
    changeset = r.c(startChangeset())
    r.c(createObject("host", "h"))
    r.cvoid(setAttribute("host", "h", "service", ["a"]))

    hdata = {"h":
        {
            "hardware": [None, None],
            "note": [None, None],
            "service": ["h", ["a"]],
            "virtual_hardware": ["h", "h"],
        }
    }

    helper_check_host(r, hdata)
    expectedResolved = [
        {"command": "createObject", "kindName": "host", "objectName": "h"},
        {"command": "setAttribute", "kindName": "host", "objectName": "h", "attributeName": "virtual_hardware", "oldAttributeData": None, "attributeData": "h"},
        {"command": "setAttribute", "kindName": "host", "objectName": "h", "attributeName": "service", "oldAttributeData": None, "attributeData": ["a"]},
        {"command": "setAttribute", "kindName": "virtual_hardware", "objectName": "h", "attributeName": "host", "oldAttributeData": None, "attributeData": "h"},
    ]
    r.assertEqual(r.c(resolvedDataDifferenceInTemporaryChangeset(changeset)), expectedResolved)
    r.assertEqual(r.c(dataDifferenceInTemporaryChangeset(changeset)), expectedResolved)

    # test after a commit
    rev = r.c(commitChangeset("."))
    helper_check_host(r, hdata)
    r.assertEqual(r.c(resolvedDataDifference(revisionIncrement(rev, -1), rev)), expectedResolved)
    r.assertEqual(r.c(dataDifference(revisionIncrement(rev, -1), rev)), expectedResolved)
