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



AttributeDefinition::AttributeDefinition(const Db::Identifier &attributeName, const Db::Value &assignedValue):
    attribute(attributeName), value(assignedValue)
{
}


/** @short Pretty printer for the Deska::Db::NonOptionalValue in the CLI

The pretty printers for dates and timestamps are required ebcause otherwise boost::date_time tries to be clever and messes up our
dates to an English form.  This is bad, because showing "2010-Nov-09" while accepting only "2010-11-09" sucks.

If you thought for a minute "hey, maybe boost::date_time respects current locale through LC_TIME" -- it doesn't.

These functions are shamelessly copied from JsonConversionTraits<boost::gregorian::date> in src/deska/db/JsonExtraction.cpp.  Yes,
copy-paste is evil, but depending on functions from the JSON library in this context looks a little more evil to me.
*/
struct NonOptionalValuePrettyPrint: public boost::static_visitor<std::string>
{
    /** @short Pretty printer for timestamps */
    result_type operator()(const boost::posix_time::ptime &value) const
    {
        return boost::gregorian::to_iso_extended_string(value.date()) + std::string(" ") + boost::posix_time::to_simple_string(value.time_of_day());
    }

    /** @short Pretty printer for dates that always uses our canonical representation */
    result_type operator()(const boost::gregorian::date &value) const
    {
        return boost::gregorian::to_iso_extended_string(value);
    }

    /** @short Pretty printer template for everything else -- simply let the operator<< do its job */
    template<typename T>
    result_type operator()(const T & value) const
    {
        std::ostringstream ss;
        ss << value;
        return ss.str();
    }
};


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


}
}
