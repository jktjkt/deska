import sys
import os
import datetime
sys.path.append(os.getcwd())
import libLowLevelPyDeska as _l

fe1 = _l.MetadataExpression(
    _l.ComparisonOperator.COLUMN_LT, "revision",
    _l.Py_2_DeskaMetadataValue(_l.RevisionId(333)))
fe2 = _l.AttributeExpression(
    _l.ComparisonOperator.COLUMN_EQ, "hardware", "vendor",
    _l.Py_2_DeskaDbValue("hp"))
print fe1
print _l.Expression(fe1)
print fe2
print _l.Expression(fe2)

tmp = _l.std_vector_Filter()
tmp.append(_l.Filter(_l.Expression(fe1)))
tmp.append(_l.Filter(_l.Expression(fe2)))
of = _l.OrFilter(tmp)
del tmp
print of
print

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
#resolvedData = c.resolvedObjectData("host", "hpv2")
#print resolvedData
# not implemented, either
#multipleData = c.multipleObjectData("host", f2)
#print multipleData
# not implemented, either
#multipleResolved = c.multipleResolvedObjectData("host", f2)
#print multipleResolved
