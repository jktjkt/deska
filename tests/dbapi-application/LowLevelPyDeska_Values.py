'''Test libLowLevelPyDeska's support for Python binding for Deska Values'''

from apiUtils import *
import datetime

def imperative(r):

    import sys
    sys.path = ["../.."] + sys.path
    import libLowLevelPyDeska as _l

    variants = ("ahoj", 3, 333.666, None, _l.IPv4Address("127.0.0.1"),
                _l.IPv6Address("::1"), _l.MacAddress("00:16:3e:37:53:2B"),
                datetime.datetime.now(), datetime.date.today())

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

