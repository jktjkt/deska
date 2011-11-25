'''Test creating, resuming and throwing away of pending changesets'''

from apiUtils import *

def imperative(r):
    # start with an empty state
    r.assertEqual(r.c(pendingChangesets()), [])
    # create new changeset
    r.assertEqual(r.c(startChangeset()), "tmp1")
    # verify that it is indeed present
    r.assertEqual(r.c(pendingChangesets()), [
        {'status': 'INPROGRESS', 'changeset': 'tmp1',
         'author': DeskaDbUser(),
         'timestamp': CurrentTimestamp(), 'parentRevision': 'r1',
         'message': ''}
    ])
    # abort this changeset
    r.cvoid(abortCurrentChangeset())
    # there should be no pending changesets at this time
    r.assertEqual(r.c(pendingChangesets()), [])
    # abort once again, this will fail
    r.cfail(abortCurrentChangeset(), NoChangesetError())
    # detach once again, this will fail
    r.cfail(detachFromCurrentChangeset("xyz"), NoChangesetError())
    # try to commit a non-existent changeset
    r.cfail(commitChangeset("xyz"), NoChangesetError())
    # create new changeset once again
    r.assertEqual(r.c(startChangeset()), "tmp2")
    # and detach from it
    r.cvoid(detachFromCurrentChangeset("xyz"))
    # verify that it's listed in a correct state
    r.assertEqual(r.c(pendingChangesets()), [
        {'status': 'DETACHED', 'changeset': 'tmp2',
         'author': DeskaDbUser(),
         'timestamp': CurrentTimestamp(), 'parentRevision': 'r1',
         'message': 'xyz'}
    ])
    # attach to tmp2 again
    r.cvoid(resumeChangeset("tmp2"))
    # it should be pending again
    r.assertEqual(r.c(pendingChangesets()), [
        {'status': 'INPROGRESS', 'changeset': 'tmp2',
         'author': DeskaDbUser(),
         'timestamp': CurrentTimestamp(), 'parentRevision': 'r1',
         'message': 'xyz'}
    ])
    # create third changeset; this should fail, as we're already in one
    r.cfail(startChangeset(), ChangesetAlreadyOpenError())
    # try attaching once again; this should again fail
    r.cfail(resumeChangeset("tmp2"), ChangesetAlreadyOpenError())
    # clean the state
    r.cvoid(detachFromCurrentChangeset("foo"))
    # try attaching a persistent revision; this should fail
    r.cfail(resumeChangeset("r0"), ChangesetParsingError())
    # try the same with a non-existing permanent changeset
    r.cfail(resumeChangeset("r123"), ChangesetParsingError())
