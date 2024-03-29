from apiUtils import *
from testdbapi import Connection

def imperative(r):
    # Let's play around with concurrent access to the DB
    conn1 = Connection(r.cmd)
    conn2 = Connection(r.cmd)

    # At this point, there are no pending changesets
    r.assertEqual(r.c(pendingChangesets(), conn1), [])
    r.assertEqual(r.c(pendingChangesets(), conn2), [])

    # The newly created one shall be immediately visible from both connections
    changeset = r.c(startChangeset(), conn1)
    r.assertEqual(len(r.c(pendingChangesets(), conn1)), 1)
    r.assertEqual(len(r.c(pendingChangesets(), conn2)), 1)

    # Calling freezeView() doesn't affect the view of the pending changesets
    r.cvoid(freezeView(), conn2)
    r.cvoid(abortCurrentChangeset(), conn1)
    r.assertEqual(r.c(pendingChangesets(), conn1), [])
    r.assertEqual(r.c(pendingChangesets(), conn2), [])

    # Objects created in a changeset shall not be visible in other sessions
    changeset = r.c(startChangeset(), conn1)
    r.c(createObject("vendor", "v1"), conn1)
    r.assertEqual(r.c(kindInstances("vendor"), conn1), ["v1"])
    r.assertEqual(r.c(kindInstances("vendor"), conn2), [])
    conn3 = Connection(r.cmd)
    # ...not even in a non-frozen one.
    r.assertEqual(r.c(kindInstances("vendor"), conn3), [])

    # Commit the change. It shall be invisible in the conn2.
    r.assertEqual(r.c(commitChangeset("."), conn1), "r2")
    r.assertEqual(r.c(kindInstances("vendor"), conn1), ["v1"])
    r.assertEqual(r.c(kindInstances("vendor"), conn2), [])
    r.assertEqual(r.c(kindInstances("vendor"), conn3), ["v1"])

    # Check that a frozen view and an active changeset are mutually exclusive
    r.cfail(startChangeset(), conn=conn2, exception=FreezingError())
    changeset = r.c(startChangeset(), conn1)
    r.cfail(freezeView(), conn=conn1, exception=FreezingError())
    r.cfail(unFreezeView(), conn=conn1, exception=FreezingError())
    r.cfail(resumeChangeset(changeset), conn=conn2, exception=FreezingError())

    # A locked changeset cannot be stolen by another session
    r.cvoid(lockCurrentChangeset(), conn1)
    r.cfail(resumeChangeset(changeset), conn=conn3, exception=ChangesetLockingError())
    # Try to lock it once more from the original session.  This should work.
    r.cvoid(lockCurrentChangeset(), conn1)
    r.cfail(resumeChangeset(changeset), conn=conn3, exception=ChangesetLockingError())
    # Now get the lock count back to one. It shall remain locked.
    r.cvoid(unlockCurrentChangeset(), conn1)
    r.cfail(resumeChangeset(changeset), conn=conn3, exception=ChangesetLockingError())
    # Unlock the changeset and steal it
    r.cvoid(unlockCurrentChangeset(), conn1)
    r.cvoid(resumeChangeset(changeset), conn=conn3)
    # Now the first session shall not be able to commit the changeset
    r.cfail(commitChangeset("."), conn=conn1, exception=NoChangesetError())
    # On the other hand, the third session shall be able to go forward.
    # The third connection was not holding its lock.
    r.c(commitChangeset("."), conn=conn3)
    # Let's try to create a changeset, steal it, lock it from the other session
    # and try to commit
    changeset = r.c(startChangeset(), conn=conn1)
    r.cvoid(resumeChangeset(changeset), conn=conn3)
    r.cvoid(lockCurrentChangeset(), conn=conn3)
    r.cfail(resumeChangeset(changeset), conn=conn1, exception=ChangesetLockingError())
    r.c(commitChangeset("."), conn=conn3)
    r.cfail(resumeChangeset(changeset), conn=conn1, exception=ChangesetRangeError())
