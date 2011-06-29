import sys
import os
import datetime
sys.path.append(os.getcwd())
import libLowLevelPyDeska as _l

def verify(x):
    print "%s: %s" % (type(x), x)
    deska_val = _l.Py_2_DeskaDbValue(x)
    try:
        non_opt = _l.DeskaDbValue_2_DeskaDbNonOptionalValue(deska_val)
        print "Deska::Db::NonOptionalValue: str  %s" % str(non_opt)
        print "Deska::Db::NonOptionalValue: repr %s" % repr(non_opt)
    except RuntimeError, e:
        print "deoptionalify failed"
    py_x = _l.DeskaDbValue_2_Py(deska_val)
    print "Deska::Db::Value -> Py str : %s" % str(py_x)
    print "Deska::Db::Value -> Py repr: %s" % repr(py_x)
    if str(x) != str(py_x):
        print "*DIFFERENT* strings: %s, %s" % (str(x), str(py_x))
    if x != py_x:
        print "*DIFFERENT* objects"
    print

variants = ("ahoj", 3, 333.666, None,
            _l.IPv4Address("127.0.0.1"),
            _l.IPv6Address("::1"),
            _l.MacAddress("00:16:3e:37:53:2B"),
            datetime.datetime.now(), datetime.date.today(),
           )

for x in variants:
    verify(x)

for x in variants:
    print "%s: %s" % (type(x), x)
    metadataval = _l.Py_2_DeskaMetadataValue(x)

    print "Deska::Db::MetadataValue: str  %s" % str(metadataval)
    print "Deska::Db::MetadataValue: repr %s" % repr(metadataval)
    py_x = _l.DeskaMetadataValue_2_Py(metadataval)
    print "Deska::Db::MetadataValue -> Py: str  %s" % str(py_x)
    print "Deska::Db::MetadataValue -> Py: repr %s" % repr(py_x)
    if str(x) != str(py_x):
        print "*DIFFERENT* strings: %s, %s" % (str(x), str(py_x))
    if x != py_x:
        print "*DIFFERENT* objects: %s %s, %s %s" % (
            type(x), x, type(py_x), py_x)
    print

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
