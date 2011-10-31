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

#include "deska/LowLevelPyDeska/Value.h"
#include "deska/db/AdditionalValueStreamOperators.h"

using namespace boost::python;
using namespace Deska::Db;

/** @short Convert a Deska::Db::Value into Deska::Db::NonOptionalValue or throw an exception */
NonOptionalValue DeskaDbValue_2_DeskaDbNonOptionalValue(const Value &v)
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
api::object DeskaDbValue_2_Py(const Value &v)
{
    return v ? boost::apply_visitor(DeskaValueToPythonObject(), *v): api::object();
}

/** @short Convert a python object into the Deska::Db::Value */
Value Py_2_DeskaDbValue(const api::object &o)
{
    // None
    if (o == api::object())
        return Value();

    // string
    extract<std::string> get_str(o);
    if (get_str.check())
        return NonOptionalValue(get_str());

    // set<string>
    extract<std::set<std::string> > get_str_set(o);
    if (get_str_set.check())
        return NonOptionalValue(get_str_set());

    // bool
    // This check has to be performed using low-level Python functions to prevent boost::python from treting ints as bools and vice versa.
    // An alternative is a project-wide define, which is not exactly an etalon of elegance.
    // See http://boost.2283326.n4.nabble.com/Avoiding-implicit-boost-python-extract-lt-gt-conversions-td3433520.html for details
    if (PyBool_Check(o.ptr())) {
        extract<bool> get_bool(o);
        BOOST_ASSERT(get_bool.check());
        return NonOptionalValue(get_bool());
    }

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

    // boost::posix_time::ptime
    extract<boost::posix_time::ptime> get_ptime(o);
    if (get_ptime.check())
        return NonOptionalValue(get_ptime());

    // boost::gregorian::date
    extract<boost::gregorian::date> get_date(o);
    if (get_date.check())
        return NonOptionalValue(get_date());

    throw std::runtime_error("Unsupported type of a python object");
}

/** @short Construct an IPv4Address from string */
boost::asio::ip::address_v4 *ipv4AddressFromString(const std::string &s)
{
    return new boost::asio::ip::address_v4(boost::asio::ip::address_v4::from_string(s));
}

/** @short Construct an IPv6Address from string */
boost::asio::ip::address_v6 *ipv6AddressFromString(const std::string &s)
{
    return new boost::asio::ip::address_v6(boost::asio::ip::address_v6::from_string(s));
}

void exportDeskaValue()
{
    // At first, wrap the boost::optional. This is not meant to be used directly.
    class_<Value>("Value")
            .def(not self)
            .def(self == other<Value>())
            .def("__repr__", repr_Value)
            .def("__str__", str_Value) // cannot use the self_ns::str because of boost::optional whose operator<< sucks
            ;
    // Then wrap the underlying variant
    class_<NonOptionalValue>("NonOptionalValue")
            .def(self == other<NonOptionalValue>())
            .def("__repr__", Deska::Db::repr_NonOptionalValue)
            .def(self_ns::str(self))
            ;

    // Functions that convert between the Python and Deska representations of various values
    def("DeskaDbValue_2_DeskaDbNonOptionalValue", DeskaDbValue_2_DeskaDbNonOptionalValue);
    def("Py_2_DeskaDbValue", Py_2_DeskaDbValue);
    def("DeskaDbValue_2_Py", DeskaDbValue_2_Py);

    // Custom classes for the Deska::Db::Value
    class_<boost::asio::ip::address_v4>("IPv4Address")
            .def("__init__", make_constructor(ipv4AddressFromString))
            .def(self == other<boost::asio::ip::address_v4>())
            .def(self != other<boost::asio::ip::address_v4>())
            .def(self_ns::str(self));
    class_<boost::asio::ip::address_v6>("IPv6Address")
            .def("__init__", make_constructor(ipv6AddressFromString))
            .def(self == other<boost::asio::ip::address_v6>())
            .def(self != other<boost::asio::ip::address_v6>())
            .def(self_ns::str(self));
    class_<MacAddress>("MacAddress", init<const std::string&>())
            .def(self == other<MacAddress>())
            .def(self != other<MacAddress>())
            .def(self_ns::str(self));
}
