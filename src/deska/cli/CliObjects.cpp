/*
* Copyright (C) 2011 Tomáš Hubík <hubik.tomas@gmail.com>
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
#include "CliObjects.h"


namespace Deska {
namespace Cli {


ObjectDefinition::ObjectDefinition(const Db::Identifier &kindName, const Db::Identifier &objectName):
    kind(kindName), name(objectName)
{
}



std::ostream& operator<<(std::ostream &stream, const ObjectDefinition &o)
{
    if (o.name.empty())
        return stream << "new " << o.kind;
    else
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



bool operator<(const ObjectDefinition &a, const ObjectDefinition &b)
{
    if (a.kind < b.kind) {
        return true;
    } else if (a.kind > b.kind) {
        return false;
    } else {
        return (a.name < b.name);
    }
}



AttributeDefinition::AttributeDefinition(const Db::Identifier &attributeName, const Db::Value &assignedValue):
    attribute(attributeName), value(assignedValue)
{
}



std::ostream& operator<<(std::ostream &stream, const AttributeDefinition &a)
{
    if (a.value) {
        return stream << a.attribute << " " << boost::apply_visitor(NonOptionalValuePrettyPrint(), *(a.value));
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



ObjectDefinition stepInContext(const ObjectDefinition &currentObject, const ObjectDefinition &nestedObject)
{
    Db::Identifier newName = currentObject.name + "->" + nestedObject.name;
    return ObjectDefinition(nestedObject.kind, newName);
}



NonOptionalValuePrettyPrint::result_type NonOptionalValuePrettyPrint::operator()(const boost::posix_time::ptime &value) const
{
    return boost::gregorian::to_iso_extended_string(value.date()) + std::string(" ") + boost::posix_time::to_simple_string(value.time_of_day());
}



NonOptionalValuePrettyPrint::result_type NonOptionalValuePrettyPrint::operator()(const boost::gregorian::date &value) const
{
    return boost::gregorian::to_iso_extended_string(value);
}



NonOptionalValuePrettyPrint::result_type NonOptionalValuePrettyPrint::operator()(const std::string &value) const
{
    bool quote = (value.find('"') != std::string::npos);
    bool apost = (value.find('\'') != std::string::npos);
    bool space = ((value.find(' ') != std::string::npos) || (value.find('\t') != std::string::npos));
    BOOST_ASSERT(!(quote && apost && space));
    // No wrapping for string without spaces needed
    if (!space)
        return value;
    std::ostringstream ostr;
    // String with quotes should be wrapped in apostrophes
    if (quote) {
        ostr << "\'" << value << "\'";
        return ostr.str();
    }
    // String with apostrophes should be wrapped in quotes
    if (apost) {
        ostr << "\"" << value << "\"";
        return ostr.str();
    }
    // String with spaces only we can wrap in quotes
    ostr << "\"" << value << "\"";
    return ostr.str();
}



template <typename T>
NonOptionalValuePrettyPrint::result_type NonOptionalValuePrettyPrint::operator()(const T & value) const
{
    std::ostringstream ss;
    ss << value;
    return ss.str();
}



/////////////////////////Template instances for linker//////////////////////////

template NonOptionalValuePrettyPrint::result_type NonOptionalValuePrettyPrint::operator()(const std::string & value) const;

template NonOptionalValuePrettyPrint::result_type NonOptionalValuePrettyPrint::operator()(const double & value) const;

template NonOptionalValuePrettyPrint::result_type NonOptionalValuePrettyPrint::operator()(const int & value) const;

template NonOptionalValuePrettyPrint::result_type NonOptionalValuePrettyPrint::operator()(const bool & value) const;

template NonOptionalValuePrettyPrint::result_type NonOptionalValuePrettyPrint::operator()(const boost::asio::ip::address_v4 & value) const;

template NonOptionalValuePrettyPrint::result_type NonOptionalValuePrettyPrint::operator()(const boost::asio::ip::address_v6 & value) const;

template NonOptionalValuePrettyPrint::result_type NonOptionalValuePrettyPrint::operator()(const Db::MacAddress & value) const;

template NonOptionalValuePrettyPrint::result_type NonOptionalValuePrettyPrint::operator()(const std::set<Db::Identifier> & value) const;


}
}
