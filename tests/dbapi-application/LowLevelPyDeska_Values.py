'''Test libLowLevelPyDeska's support for Python binding for Deska Values'''

from apiUtils import *
import datetime

def imperative(r):

    import libLowLevelPyDeska as _l

    variants = ("ahoj", 3, 333.666, None, _l.IPv4Address("127.0.0.1"),
                _l.IPv6Address("::1"), _l.MacAddress("00:16:3e:37:53:2B"),
                datetime.datetime.now(), datetime.date.today(), True, False)

    for x in variants:
        print "%s: %s" % (type(x), x)

        # verify Deska::Db::Value
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
        r.assertEqual(str(x), str(py_x))
        r.assertEqual(x, py_x)

        # verify Deska::Db::MetadataValue
        metadataval = _l.Py_2_DeskaMetadataValue(x)
        print "Deska::Db::MetadataValue: str  %s" % str(metadataval)
        print "Deska::Db::MetadataValue: repr %s" % repr(metadataval)
        py_x = _l.DeskaMetadataValue_2_Py(metadataval)
        print "Deska::Db::MetadataValue -> Py: str  %s" % str(py_x)
        print "Deska::Db::MetadataValue -> Py: repr %s" % repr(py_x)
        r.assertEqual(str(x), str(py_x))
        r.assertEqual(x, py_x)

    # test filter expressions
    fe1 = _l.MetadataExpression(
        _l.ComparisonOperator.COLUMN_LT, "revision",
        _l.Py_2_DeskaMetadataValue(_l.RevisionId(333)))
    fe2 = _l.AttributeExpression(
        _l.ComparisonOperator.COLUMN_EQ, "hardware", "vendor",
        _l.Py_2_DeskaDbValue("hp"))
    r.assertEqual(repr(fe1), "MetadataExpression(revision < MetadataValue<RevisionId>(r333))")
    r.assertEqual(repr(fe2), "AttributeExpression(hardware.vendor == Value<string>(hp))")
    r.assertEqual(repr(_l.Expression(fe1)),
        "Expression(MetadataExpression(revision < MetadataValue<RevisionId>(r333)))")
    r.assertEqual(repr(_l.Expression(fe2)),
        "Expression(AttributeExpression(hardware.vendor == Value<string>(hp)))")

    f1 = _l.std_vector_Filter()
    f1.append(_l.Filter(_l.Expression(fe1)))
    f1.append(_l.Filter(_l.Expression(fe2)))
    of = _l.OrFilter(f1)
    del f1
    r.assertEqual(repr(of),
        "OrFilter([" +
        "MetadataExpression(revision < MetadataValue<RevisionId>(r333)), " +
        "AttributeExpression(hardware.vendor == Value<string>(hp)), " +
        "])")

    fe3 = _l.AttributeExpression(_l.ComparisonOperator.COLUMN_CONTAINS, "hardware", "vendor",
        _l.Py_2_DeskaDbValue("hp"))
    r.assertEqual(repr(_l.Expression(fe3)),
        "Expression(AttributeExpression(hardware.vendor contains Value<string>(hp)))")

    fe4 = _l.AttributeExpression(_l.ComparisonOperator.COLUMN_NOT_CONTAINS, "hardware", "vendor",
        _l.Py_2_DeskaDbValue(_l.std_set_std_string(("hp", "compaq"))))
    r.assertEqual(repr(_l.Expression(fe4)),
        "Expression(AttributeExpression(hardware.vendor not contains Value<identifier_set>([compaq, hp])))")

    fe5 = _l.SpecialExpression(_l.SpecialFilterType.SPECIAL_EMBEDDED_LAST_ONE,
                               "interface")
    r.assertEqual(repr(_l.Expression(fe5)),
                  "Expression(SpecialExpression(interface embeddedLastOne))")
