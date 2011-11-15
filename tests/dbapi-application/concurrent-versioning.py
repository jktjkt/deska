from apiUtils import *
from testdbapi import Connection

def imperative(r):
    # Let's play around with concurrent access to the DB
    conn2 = Connection(r.cmd)

    # At this point, there are no pending changesets
    r.assertEqual(r.c(pendingChangesets()), [])
    r.assertEqual(r.c(pendingChangesets(), conn2), [])

    # The newly created one shall be immediately visible from both connections
    changeset = r.c(startChangeset())
    r.assertEqual(len(r.c(pendingChangesets())), 1)
    r.assertEqual(len(r.c(pendingChangesets(), conn2)), 1)

    # Calling freezeView() doesn't affect the view of the pending changesets
    r.cvoid(freezeView(), conn2)
    r.cvoid(abortCurrentChangeset())
    r.assertEqual(r.c(pendingChangesets()), [])
    r.assertEqual(r.c(pendingChangesets(), conn2), [])

    # Objects created in a changeset shall not be visible in other sessions
    changeset = r.c(startChangeset())
    r.c(createObject("vendor", "v1"))
    r.assertEqual(r.c(kindInstances("vendor")), ["v1"])
    r.assertEqual(r.c(kindInstances("vendor"), conn2), [])
    conn3 = Connection(r.cmd)
    # ...not even in an non-frozen one.
    r.assertEqual(r.c(kindInstances("vendor"), conn3), [])

    # Commit the change. It shall be invisible in the conn2.
    r.assertEqual(r.c(commitChangeset(".")), "r2")
    r.assertEqual(r.c(kindInstances("vendor")), ["v1"])
    r.assertEqual(r.c(kindInstances("vendor"), conn2), [])
    r.assertEqual(r.c(kindInstances("vendor"), conn3), ["v1"])

    # FIXME: Redmine #305, frozen view cannot start/resume changesets
    #r.cfail(startChangeset(), conn2)
