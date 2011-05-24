j = [
    ( # start with an empty state
    {"command": "pendingChangesets"},
    {'response': 'pendingChangesets', "pendingChangesets": []}
    ),
    ( # create new changeset
    {"command": "startChangeset"},
    {'response': 'startChangeset', "startChangeset": "tmp1"}
    ),
    # FIXME: this depends on current date
    #( # verify that it is indeed present
    #{"command": "pendingChangesets"},
    #{'response': 'pendingChangesets', "pendingChangesets":
    # # FIXME: add data here
    # []}
    #),
]
