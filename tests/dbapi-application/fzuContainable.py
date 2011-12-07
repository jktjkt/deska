'''Test the containable objects'''

from apiUtils import *
import sys

def imperative(r):
    r.assertEqual(r.c(startChangeset()), "tmp1")
    r.c(createObject("modelhardware", "fake"))
    r.assertEqual(r.c(createObject("hardware", "foo")), "foo")
    r.cvoid(setAttribute("hardware", "foo", "modelhardware", "fake"))
    # FIXME: Redmine #401, even a deleted object blocks creating proper objects
    ##r.assertEqual(r.c(createObject("switch", "foo")), "foo")
    # The DB will not know where to associate this object
    ##r.cfail(createObject("box", "foo"), exception=ConstraintError())
    r.assertEqual(r.c(createObject("host", "foo")), "foo")
    exc = r.cfail(commitChangeset("."), exception=ConstraintError())
    r.assertTrue(exc["message"].find("null value in column \"box\" violates not-null constraint") != -1)

    ##r.cvoid(deleteObject("switch", "foo"))
    r.assertEqual(r.c(createObject("box", "foo")), "foo")
    r.assertEqual(r.c(commitChangeset(".")), "r2")
