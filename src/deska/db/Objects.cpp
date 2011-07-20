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

#include <sstream>
#include <boost/date_time/posix_time/time_formatters.hpp>

#include "Objects.h"
#include "deska/db/AdditionalValueStreamOperators.h"

namespace Deska {
namespace Db {

std::ostream& operator<<(std::ostream &stream, const Type t)
{
    switch (t) {
    case TYPE_IDENTIFIER:
        return stream << "TYPE_IDENTIFIER";
    case TYPE_STRING:
        return stream << "TYPE_STRING";
    case TYPE_INT:
        return stream << "TYPE_INT";
    case TYPE_DOUBLE:
        return stream << "TYPE_DOUBLE";
    case TYPE_IPV4_ADDRESS:
        return stream << "TYPE_IPV4_ADDRESS";
    case TYPE_IPV6_ADDRESS:
        return stream << "TYPE_IPV6_ADDRESS";
    case TYPE_MAC_ADDRESS:
        return stream << "MAC_ADDRESS";
    case TYPE_DATE:
        return stream << "TYPE_DATE";
    case TYPE_TIMESTAMP:
        return stream << "TYPE_TIMESTAMP";
    }
    return stream << "[Invalid type:" << static_cast<int>(t) << "]";
}

bool operator==(const KindAttributeDataType &a, const KindAttributeDataType &b)
{
    return a.name == b.name && a.type == b.type;
}

bool operator!=(const KindAttributeDataType &a, const KindAttributeDataType &b)
{
    return !(a == b);
}

std::ostream& operator<<(std::ostream &stream, const KindAttributeDataType &k)
{
    return stream << k.name << ": " << k.type;
}

bool operator==(const ObjectRelation &a, const ObjectRelation &b)
{
    return a.kind == b.kind && a.target == b.target;
}

bool operator!=(const ObjectRelation &a, const ObjectRelation &b)
{
    return !(a == b);
}


bool operator<(const ObjectRelation &a, const ObjectRelation &b)
{
    if (a.kind == b.kind) {
        return a.target < b.target;
    } else {
        return a.kind < b.kind;
    }
}

std::ostream& operator<<(std::ostream &stream, const ObjectRelation& o)
{
    switch (o.kind) {
    case RELATION_MERGE_WITH:
        return stream << "mergeWith(" << o.target << ")";
    case RELATION_EMBED_INTO:
        return stream << "embedInto(" << o.target << ")";
    case RELATION_REFERS_TO:
        return stream << "refersTo(" << o.target << ")";
    case RELATION_TEMPLATIZED:
        return stream << "templatized(" << o.target << ")";
    case RELATION_INVALID:
        return stream << "RELATION_INVALID";
    }
    return stream << "[Invalid relation: " << static_cast<int>(o.kind) << "]";
}

ObjectRelation::ObjectRelation(const ObjectRelationKind _kind, const Identifier &_target):
    kind(_kind), target(_target)
{
}

ObjectRelation ObjectRelation::mergeWith(const Identifier &target)
{
    return ObjectRelation(RELATION_MERGE_WITH, target);
}

ObjectRelation ObjectRelation::embedInto(const Identifier &target)
{
    return ObjectRelation(RELATION_EMBED_INTO, target);
}

ObjectRelation ObjectRelation::refersTo(const Identifier &target)
{
    return ObjectRelation(RELATION_REFERS_TO, target);
}

ObjectRelation ObjectRelation::templatized(const Identifier &target)
{
    return ObjectRelation(RELATION_TEMPLATIZED, target);
}


ObjectDefinition::ObjectDefinition(const Identifier &kindName, const Identifier &objectName):
    kind(kindName), name(objectName)
{
}

std::ostream& operator<<(std::ostream &stream, const ObjectDefinition &o)
{
    return stream << o.kind << " " << o.name;
}

bool operator==(const ObjectDefinition &a, const ObjectDefinition &b)
{
    return a.kind == b.kind && a.name == b.name;
}

bool operator!=(const ObjectDefinition &a, const ObjectDefinition &b)
{
    return !(a == b);
}


AttributeDefinition::AttributeDefinition(const Identifier &attributeName, const Value &assignedValue):
    attribute(attributeName), value(assignedValue)
{
}

/** @short Variant visitor for printing the attribute into an ostream

This is required because the boost::posix_time::ptime lacks a proper operator<<.
*/
struct PrintValue: public boost::static_visitor<void> {
    std::ostream *str;
    PrintValue(std::ostream *stream): str(stream) {}

    /** @short Default implementation: just use proper operator<< */
    template<typename T>
    void operator()(const T &t) const {
        *str << t;
    }

    /** @short Got to provide a specialization for boost::posix_time::ptime */
    void operator()(const boost::posix_time::ptime &t) const {
        *str << boost::posix_time::to_simple_string(t);
    }
};

std::ostream& operator<<(std::ostream &stream, const AttributeDefinition &a)
{
    if (a.value) {
        stream << a.attribute << " ";
        boost::apply_visitor(PrintValue(&stream), *(a.value));
        return stream;
    } else {
        return stream << "no " << a.attribute;
    }
}

bool operator==(const AttributeDefinition &a, const AttributeDefinition &b)
{
    return a.attribute == b.attribute && a.value == b.value;
}

bool operator!=(const AttributeDefinition &a, const AttributeDefinition &b)
{
    return !(a == b);
}

/** @short Variant visitor that returns the type name of a Deska::Db::Value */
struct DeskaValueTypeName: public boost::static_visitor<std::string>
{
    result_type operator()(const std::string &v) const
    {
        return "string";
    }
    result_type operator()(const int &v) const
    {
        return "int";
    }
    result_type operator()(const double &v) const
    {
        return "double";
    }
    result_type operator()(const boost::asio::ip::address_v4 &v) const
    {
        return "IPv4Address";
    }
    result_type operator()(const boost::asio::ip::address_v6 &v) const
    {
        return "IPv6Address";
    }
    result_type operator()(const MacAddress &v) const
    {
        return "MacAddress";
    }
    result_type operator()(const boost::posix_time::ptime &v) const
    {
        return "timestamp";
    }
    result_type operator()(const boost::gregorian::date &v) const
    {
        return "date";
    }
};

/** @short __repr__ for Deska::Db::NonOptionalValue */
std::string repr_NonOptionalValue(const NonOptionalValue &v)
{
    std::ostringstream ss;
    ss << "Value<" << boost::apply_visitor(DeskaValueTypeName(), v) << ">(" << v << ")";
    return ss.str();
}

/** @short __repr__ for Deska::Db::Value */
std::string repr_Value(const Value &v)
{
    return v ? repr_NonOptionalValue(*v) : std::string("Value(None)");
}

/** @short __str__ for Deska::Db::Value */
std::string str_Value(const Value &v)
{
    if (v) {
        std::ostringstream ss;
        ss << *v;
        return ss.str();
    }
    return "None";
}


}
}
