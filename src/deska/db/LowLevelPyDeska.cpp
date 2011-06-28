/* Copyright (C) 2011 Jan Kundrát <kundratj@fzu.cz>
*
* This file is part of the Deska, a tool for central administration of a grid site
* http://projects.flaska.net/projects/show/deska
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or the version 3 of the License.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; see the file COPYING.  If not, write to
* the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
* Boston, MA 02110-1301, USA.
* */

#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include "deska/db/Connection.h"
#include "deska/db/AdditionalValueStreamOperators.h"

using namespace boost::python;
using namespace Deska::Db;

// FIXME: remove later?
template<typename T>
T Value_extract(const Value &v)
{
    if (v) {
        try {
            return boost::get<T>(*v);
        } catch (const boost::bad_get &e) {
            throw std::runtime_error(std::string("Deska::Db::Value is of a different type: ") + e.what());
        }
    } else {
        throw std::runtime_error("Deska::Db::Value is null");
    }
}

/** @short Convert a Deska::Db::Value into Deska::Db::NonOptionalValue or throw an exception */
NonOptionalValue deoptionalify(const Value &v)
{
    if (v) {
        return *v;
    } else {
        throw std::runtime_error("Deska::Db::Value is null");
    }
}

/** @short Variant visitor for converting a Deska::Db::Value to a Python object */
struct DeskaValueToPythonObject: public boost::static_visitor<api::object>
{
    template <typename T>
    result_type operator()(const T &v) const
    {
        return result_type(v);
    }
};


/** @short Convert a Deska::Db::Value to a python object */
api::object pythonify(const Value &v)
{
    return v ? boost::apply_visitor(DeskaValueToPythonObject(), *v): api::object();
}

/** @short Convert a python object into the Deska::Db::Value */
Value valueify(const api::object &o)
{
    // None
    if (o == api::object())
        return Value();

    // string
    extract<std::string> get_str(o);
    if (get_str.check())
        return NonOptionalValue(get_str());

    // int
    extract<int> get_int(o);
    if (get_int.check())
        return NonOptionalValue(get_int());

    // double
    extract<double> get_double(o);
    if (get_double.check())
        return NonOptionalValue(get_double());

    // IPv4 address
    extract<boost::asio::ip::address_v4> get_ipv4(o);
    if (get_ipv4.check())
        return NonOptionalValue(get_ipv4());

    // IPv6 address
    extract<boost::asio::ip::address_v6> get_ipv6(o);
    if (get_ipv6.check())
        return NonOptionalValue(get_ipv6());

    // MAC address
    extract<MacAddress> get_mac(o);
    if (get_mac.check())
        return NonOptionalValue(get_mac());

    throw std::runtime_error("Unsupported type of a python object");
    //return Value();
}

boost::asio::ip::address_v4 *ipv4AddressFromString(const std::string &s)
{
    return new boost::asio::ip::address_v4(boost::asio::ip::address_v4::from_string(s));
}

boost::asio::ip::address_v6 *ipv6AddressFromString(const std::string &s)
{
    return new boost::asio::ip::address_v6(boost::asio::ip::address_v6::from_string(s));
}

