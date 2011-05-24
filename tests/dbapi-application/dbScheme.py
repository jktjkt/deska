j = [(
    {"command": "kindNames"},
    {'response': 'kindNames',
     'kindNames': ['interface', 'vendor', 'host', 'hardware']}
    ),
    ({"command": "kindAttributes", "kindName": "interface"},
     {"kindName": "interface", "kindAttributes":
        {"note": "string", "ip": "string", "host": "int", "mac": "string"},
      "response": "kindAttributes"}),
    ({"kindName": "hardware", "command": "kindRelations"},
     {"kindName": "hardware", "kindRelations": [],
      "response": "kindRelations"})
]