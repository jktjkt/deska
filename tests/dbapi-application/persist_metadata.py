from apiUtils import *

def imperative(r):
    res_kindNames = r.c(kindNames())
    r.assertEqual(type(res_kindNames), list)

    for kind in res_kindNames:
        r.assertEqual(type(r.c(kindInstances(kind))), list)
        r.assertEqual(type(r.c(kindRelations(kind))), list)
        r.assertEqual(type(r.c(kindAttributes(kind))), dict)

    # FIXME: fail in kindInstances("error_kind_name"); dtto for kindRelations
    # and kindAttributes

    # test kinsInstances from inside a changeset
    r.c(startChangeset())
    for kind in res_kindNames:
        r.assertEqual(type(r.c(kindInstances(kind))), list)
        r.assertEqual(type(r.c(kindRelations(kind))), list)
        r.assertEqual(type(r.c(kindAttributes(kind))), dict)
    r.c(abortCurrentChangeset())
