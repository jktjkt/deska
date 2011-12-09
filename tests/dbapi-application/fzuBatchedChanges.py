'''Test the containable objects'''

from apiUtils import *
import sys

def imperative(r):
    # Init the objects
    r.assertEqual(r.c(startChangeset()), "tmp1")
    r.c(createObject("modelhardware", "fake"))
    r.assertEqual(r.c(createObject("hardware", "foo")), "foo")
    r.cvoid(setAttribute("hardware", "foo", "modelhardware", "fake"))
    r.assertEqual(r.c(createObject("host", "foo")), "foo")
    r.assertEqual(r.c(createObject("box", "foo")), "foo")
    r.assertEqual(r.c(commitChangeset(".")), "r2")

    # Try a mass-renaming
    r.assertEqual(r.c(startChangeset()), "tmp2")
    oldname = "foo"
    newname = "bar"
    modifications = [{"command": "renameObject", "kindName": kind, "oldObjectName": oldname, "newObjectName": newname}
            for kind in ["host", "hardware", "box"]]
    # Duplicate operations shall fail
    r.cfail(applyBatchedChanges(modifications + [modifications[0]]), exception=NotFoundError())
    # And the whole command shall also be atomic
    r.cvoid(applyBatchedChanges(modifications))
    r.assertEqual(r.c(commitChangeset(".")), "r3")
