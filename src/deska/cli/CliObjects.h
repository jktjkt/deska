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

#ifndef DESKA_CLI_CLIOBJECTS_H
#define DESKA_CLI_CLIOBJECTS_H

#include <iosfwd>
#include "deska/db/Objects.h"


namespace Deska {
namespace Cli {

/** @short Structure for pairs kind name - object name. */
struct ObjectDefinition
{
    /** @short Constructor only assignes the data members.
    *
    *   @param kindName Name of the kind (eg. host)
    *   @param objectName Name of the instance of the kind (eg. hpv2)
    */
    ObjectDefinition(const Db::Identifier &kindName, const Db::Identifier &objectName);

    /** Name of the kind */
    Db::Identifier kind;
    /** Name of the instance of the kind */
    Db::Identifier name;
};

std::ostream& operator<<(std::ostream &stream, const ObjectDefinition &o);
bool operator==(const ObjectDefinition &a, const ObjectDefinition &b);
bool operator!=(const ObjectDefinition &a, const ObjectDefinition &b);
bool operator<(const ObjectDefinition &a, const ObjectDefinition &b);


/** @short Structure for pairs attribute name - attribute value. */
struct AttributeDefinition
{
    /** @short Constructor only assignes the data members.
    *
    *   @param attributeName Name of the attribute (eg. ip)
    *   @param assignedValue Value of the attribute (eg. 192.168.10.56)
    */
    AttributeDefinition(const Db::Identifier &attributeName, const Db::Value &assignedValue);

    /** Name of the attribute */
    Db::Identifier attribute;
    /** Value of the attribute */
    Db::Value value;
};

std::ostream& operator<<(std::ostream &stream, const AttributeDefinition &a);
bool operator==(const AttributeDefinition &a, const AttributeDefinition &b);
bool operator!=(const AttributeDefinition &a, const AttributeDefinition &b);

/** @short Generates new object definition from current object by stepping in the context of nested object.
*
*   Function does not check existance of any of the objects neither checks if the kind of the second can
*   be nested in the first one.
*
*   @param currentObject Current object from which we want to step in its parent.
*   @param nestedObject Parent of the currentObject with short name without ->
*/
ObjectDefinition stepInContext(const ObjectDefinition &currentObject, const ObjectDefinition &nestedObject);



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
    result_type operator()(const boost::posix_time::ptime &value) const;

    /** @short Pretty printer for dates that always uses our canonical representation */
    result_type operator()(const boost::gregorian::date &value) const;

    /** @short Pretty printer for strings */
    result_type operator()(const std::string &value) const;

    /** @short Pretty printer for booleans */
    result_type operator()(const bool &value) const;

    /** @short Pretty printer template for everything else -- simply let the operator<< do its job */
    template <typename T>
    result_type operator()(const T & value) const;
};


}
}

#endif // DESKA_CLI_CLIOBJECTS_H
