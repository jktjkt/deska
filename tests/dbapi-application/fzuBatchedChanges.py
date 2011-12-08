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
    r.cvoid(applyBatchedChanges([
        {"command": "renameObject", "kindName": "host", "oldObjectName": oldname, "newObjectName": newname},
        {"command": "renameObject", "kindName": "hardware", "oldObjectName": oldname, "newObjectName": newname},
        {"command": "renameObject", "kindName": "box", "oldObjectName": oldname, "newObjectName": newname},
        #{"command": "renameObject", "kindName": "host", "oldObjectName": oldname, "newObjectName": newname}
    ]))
    r.assertEqual(r.c(commitChangeset(".")), "r3")
