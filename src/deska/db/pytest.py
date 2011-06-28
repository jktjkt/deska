import libLowLevelPyDeska

c = libLowLevelPyDeska.Connection()
kindNames = c.kindNames()
print "kindNames: %s" % str([x for x in kindNames])

for kind in kindNames:
    kindRelations = c.kindRelations(kind)
    print "kindRelations(%s): %s" % (kind, [str(x) for x in kindRelations])
