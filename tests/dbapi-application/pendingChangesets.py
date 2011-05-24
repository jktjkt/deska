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
]
