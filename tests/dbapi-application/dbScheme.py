'''Tests for retrieval of the database scheme'''

from apiUtils import *

declarative = [
    kindNames().returns(
        AnyOrderList(('interface', 'vendor', 'host', 'hardware'))),

    kindAttributes("interface").returns(
        {"note": "string", "ip4": "string", "ip6": "string", "host": "identifier", "mac": "string"}),
    kindAttributes("vendor").returns({}),
    kindAttributes("host").returns(
        {
            "hardware": "identifier",
            "note": "string"}
    ),
    kindAttributes("hardware").returns(
        {"warranty": "string", "purchase": "string",
         "vendor": "identifier", 
         "cpu_num": "int", "ram": "int", "note": "string"}
    ),

    # try to ask for a non-existing object
    kindAttributes("pwnpwn").throws(InvalidKindError()), 

    kindRelations("interface").returns(
        [{'relation': 'EMBED_INTO', 'target': 'host'}]
    ),
    kindRelations("vendor").returns([]),
    kindRelations("host").returns([]),
    kindRelations("hardware").returns([]),
]
