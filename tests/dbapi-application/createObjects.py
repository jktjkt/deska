'''Test createObject() behavior with pending changesets and revisions'''

from apiUtils import *

def imperative(r):
    # start with an empty state
    r.assertEqual(r.c(pendingChangesets()), [])
    # create new changeset
    tmp1 = r.c(startChangeset())
    r.assertEqual(tmp1, "tmp1")
    # verify that it is indeed present
    r.assertEqual(r.c(pendingChangesets()),
    [
        {'status': 'INPROGRESS', 'changeset': tmp1,
         'author': DeskaDbUser(),
         'timestamp': CurrentTimestamp(), 'parentRevision': 'r1',
         'message': ''}
    ])
    # abort this changeset
    r.cvoid(abortCurrentChangeset())
    # try to create an object -- this should fail
    r.cfail(createObject("vendor", "a"), NoChangesetError())
    # open another changeset
    tmp2 = r.c(startChangeset())
    r.assertEqual(tmp2, "tmp2")

    r.assertEqual(r.c(createObject("vendor", "a")), "a")
    r.assertEqual(r.c(createObject("vendor", "aa")), "aa")
    r.assertEqual(r.c(createObject("vendor", "blabla")), "blabla")
    # FIXME: Fails, Redmine#398
    #r.cfail(createObject("vendor", "a"), exception=AlreadyExistsError())

    r.assertEqual(r.c(kindInstances("vendor")),
                  AnyOrderList(("a", "aa", "blabla")))

    # close it again; this should make current state empty
    r.cvoid(abortCurrentChangeset())
    r.assertEqual(r.c(kindInstances("vendor")), [])

    # do it one more time
    tmp3 = r.c(startChangeset())
    r.assertEqual(tmp3, "tmp3")

    r.assertEqual(r.c(createObject("vendor", "a")), "a")
    r.assertEqual(r.c(createObject("vendor", "aa")), "aa")
    r.assertEqual(r.c(createObject("vendor", "blabla")), "blabla")

    r.assertEqual(r.c(kindInstances("vendor")),
                  AnyOrderList(("a", "aa", "blabla")))

    # save the result
    r2 = r.c(commitChangeset("Saving three new vendors"))
    r.assertEqual(r2, "r2")

    # the data should be still there
    r.assertEqual(r.c(kindInstances("vendor")),
                  AnyOrderList(("a", "aa", "blabla")))

    # ...but not in the previous version...
    r.assertEqual(r.c(kindInstances("vendor", revision="r1")), [])

    # ...but indeed present when we ask specifically for the current one
    r.assertEqual(r.c(kindInstances("vendor", revision=r2)),
                  AnyOrderList(("a", "aa", "blabla")))

    tmp4 = r.c(startChangeset())
    r.c(createObject("vendor", "a-b"))
    r.cfail(createObject("vendor", "a-"), exception=ConstraintError())
    r.cfail(createObject("vendor", "-a"), exception=ConstraintError())
    r.assertEqual(r.c(commitChangeset(".")), "r3")
