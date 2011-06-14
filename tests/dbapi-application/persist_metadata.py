from apiUtils import *

def imperative(r):
    kinds = r.c(kindNames())
    doStuff(r, kinds)

    # now repeat the test from inside a changeset
    r.c(startChangeset())
    doStuff(r, kinds)
    r.c(abortCurrentChangeset())

def doStuff(r, kinds):
    res_kindNames = r.c(kindNames())
    r.assertEqual(type(res_kindNames), list)
    r.assertEqual(set(res_kindNames), set(kinds))

    for kind in res_kindNames:
        r.assertEqual(type(r.c(kindInstances(kind))), list)
        r.assertEqual(type(r.c(kindRelations(kind))), list)
        r.assertEqual(type(r.c(kindAttributes(kind))), dict)

    r.cfail(kindInstances("error_kind_name"))
    r.cfail(kindRelations("error_kind_name"))
    r.cfail(kindAttributes("error_kind_name"))
