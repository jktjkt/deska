'''Test the containable objects'''

from apiUtils import *
import sys

def imperative(r):
    r.assertEqual(r.c(startChangeset()), "tmp1")
    r.assertEqual(r.c(createObject("hardware", "foo")), "foo")
    r.assertEqual(r.c(createObject("switch", "foo")), "foo")
    r.cfail(createObject("box", "foo"), exception=ConstraintError())
    #r.assertEqual(r.c(commitChangeset(".")), "r1")
