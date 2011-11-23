'''Test that Python's support for identifier sets works properly'''

from apiUtils import *
import datetime
import deska
import libLowLevelPyDeska as _l

def imperative(r):
    args = _l.std_vector_string()
    args.append(r.path_deska_server_bin)
    services = ["www", "ftp", "ui", "dpm", "imap"]
    r.c(startChangeset())
    for x in range(10):
        objname = "x%d" % x
        r.assertEqual(r.c(createObject("host", objname)), objname)
    for service in services:
        r.assertEqual(r.c(createObject("service", service)), service)
    r.c(commitChangeset("objects set up"))

    # set to a single-value list through an absolute name
    r.c(startChangeset())
    r.cvoid(setAttribute("host", "x0", "service", ["www"]))
    r.assertEqual(verifyingObjectMultipleData(r, "host", "x0")["service"], ["www"])
    r.c(commitChangeset("set host x0 service www"))

    r.assertEqual(verifyingObjectMultipleData(r, "host", "x0")["service"], ["www"])
    deska.init(_l.Connection(args))
    r.assertEqual(deska.host[deska.host.name == "x0"]["x0"].service, ["www"])

    # set to a two-value list through an absolute name
    r.c(startChangeset())
    r.cvoid(setAttribute("host", "x1", "service", ["www", "imap"]))
    r.assertEqual(verifyingObjectMultipleData(r, "host", "x1")["service"], AnyOrderList(["www", "imap"]))
    r.c(commitChangeset("set host x1 service www & imap"))

    r.assertEqual(verifyingObjectMultipleData(r, "host", "x1")["service"], AnyOrderList(["www", "imap"]))
    deska.init(_l.Connection(args))
    r.assertEqual(deska.host[deska.host.name == "x1"]["x1"].service, AnyOrderList(["www", "imap"]))

    # use incremental operations
    r.c(startChangeset())
    r.cvoid(setAttributeInsert("host", "x2", "service", "www"))
    r.assertEqual(verifyingObjectMultipleData(r, "host", "x2")["service"], AnyOrderList(["www"]))
    r.cvoid(setAttributeInsert("host", "x2", "service", "imap"))
    r.assertEqual(verifyingObjectMultipleData(r, "host", "x2")["service"], AnyOrderList(["www", "imap"]))
    r.cvoid(setAttributeRemove("host", "x2", "service", "imap"))
    r.assertEqual(verifyingObjectMultipleData(r, "host", "x2")["service"], AnyOrderList(["www"]))
    r.c(commitChangeset("set host x2 service www (incremental)"))

    r.assertEqual(verifyingObjectMultipleData(r, "host", "x2")["service"], AnyOrderList(["www"]))
    deska.init(_l.Connection(args))
    r.assertEqual(deska.host[deska.host.name == "x2"]["x2"].service, AnyOrderList(["www"]))

    # now add the imap role back in
    r.c(startChangeset())
    r.cvoid(setAttributeInsert("host", "x2", "service", "imap"))
    r.assertEqual(verifyingObjectMultipleData(r, "host", "x2")["service"], AnyOrderList(["www", "imap"]))
    r.c(commitChangeset("set host x2 service www and imap (incremental)"))

    r.assertEqual(verifyingObjectMultipleData(r, "host", "x2")["service"], AnyOrderList(["www", "imap"]))
    deska.init(_l.Connection(args))
    r.assertEqual(deska.host[deska.host.name == "x2"]["x2"].service, AnyOrderList(["www", "imap"]))

    # test filtering
    r.assertEqual(sorted(deska.host[deska.host.service.contains("www")].keys()),
                  ["x%d" % i for i in range(3)])
    # FAIL Redmine#318: negative filtering via identifier_set is not supported yet
    #r.assertEqual(sorted(deska.host[deska.host.service.notContains("www")].keys()),
    #              ["x%d" % i for i in range(3, 10)])

    # FIXME: more filters for set-specific stuff
    # FIXME: compound filters