BOOST_PYTHON_MODULE(libLowLevelPyDeska)
{
    // required for kindNames
    typedef std::vector<std::string> vect_string;
    class_<vect_string>("std_vector_string")
            .def(vector_indexing_suite<vect_string>());

    // required for kindRelations
    typedef std::vector<ObjectRelation> vect_ObjectRelation;
    class_<vect_ObjectRelation>("std_vector_Deska_Db_ObjectRelation")
            .def(vector_indexing_suite<vect_ObjectRelation>());

    enum_<ObjectRelationKind>("ObjectRelationKind")
            .value("MERGE_WITH", RELATION_MERGE_WITH)
            .value("EMBED_INTO", RELATION_EMBED_INTO)
            .value("REFERS_TO", RELATION_REFERS_TO)
            .value("IS_TEMPLATE", RELATION_IS_TEMPLATE)
            .value("TEMPLATIZED", RELATION_TEMPLATIZED);

    class_<ObjectRelation>("ObjectRelation", no_init)
            .def_readonly("kind", &ObjectRelation::kind)
            .def_readonly("target", &ObjectRelation::target)
            .def(self_ns::str(self));

    // required for kindAttributesboost::python::
    typedef std::vector<KindAttributeDataType> vect_KindAttributeDataType;
    class_<vect_KindAttributeDataType>("std_vector_Deska_Db_KindAttributeDataType")
            .def(vector_indexing_suite<vect_KindAttributeDataType>());
    enum_<Type>("AttributeType")
            .value("IDENTIFIER", TYPE_IDENTIFIER)
            .value("STRING", TYPE_STRING)
            .value("INT", TYPE_INT)
            .value("DOUBLE", TYPE_DOUBLE)
            .value("IPV4_ADDRESS", TYPE_IPV4_ADDRESS)
            .value("IPV6_ADDRESS", TYPE_IPV6_ADDRESS)
            .value("MAC_ADDRESS", TYPE_MAC_ADDRESS)
            .value("DATE", TYPE_DATE)
            .value("TIMESTAMP", TYPE_TIMESTAMP);

    class_<KindAttributeDataType>("KindAttributeDataType", no_init)
            .def_readonly("name", &KindAttributeDataType::name)
            .def_readonly("type", &KindAttributeDataType::type)
            .def(self_ns::str(self));

    // revisions and changesets (required for filters)
    class_<RevisionId>("RevisionId", init<const unsigned int>())
            .def(self == other<RevisionId>())
            .def(self_ns::str(self));
    class_<TemporaryChangesetId>("TemporaryChangesetId", init<const unsigned int>())
            .def(self == other<TemporaryChangesetId>())
            .def(self_ns::str(self));

    // The attribute value

    // At first, wrap the boost::optional. This is not meant to be used directly.
    class_<Value>("Value")
            .def(not self)
            .def(self == other<Value>())
            .def(self_ns::str(self));
    // Then wrap the underlying variant
    class_<NonOptionalValue>("NonOptionalValue")
            .def(self == other<NonOptionalValue>())
            .def(self_ns::str(self));

    // Functions that convert between the Python and Deska representations of various values
    def("deoptionalify", deoptionalify);
    def("pythonify", pythonify);
    def("valueify", valueify);

    // Custom classes for the Deska::Db::Value
    class_<boost::asio::ip::address_v4>("IPv4Address")
            .def("__init__", make_constructor(ipv4AddressFromString))
            .def(self == other<boost::asio::ip::address_v4>())
            .def(self_ns::str(self));
    class_<boost::asio::ip::address_v6>("IPv6Address")
            .def("__init__", make_constructor(ipv6AddressFromString))
            .def(self == other<boost::asio::ip::address_v6>())
            .def(self_ns::str(self));
    class_<MacAddress>("MacAddress", init<const std::string&>())
            .def(self == other<MacAddress>())
            .def(self_ns::str(self));

    // filters
    enum_<ComparisonOperator>("ComparisonOperator")
            .value("COLUMN_EQ", FILTER_COLUMN_EQ)
            .value("COLUMN_NE", FILTER_COLUMN_NE)
            .value("COLUMN_GT", FILTER_COLUMN_GT)
            .value("COLUMN_GE", FILTER_COLUMN_GE)
            .value("COLUMN_LT", FILTER_COLUMN_LT)
            .value("COLUMN_LE", FILTER_COLUMN_LE);
    //class_<MetadataValue>

    // DBAPI connection implementation
    class_<Connection, boost::noncopyable>("Connection")
            .def("kindNames", &Connection::kindNames)
            .def("kindRelations", &Connection::kindRelations)
            .def("kindAttributes", &Connection::kindAttributes);
}
