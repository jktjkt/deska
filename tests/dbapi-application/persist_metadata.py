from apiUtils import *

def imperative(r):
    kinds = sorted(r.c(kindNames()))
    doStuff(r, kinds)

    # now repeat the test from inside a changeset
    r.c(startChangeset())
    doStuff(r, kinds)
    r.c(abortCurrentChangeset())

def doStuff(r, kinds):
    res_kindNames = r.c(kindNames())
    r.assertEqual(type(res_kindNames), list)
    r.assertEqual(sorted(res_kindNames), kinds)

    for kind in res_kindNames:
        r.assertEqual(type(r.c(kindInstances(kind))), list)
        r.assertEqual(type(r.c(kindRelations(kind))), list)
        r.assertEqual(type(r.c(kindAttributes(kind))), dict)

    r.cfail(kindInstances("error_kind_name"), InvalidKindError())
    r.cfail(kindRelations("error_kind_name"), InvalidKindError())
    r.cfail(kindAttributes("error_kind_name"), InvalidKindError())
