'''Tests for retrieval of the database scheme'''

from apiUtils import *

helper_interface_attrs = {
    "note": "string", "ip4": "ipv4address", "ip6": "ipv6address", "host": "identifier",
    "mac": "macaddress", "template": "identifier"
}
helper_hardware_attrs = {
    "warranty": "date", "purchase": "date", "vendor": "identifier",
    "template": "identifier", "cpu_num": "int", "ram": "int", "note": "string"
}

declarative = [
    kindNames().returns(
        AnyOrderList(('interface', 'interface_template', 'vendor', 'hardware_template', 'host', 'hardware'))),

    kindAttributes("interface").returns(helper_interface_attrs),
    kindAttributes("interface_template").returns(
        dict((k, v) for k, v in helper_interface_attrs.iteritems() if k != "host")),
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
        AnyOrderList([
            {'relation': 'EMBED_INTO', 'target': 'host'},
            {'relation': 'TEMPLATIZED', 'target': 'interface_template'}])
    ),
    kindRelations("interface_template").returns(
       AnyOrderList([{'relation': 'TEMPLATIZED', 'target':
                      'interface_template'}])),
    kindRelations("host").returns(
        AnyOrderList([{'relation': 'REFERS_TO', 'target': 'hardware'}])
    ),
    kindRelations("hardware").returns(
        AnyOrderList([
            {'relation': 'REFERS_TO', 'target': 'vendor'},
            {'relation': 'TEMPLATIZED', 'target': 'hardware_template'}])
    ),
    kindRelations("hardware_template").returns(
       AnyOrderList([
           {'relation': 'REFERS_TO', 'target': 'vendor'},
           {'relation': 'TEMPLATIZED', 'target': 'hardware_template'},
       ])
    ),
    kindRelations("vendor").returns([]),
]
