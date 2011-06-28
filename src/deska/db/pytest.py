import sys
import os
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
    print

for x in ("ahoj", 1, 333.666, None,
          libLowLevelPyDeska.IPv4Address("127.0.0.1"),
          libLowLevelPyDeska.IPv6Address("::1"),
         ):
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
