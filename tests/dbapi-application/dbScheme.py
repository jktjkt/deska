'''Tests for retrieval of the database scheme'''

from apiUtils import *

helper_interface_attrs = {
    "note": "string", "ip4": "string", "ip6": "string", "host": "identifier",
    "mac": "string", "template": "identifier"
}
helper_hardware_attrs = {
    "warranty": "string", "purchase": "string", "vendor": "identifier",
    "template": "identifier", "cpu_num": "int", "ram": "int", "note": "string"
}

declarative = [
    kindNames().returns(
        AnyOrderList(('interface', 'interface_template', 'vendor', 'hardware_template', 'host', 'hardware'))),

    kindAttributes("interface").returns(helper_interface_attrs),
    kindAttributes("interface_template").returns(helper_interface_attrs),
    kindAttributes("vendor").returns({}),
    kindAttributes("host").returns(
        {
            "hardware": "identifier",
            "note": "string"}
    ),
    kindAttributes("hardware").returns(helper_hardware_attrs),
    kindAttributes("hardware_template").returns(helper_hardware_attrs),

    # try to ask for a non-existing object
    kindAttributes("pwnpwn").throws(InvalidKindError()),

    kindRelations("interface").returns(
        [{'relation': 'EMBED_INTO', 'target': 'host'}]
    ),
    kindRelations("host").returns(
        [{'relation': 'REFERS_TO', 'target': 'hardware'}]
    ),
    kindRelations("hardware").returns(
        [{'relation': 'REFERS_TO', 'target': 'vendor'}]
    ),
    kindRelations("vendor").returns([]),
]
