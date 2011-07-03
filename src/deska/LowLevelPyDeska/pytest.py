import sys
import os
import datetime
sys.path.append(os.getcwd())
import libLowLevelPyDeska as _l

c = _l.Connection()
kindNames = c.kindNames()
print "kindNames: %s" % kindNames

for kind in kindNames:
    kindRelations = c.kindRelations(kind)
    print "kindRelations(%s): %s" % (kind, kindRelations)

for kind in kindNames:
    kindAttributes = c.kindAttributes(kind)
    print "kindAttributes(%s): %s" % (kind, kindAttributes)

for kind in kindNames:
    kindInstances = []
    kindInstances.append(c.kindInstances(kind))
    kindInstances.append(c.kindInstances(kind,
                                         _l.OptionalFilter()))
    kindInstances.append(c.kindInstances(kind,
                                         _l.OptionalFilter(),
                                         _l.OptionalRevisionId()))
    kindInstances.append(c.kindInstances(kind,
                                         _l.OptionalFilter(),
                                         _l.OptionalRevisionId(
                                             _l.RevisionId(1)
                                         )))
    kindInstances.append(c.kindInstances(kind,
                                         _l.OptionalFilter(of)))


    for answer in kindInstances:
        print "kindInstances(%s): %s" % (kind, answer)

objData = c.objectData("host", "hpv2")
print objData
for x in objData:
    print "%s %r" % (type(x.key()), x.key())
    print "%s %r" % (type(x.data()), x.data())

f2 = _l.Filter(_l.Expression(
    _l.AttributeExpression(
        _l.ComparisonOperator.COLUMN_NE,
        "host", "name", _l.Py_2_DeskaDbValue("non-existant"))
))
print f2
# this would fail, it is not implemented yet
#resolvedData = c.resolvedObjectDataWithOrigin("host", "hpv2")
#print resolvedData
# not implemented, either
#multipleData = c.multipleObjectData("host", f2)
#print multipleData
# not implemented, either
#multipleResolved = c.multipleResolvedObjectDataWithOrigin("host", f2)
#print multipleResolved
