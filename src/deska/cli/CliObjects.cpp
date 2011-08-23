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



std::ostream& operator<<(std::ostream &stream, const AttributeDefinition &a)
{
    if (a.value) {
        return stream << a.attribute << " " << *(a.value);
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
