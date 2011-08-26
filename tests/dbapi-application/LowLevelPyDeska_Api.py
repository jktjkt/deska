'''Test libLowLevelPyDeska's support for low-level DBAPI operations'''

from apiUtils import *

def imperative(r):

    import sys
    sys.path = ["../.."] + sys.path
    import libLowLevelPyDeska as _l

    objectNames = ["a", "b", "c"]
    r.c(startChangeset())
    for obj in objectNames:
        r.assertEquals(r.c(createObject("host", obj)), obj)
        r.cvoid(setAttribute("host", obj, "note_host", "This is host %s" % obj))
    revision = r.c(commitChangeset("Added three objects"))

    r.assertTrue(revision.startswith("r"))
    revisionNum = int(revision[1:])

    # let's actually test the bindings now
    c = _l.Connection()
    # start with kindNames
    kindNames = c.kindNames()
    r.assertEquals(sorted(kindNames), sorted(["hardware", "host", "host_template", "vendor",
                                              "interface", "interface_template", "service",
                                              "hardware_template"]))

    # continue with kindRelations
    expectedRelations = {
        "interface": "[embedInto(host), templatized(interface_template)]",
        "hardware": "[mergeWith(host), refersTo(vendor), templatized(hardware_template)]",
        "host": "[mergeWith(hardware), refersTo(service), templatized(host_template)]",
        "vendor": "[]",
        "service": "[]",
        "hardware_template": "[templatized(hardware_template)]",
        # the embedInto is *not* present in this case, as templates cannot define this attribute
        "interface_template": "[templatized(interface_template)]",
    }
    for kind in kindNames:
        kindRelations = c.kindRelations(kind)
        # FIXME: Redmine #272
        if not kind.endswith("_template"):
            r.assertEquals(repr(sorted(kindRelations)), expectedRelations[kind])

    # check kindAttributes
    expectedAttrs = {
        "hardware": "[cpu_num: TYPE_INT, host: TYPE_IDENTIFIER, note_hardware: TYPE_STRING, purchase: TYPE_DATE, ram: TYPE_INT, template_hardware: TYPE_IDENTIFIER, vendor: TYPE_IDENTIFIER, warranty: TYPE_DATE]",
        "hardware_template": "[cpu_num: TYPE_INT, note_hardware: TYPE_STRING, purchase: TYPE_DATE, ram: TYPE_INT, template_hardware: TYPE_IDENTIFIER, vendor: TYPE_IDENTIFIER, warranty: TYPE_DATE]",
        "host": "[hardware: TYPE_IDENTIFIER, note_host: TYPE_STRING, service: TYPE_IDENTIFIER_SET, template_host: TYPE_IDENTIFIER]",
        "host_template": "[note_host: TYPE_STRING, service: TYPE_IDENTIFIER_SET, template_host: TYPE_IDENTIFIER]",
        "interface": "[host: TYPE_IDENTIFIER, ip4: TYPE_IPV4_ADDRESS, ip6: TYPE_IPV6_ADDRESS, mac: TYPE_MAC_ADDRESS, note: TYPE_STRING, template_interface: TYPE_IDENTIFIER]",
        "interface_template": "[ip4: TYPE_IPV4_ADDRESS, ip6: TYPE_IPV6_ADDRESS, mac: TYPE_MAC_ADDRESS, note: TYPE_STRING, template_interface: TYPE_IDENTIFIER]",
        "vendor": "[]",
        "service": "[isvm: TYPE_INT, note: TYPE_STRING]"
    }
    for kind in kindNames:
        kindAttributes = c.kindAttributes(kind)
        r.assertEquals(repr(sorted(kindAttributes)), expectedAttrs[kind])

    print revision

    # check kindInstances and their behavior with various combinations of filters and RevisionIds
    kindInstances = []
    kindInstances.append(c.kindInstances("host"))
    kindInstances.append(c.kindInstances("host",
                                         _l.OptionalFilter()))
    kindInstances.append(c.kindInstances("host",
                                         _l.OptionalFilter(),
                                         _l.OptionalRevisionId()))
    kindInstances.append(c.kindInstances("host",
                                         _l.OptionalFilter(),
                                         _l.OptionalRevisionId(
                                             _l.RevisionId(revisionNum)
                                         )))
    # we have to build a filter now
    fe1 = _l.MetadataExpression(
        _l.ComparisonOperator.COLUMN_LT, "revision",
        _l.Py_2_DeskaMetadataValue(_l.RevisionId(333)))
    f1 = _l.std_vector_Filter()
    of = _l.OrFilter(f1)

    # try an empty OrFilter
    kindInstances.append(c.kindInstances("host",
                                         _l.OptionalFilter(of)))

    # an empty OrFilter
    kindInstances.append(c.kindInstances("host",
                                         _l.OptionalFilter(_l.OrFilter(
                                             _l.std_vector_Filter()))))
    # an empty AndFilter
    kindInstances.append(c.kindInstances("host",
                                         _l.OptionalFilter(_l.AndFilter(
                                             _l.std_vector_Filter()))))

    for res in kindInstances:
        r.assertEquals(sorted(res), sorted(["a", "b", "c"]))
