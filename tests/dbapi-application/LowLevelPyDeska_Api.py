'''Test libLowLevelPyDeska's support for low-level DBAPI operations'''

from apiUtils import *

def imperative(r):

    import sys
    sys.path = ["../.."] + sys.path
    import libLowLevelPyDeska as _l

    objectNames = ["a", "b", "c"]
    r.c(startChangeset())
    for obj in objectNames:
        r.c(createObject("host", obj))
        r.c(setAttribute("host", obj, "note", "This is host %s" % obj))
    revision = r.c(commitChangeset("Added three objects"))

    r.assertTrue(revision.startswith("r"))
    revisionNum = int(revision[1:])

    # let's actually test the bindings now
    c = _l.Connection()
    # start with kindNames
    kindNames = c.kindNames()
    r.assertEquals(set(kindNames), set(["hardware", "host", "vendor",
                                        "interface"]))

    # continue with kindRelations
    expectedRelations = {
        "interface": "[embedInto(host), ]",
        "hardware": "[]", "host": "[]", "vendor": "[]"
    }
    for kind in kindNames:
        kindRelations = c.kindRelations(kind)
        r.assertEquals(repr(kindRelations), expectedRelations[kind])

    # check kindAttributes
    expectedAttrs = {
        "hardware": "[warranty: TYPE_STRING, purchase: TYPE_STRING, vendor: TYPE_IDENTIFIER, cpu_num: TYPE_INT, ram: TYPE_INT, note: TYPE_STRING, ]",
        "host": "[hardware: TYPE_IDENTIFIER, note: TYPE_STRING, ]",
        "interface": "[note: TYPE_STRING, host: TYPE_IDENTIFIER, ip6: TYPE_STRING, mac: TYPE_STRING, ip4: TYPE_STRING, ]",
        "vendor": "[]"
    }
    for kind in kindNames:
        kindAttributes = c.kindAttributes(kind)
        r.assertEquals(repr(kindAttributes), expectedAttrs[kind])
