'''Tests for retrieval of the database scheme'''

from apiUtils import *

helper_interface_attrs = {
    "host": "identifier", "note": "string", "ip4": "ipv4address", "ip6": "ipv6address",
    "mac": "macaddress", "template_interface": "identifier"
}
helper_hardware_attrs = {
    "warranty": "date", "purchase": "date", "vendor": "identifier", "cpu_ht": "bool",
    "template_hardware": "identifier", "cpu_num": "int", "ram": "int", "note_hardware": "string", "host" : "identifier"
}
helper_host_attrs = {
    "hardware": "identifier", "note_host": "string",
    "service": "identifier_set", "template_host": "identifier"
}

def helper_split_templated_args(source, ignored):
    return dict((k, v) for k, v in source.iteritems() if k not in ignored)

def imperative(r):
    r.assertEqual(r.c(kindNames()), AnyOrderList(
        ('interface', 'interface_template', 'vendor', 'hardware_template', 'host', 'hardware', 'service', 'host_template')))

    r.assertEqual(r.c(kindAttributes("interface")), helper_interface_attrs)
    r.assertEqual(r.c(kindAttributes("interface_template")), helper_split_templated_args(helper_interface_attrs, ("host",)))
    r.assertEqual(r.c(kindAttributes("vendor")), {})
    r.assertEqual(r.c(kindAttributes("service")), {"note": "string"})
    r.assertEqual(r.c(kindAttributes("host")), helper_host_attrs)
    r.assertEqual(r.c(kindAttributes("host_template")), helper_split_templated_args(helper_host_attrs, ("hardware",)))
    r.assertEqual(r.c(kindAttributes("hardware")), helper_hardware_attrs)
    r.assertEqual(r.c(kindAttributes("hardware_template")), helper_split_templated_args(helper_hardware_attrs, ("hardware","host")))

    # try to ask for a non-existing object
    r.cfail(kindAttributes("pwnpwn"), InvalidKindError())

    r.assertEqual(r.c(kindRelations("interface")),
        AnyOrderList([
            {'relation': 'EMBED_INTO', 'target': 'host', 'column': 'host'},
            {'relation': 'TEMPLATIZED', 'target': 'interface_template', 'column': 'template_interface'}])
    )
    r.assertEqual(r.c(kindRelations("interface_template")), AnyOrderList([
        {'relation': 'TEMPLATIZED', 'target': 'interface_template', 'column': 'template_interface'}]))
    r.assertEqual(r.c(kindRelations("host")),
        AnyOrderList([
            {'relation': 'MERGE_WITH', 'target': 'hardware', 'column': 'hardware'},
            {'relation': 'TEMPLATIZED', 'target': 'host_template', 'column': 'template_host'},
            {'relation': 'REFERS_TO', 'target': 'service', 'column': 'service'}
        ])
    )
    r.assertEqual(r.c(kindRelations("hardware")),
        AnyOrderList([
            {'relation': 'REFERS_TO', 'target': 'vendor', 'column': 'vendor'},
            {'relation': 'MERGE_WITH', 'target': 'host', 'column': 'host'},
            {'relation': 'TEMPLATIZED', 'target': 'hardware_template', 'column': 'template_hardware'}])
    )
    r.assertEqual(r.c(kindRelations("hardware_template")),
       AnyOrderList([
           {'relation': 'REFERS_TO', 'target': 'vendor', 'column': 'vendor'},
           {'relation': 'TEMPLATIZED', 'target': 'hardware_template', 'column': 'template_hardware'},
       ])
    )
    r.assertEqual(r.c(kindRelations("vendor")), [])
    r.assertEqual(r.c(kindRelations("service")), [])
