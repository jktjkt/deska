from apiUtils import *

j = [
    # start with an empty state
    pendingChangesets().returns([]),
    # create new changeset
    startChangeset().returns("tmp1"),
    # verify that it is indeed present
    pendingChangesets().returns([
        {'status': 'INPROGRESS', 'changeset': 'tmp1',
         'author': DeskaDbUser(),
         'timestamp': CurrentTimestamp(), 'parentRevision': 'r1',
         'message': ''}
    ]),
    # abort this changeset
    abortCurrentChangeset(),
    # there should be no pending changesets at this time
    pendingChangesets().returns([]),
    # abort once again, this will fail
    abortCurrentChangeset().throws(NoChangesetError()),
    # detach once again, this will fail
    detachFromCurrentChangeset("xyz"),#.throws(NoChangesetError()),
    # create new changeset once again
    startChangeset().returns("tmp2"),
    # and detach from it
    detachFromCurrentChangeset("xyz"),
    # verify that it's listed in a correct state
    pendingChangesets().returns([
        {'status': 'DETACHED', 'changeset': 'tmp2',
         'author': DeskaDbUser(),
         'timestamp': CurrentTimestamp(), 'parentRevision': 'r1',
         'message': 'xyz'}
    ]),
    # attach to tmp2 again
    resumeChangeset("tmp2"),
    # it should be pending again
    pendingChangesets().returns([
        {'status': 'INPROGRESS', 'changeset': 'tmp2',
         'author': DeskaDbUser(),
         'timestamp': CurrentTimestamp(), 'parentRevision': 'r1',
         'message': 'xyz'}
    ]),
    # create third changeset; this should fail, as we're already in one
    startChangeset().throws(ChangesetAlreadyOpenError()),
    # try attaching once again; this should again fail
    resumeChangeset("tmp2"),#.throws(ChangesetAlreadyOpenError()),

]
