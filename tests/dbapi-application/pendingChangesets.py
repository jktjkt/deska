from apiUtils import *

j = [
    ( # start with an empty state
    {"command": "pendingChangesets"},
    {'response': 'pendingChangesets', "pendingChangesets": []}
    ),
    # create new changeset
    DBAPI().startChangeset().returns("tmp1"),
    ( # verify that it is indeed present
    {"command": "pendingChangesets"},
    {'response': 'pendingChangesets', "pendingChangesets": [
        {'status': 'INPROGRESS', 'changeset': 'tmp1',
         'author': DeskaDbUser(),
         'timestamp': CurrentTimestamp(), 'parentRevision': 'r1',
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
    ( # abort once again, this will fail
    {"command": "abortCurrentChangeset"},
    {'response': 'abortCurrentChangeset', 'dbException': NoChangesetError()
    }
    ),
    ( # detach once again, this will fail
    {"command":"detachFromCurrentChangeset","message":"xyz"},
    {'response': 'detachFromCurrentChangeset', "message": "xyz",
     #"dbException": NoChangesetError()
    }
    ),
    ( # create new changeset once again
    {"command": "startChangeset"},
    {'response': 'startChangeset', "startChangeset": "tmp2"}
    ),
    ( # and detach from it
    {"command":"detachFromCurrentChangeset","message":"xyz"},
    {'response': 'detachFromCurrentChangeset',"message":"xyz"}
    ),
    ( # verify that it's listed in a correct state
    {"command": "pendingChangesets"},
    {'response': 'pendingChangesets', "pendingChangesets": [
        {'status': 'DETACHED', 'changeset': 'tmp2',
         'author': DeskaDbUser(),
         'timestamp': CurrentTimestamp(), 'parentRevision': 'r1',
         'message': 'xyz'}
    ]}
    ),
    ( # attach to tmp2 again
    {"command":"resumeChangeset","revision":"tmp2"},
    {'response': 'resumeChangeset',"revision":"tmp2"}
    ),
    ( # it should be pending again
    {"command": "pendingChangesets"},
    {'response': 'pendingChangesets', "pendingChangesets": [
        {'status': 'INPROGRESS', 'changeset': 'tmp2',
         'author': DeskaDbUser(),
         'timestamp': CurrentTimestamp(), 'parentRevision': 'r1',
         'message': 'xyz'}
    ]}
    ),
    ( # create third changeset; this should fail, as we're already in one
    {"command": "startChangeset"},
    {'response': 'startChangeset', "dbException": ChangesetAlreadyOpenError()
    }
    ),
    ( # try attaching once again; this should again fail
    {"command":"resumeChangeset","revision":"tmp2"},
    {'response': 'resumeChangeset',"revision":"tmp2",
     #"dbException": ChangesetAlreadyOpenError()
     }
    ),

]
