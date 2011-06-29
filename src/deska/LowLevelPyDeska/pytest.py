import sys
import os
import datetime
sys.path.append(os.getcwd())
import libLowLevelPyDeska

def verify(x):
    print "%s: %s" % (type(x), x)
    deska_val = libLowLevelPyDeska.Py_2_DeskaDbValue(x)
    try:
        non_opt = libLowLevelPyDeska.DeskaDbValue_2_DeskaDbNonOptionalValue(deska_val)
        print "Deska::Db::NonOptionalValue: str  %s" % str(non_opt)
        print "Deska::Db::NonOptionalValue: repr %s" % repr(non_opt)
    except RuntimeError, e:
        print "deoptionalify failed"
    py_x = libLowLevelPyDeska.DeskaDbValue_2_Py(deska_val)
    print "Deska::Db::Value -> Py str : %s" % str(py_x)
    print "Deska::Db::Value -> Py repr: %s" % repr(py_x)
    if str(x) != str(py_x):
        print "*DIFFERENT* strings: %s, %s" % (str(x), str(py_x))
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

for x in variants:
    print "%s: %s" % (type(x), x)
    metadataval = libLowLevelPyDeska.Py_2_DeskaMetadataValue(x)

    print "Deska::Db::MetadataValue: str  %s" % str(metadataval)
    print "Deska::Db::MetadataValue: repr %s" % repr(metadataval)
    py_x = libLowLevelPyDeska.DeskaMetadataValue_2_Py(metadataval)
    print "Deska::Db::MetadataValue -> Py: str  %s" % str(py_x)
    print "Deska::Db::MetadataValue -> Py: repr %s" % repr(py_x)
    if str(x) != str(py_x):
        print "*DIFFERENT* strings: %s, %s" % (str(x), str(py_x))
    if x != py_x:
        print "*DIFFERENT* objects: %s %s, %s %s" % (
            type(x), x, type(py_x), py_x)
    print

fe1 = libLowLevelPyDeska.MetadataExpression(
    libLowLevelPyDeska.ComparisonOperator.COLUMN_LT, "revision",
    libLowLevelPyDeska.Py_2_DeskaMetadataValue(libLowLevelPyDeska.RevisionId(333)))
fe2 = libLowLevelPyDeska.AttributeExpression(
    libLowLevelPyDeska.ComparisonOperator.COLUMN_EQ, "hardware", "vendor",
    libLowLevelPyDeska.Py_2_DeskaDbValue("hp"))
print fe1
print libLowLevelPyDeska.Expression(fe1)
print fe2
print libLowLevelPyDeska.Expression(fe2)

tmp = libLowLevelPyDeska.std_vector_Filter()
tmp.append(libLowLevelPyDeska.Filter(libLowLevelPyDeska.Expression(fe1)))
tmp.append(libLowLevelPyDeska.Filter(libLowLevelPyDeska.Expression(fe2)))
of = libLowLevelPyDeska.OrFilter(tmp)
del tmp
print of
print

c = libLowLevelPyDeska.Connection()
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
                                         libLowLevelPyDeska.OptionalFilter()))
    kindInstances.append(c.kindInstances(kind,
                                         libLowLevelPyDeska.OptionalFilter(),
                                         libLowLevelPyDeska.OptionalRevisionId()))
    kindInstances.append(c.kindInstances(kind,
                                         libLowLevelPyDeska.OptionalFilter(),
                                         libLowLevelPyDeska.OptionalRevisionId(
                                             libLowLevelPyDeska.RevisionId(1)
                                         )))
    kindInstances.append(c.kindInstances(kind,
                                         libLowLevelPyDeska.OptionalFilter(of)))


    for answer in kindInstances:
        print "kindInstances(%s): %s" % (kind, answer)

objData = c.objectData("host", "hpv2")
print objData
for x in objData:
    print "%s %r" % (type(x.key()), x.key())
    print "%s %r" % (type(x.data()), x.data())

f2 = libLowLevelPyDeska.Filter(libLowLevelPyDeska.Expression(
    libLowLevelPyDeska.AttributeExpression(
        libLowLevelPyDeska.ComparisonOperator.COLUMN_NE,
        "host", "name", libLowLevelPyDeska.Py_2_DeskaDbValue("non-existant"))
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
