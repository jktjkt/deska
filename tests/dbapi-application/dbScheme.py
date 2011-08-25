'''Tests for retrieval of the database scheme'''

from apiUtils import *

helper_interface_attrs = {
    "note": "string", "ip4": "ipv4address", "ip6": "ipv6address", "host": "identifier",
    "mac": "macaddress", "template": "identifier"
}
helper_hardware_attrs = {
    "warranty": "date", "purchase": "date", "vendor": "identifier",
    "template": "identifier", "cpu_num": "int", "ram": "int", "note_hardware": "string", "host" : "identifier"
}
helper_hardware_template_attrs = dict((k,v) for (k, v) in helper_hardware_attrs.iteritems() if k != "host")

def imperative(r):
    r.assertEqual(r.c(kindNames()), AnyOrderList(('interface', 'interface_template', 'vendor', 'hardware_template', 'host', 'hardware')))

    r.assertEqual(r.c(kindAttributes("interface")), helper_interface_attrs)
    r.assertEqual(r.c(kindAttributes("interface_template")),
        dict((k, v) for k, v in helper_interface_attrs.iteritems() if k != "host"))
    r.assertEqual(r.c(kindAttributes("vendor")), {})
    r.assertEqual(r.c(kindAttributes("host")),
        {
            "hardware": "identifier",
            "note_host": "string"
        })
    r.assertEqual(r.c(kindAttributes("hardware")), helper_hardware_attrs)
    r.assertEqual(r.c(kindAttributes("hardware_template")), helper_hardware_template_attrs)

    # try to ask for a non-existing object
    r.cfail(kindAttributes("pwnpwn"), InvalidKindError())

    r.assertEqual(r.c(kindRelations("interface")),
        AnyOrderList([
            {'relation': 'EMBED_INTO', 'target': 'host'},
            {'relation': 'TEMPLATIZED', 'target': 'interface_template'}])
    )
    r.assertEqual(r.c(kindRelations("interface_template")),
       AnyOrderList([{'relation': 'TEMPLATIZED', 'target':
                      'interface_template'}])),
    r.assertEqual(r.c(kindRelations("host")),
        AnyOrderList([{'relation': 'MERGE_WITH', 'target': 'hardware'}])
    )
    r.assertEqual(r.c(kindRelations("hardware")),
        AnyOrderList([
            {'relation': 'REFERS_TO', 'target': 'vendor'},
            {'relation': 'MERGE_WITH', 'target': 'host'},
            {'relation': 'TEMPLATIZED', 'target': 'hardware_template'}])
    )
    r.assertEqual(r.c(kindRelations("hardware_template")),
       AnyOrderList([
           {'relation': 'REFERS_TO', 'target': 'vendor'},
           {'relation': 'TEMPLATIZED', 'target': 'hardware_template'},
       ])
    )
    r.assertEqual(r.c(kindRelations("vendor")), [])
