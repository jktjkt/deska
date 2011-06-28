import sys
import os
sys.path.append(os.getcwd())
import libLowLevelPyDeska

c = libLowLevelPyDeska.Connection()
kindNames = c.kindNames()
print "kindNames: %s" % str([x for x in kindNames])

for kind in kindNames:
    kindRelations = c.kindRelations(kind)
    print "kindRelations(%s): %s" % (kind, [str(x) for x in kindRelations])

for kind in kindNames:
    kindAttributes = c.kindAttributes(kind)
    print "kindAttributes(%s): %s" % (kind, [str(x) for x in kindAttributes])
