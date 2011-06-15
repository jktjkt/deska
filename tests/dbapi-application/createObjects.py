'''Test createObject() behavior with pending changesets and revisions'''

from apiUtils import *

declarative = [
    # start with an empty state
    pendingChangesets().returns([]),
    # create new changeset
    startChangeset().register("tmp1"),
    # verify that it is indeed present
    pendingChangesets().returns([
        {'status': 'INPROGRESS', 'changeset': Variable("tmp1"),
         'author': DeskaDbUser(),
         'timestamp': CurrentTimestamp(), 'parentRevision': 'r1',
         'message': ''}
    ]),
    # abort this changeset
    abortCurrentChangeset(),
    # try to create an object -- this should fail
    createObject("vendor", "a").throws(NoChangesetError()),

    # open another changeset
    startChangeset().register("tmp2"),

    createObject("vendor", "a"),
    createObject("vendor", "aa"),
    createObject("vendor", "blabla"),

    kindInstances("vendor").returns(AnyOrderList(("a", "aa", "blabla"))),

    # close it again; this should make current state empty
    abortCurrentChangeset(),
    kindInstances("vendor").returns([]),

    # do it one more time
    startChangeset().register("tmp3"),

    createObject("vendor", "a"),
    createObject("vendor", "aa"),
    createObject("vendor", "blabla"),

    kindInstances("vendor").returns(AnyOrderList(("a", "aa", "blabla"))),

    # save the result
    commitChangeset("Saving three new vendors").register("r2"),

    # the data should be still there
    kindInstances("vendor").returns(AnyOrderList(("a", "aa", "blabla"))),

    # ...but not in the previous version...
    kindInstances("vendor", revision="r1").returns([]),

    # ...but indeed present when we ask specifically for the current one
    kindInstances("vendor", revision=Variable("r2")).returns(
        AnyOrderList(("a", "aa", "blabla"))),
]
