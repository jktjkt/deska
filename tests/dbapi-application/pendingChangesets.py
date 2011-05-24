import apiUtils

j = [
    ( # start with an empty state
    {"command": "pendingChangesets"},
    {'response': 'pendingChangesets', "pendingChangesets": []}
    ),
    ( # create new changeset
    {"command": "startChangeset"},
    {'response': 'startChangeset', "startChangeset": "tmp1"}
    ),
    ( # verify that it is indeed present
    {"command": "pendingChangesets"},
    {'response': 'pendingChangesets', "pendingChangesets": [
        {'status': 'INPROGRESS', 'changeset': 'tmp1',
         'author': apiUtils.DeskaDbUser(),
         'timestamp': apiUtils.CurrentTimestamp(), 'parentRevision': 'r1',
         'message': ''}
    ]}
    ),
    ( # abort this changeset
    {"command": "abortCurrentChangeset"},
    {'response': 'abortCurrentChangeset'}
    ),
    ( # there should be no pending changesets at this time
    {"command": "pendingChangesets"},
    {'response': 'pendingChangesets', "pendingChangesets": []}
    ),
    # FIXME: try to abort once again; that should fail with some DBAPI error
    # FIXME: also try to detach, now that there's no changeset
    ( # create new changeset once again
    {"command": "startChangeset"},
    {'response': 'startChangeset', "startChangeset": "tmp2"}
    ),
    ( # and detach from it
    {"command":"detachFromCurrentChangeset","message":"xyz"},
    {'response': 'detachFromCurrentChangeset'}
    ),
    ( # verify that it's listed in a correct state
    {"command": "pendingChangesets"},
    {'response': 'pendingChangesets', "pendingChangesets": [
        {'status': 'DETACHED', 'changeset': 'tmp2',
         'author': apiUtils.DeskaDbUser(),
         'timestamp': apiUtils.CurrentTimestamp(), 'parentRevision': 'r1',
         'message': 'xyz'}
    ]}
    ),
    ( # attach to tmp2 again
    {"command":"resumeChangeset","revision":"tmp2"},
    {'response': 'resumeChangeset'}
    ),
    ( # it should be pending again
    {"command": "pendingChangesets"},
    {'response': 'pendingChangesets', "pendingChangesets": [
        {'status': 'INPROGRESS', 'changeset': 'tmp2',
         'author': apiUtils.DeskaDbUser(),
         'timestamp': apiUtils.CurrentTimestamp(), 'parentRevision': 'r1',
         'message': ''}
    ]}
    ),
    # FIXME: simulate what happens when starting a changeset while being
    # attached to one
]
