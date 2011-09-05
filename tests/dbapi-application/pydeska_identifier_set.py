'''Test that Python's support for identifier sets works properly'''

from apiUtils import *
import datetime
import deska

def imperative(r):
    services = ["www", "ftp", "ui", "dpm", "imap"]
    r.c(startChangeset())
    for x in range(10):
        objname = "x%d" % x
        r.assertEqual(r.c(createObject("host", objname)), objname)
    for service in services:
        r.assertEqual(r.c(createObject("service", service)), service)
    r.c(commitChangeset("objects set up"))

    r.c(startChangeset())
    r.cvoid(setAttribute("host", "x0", "service", ["www"]))
    r.assertEqual(verifyingObjectMultipleData(r, "host", "x0")["service"], ["www"])
    r.c(commitChangeset("set host x0 service www"))

    r.assertEqual(verifyingObjectMultipleData(r, "host", "x0")["service"], ["www"])
    deska.init()
    r.assertEqual(deska.host[deska.host.name == "x0"]["x0"].service, ["www"])
