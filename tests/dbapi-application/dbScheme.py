from apiUtils import *

declarative = [
    kindNames().returns(
        AnyOrderList(('interface', 'vendor', 'host', 'hardware'))),

    kindAttributes("interface").returns(
        {"note": "string", "ip": "string", "host": "string", "mac": "string"}),
    kindAttributes("vendor").returns({}),
    kindAttributes("host").returns(
        {
            # "hardware": "identifier", issue #232
            "hardware": "string",
            "note": "string"}
    ),
    kindAttributes("hardware").returns(
        {"warranty": "string", "purchase": "string",
         "vendor": "identifier", # fails, issue #232
         "cpu_num": "int", "ram": "int", "note": "string"}
    ),

    # try to ask for a non-existing object
    kindAttributes("pwnpwn").throws(InvalidKindError()), # fails for now

    kindRelations("interface").returns(
        [{'relation': 'EMBED_INTO', 'target': 'host'}]
    ),
    kindRelations("vendor").returns([]),
    kindRelations("host").returns([]),
    kindRelations("hardware").returns([]),
]
