import sys
import os
import datetime
sys.path.append(os.getcwd())
import libLowLevelPyDeska

p = libLowLevelPyDeska.pythonify
v = libLowLevelPyDeska.valueify
d = libLowLevelPyDeska.deoptionalify

def verify(x):
    print "%s: %s" % (type(x), x)
    deska_val = v(x)
    try:
        non_opt = d(deska_val)
        print "Deska::Db::NonOptionalValue: %s" % str(non_opt)
    except RuntimeError, e:
        print "deoptionalify failed"
    py_x = p(deska_val)
    print "Deska::Db::Value -> Py: %s %s" % (type(py_x), py_x)
    if str(x) != str(py_x):
        print "*DIFFERENT* strings"
    if x != py_x:
        print "*DIFFERENT* objects"
    print

variants = ("ahoj", 3, 333.666, None,
            libLowLevelPyDeska.IPv4Address("127.0.0.1"),
            libLowLevelPyDeska.IPv6Address("::1"),
            libLowLevelPyDeska.MacAddress("00:16:3e:37:53:2B"),
            datetime.datetime.now(), datetime.date.today(),
           )

for x in variants:
    verify(x)

c = libLowLevelPyDeska.Connection()
kindNames = c.kindNames()
print "kindNames: %s" % str([x for x in kindNames])

for kind in kindNames:
    kindRelations = c.kindRelations(kind)
    print "kindRelations(%s): %s" % (kind, [str(x) for x in kindRelations])

for kind in kindNames:
    kindAttributes = c.kindAttributes(kind)
    print "kindAttributes(%s): %s" % (kind, [str(x) for x in kindAttributes])
