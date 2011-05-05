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
#include "Objects.h"

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
    return a.kind == b.kind && a.sourceAttribute == b.sourceAttribute && a.targetTableName == b.targetTableName;
}

bool operator!=(const ObjectRelation &a, const ObjectRelation &b)
{
    return !(a == b);
}

std::ostream& operator<<(std::ostream &stream, const ObjectRelation& o)
{
    switch (o.kind) {
    case RELATION_MERGE_WITH:
        return stream << "mergeWith(" << o.targetTableName << ", " << o.sourceAttribute << ")";
    case RELATION_EMBED_INTO:
        return stream << "embedInto(" << o.targetTableName << ")";
    case RELATION_IS_TEMPLATE:
        return stream << "isTemplate(" << o.targetTableName << ")";
    case RELATION_TEMPLATIZED:
        return stream << "templatized(" << o.targetTableName << ", " << o.sourceAttribute << ")";
    case RELATION_INVALID:
        return stream << "RELATION_INVALID";
    }
    return stream << "[Invalid relation: " << static_cast<int>(o.kind) << "]";
}

ObjectRelation::ObjectRelation(const ObjectRelationKind _kind, const Identifier &_targetTableName, const Identifier &_sourceAttribute):
    kind(_kind), targetTableName(_targetTableName), sourceAttribute(_sourceAttribute)
{
}

ObjectRelation ObjectRelation::mergeWith(const Identifier &targetTableName, const Identifier &sourceAttribute)
{
    ObjectRelation res;
    res.kind = RELATION_MERGE_WITH;
    res.targetTableName = targetTableName;
    res.sourceAttribute = sourceAttribute;
    return res;
}

ObjectRelation ObjectRelation::embedInto(const Identifier &into)
{
    ObjectRelation res;
    res.kind = RELATION_EMBED_INTO;
    res.targetTableName = into;
    return res;
}

ObjectRelation ObjectRelation::isTemplate(const Identifier &toWhichKind)
{
    ObjectRelation res;
    res.kind = RELATION_IS_TEMPLATE;
    res.targetTableName = toWhichKind;
    return res;
}

ObjectRelation ObjectRelation::templatized(const Identifier &byWhichKind, const Identifier &sourceAttribute)
{
    ObjectRelation res;
    res.kind = RELATION_TEMPLATIZED;
    res.targetTableName = byWhichKind;
    res.sourceAttribute = sourceAttribute;
    return res;
}

ObjectRelation::ObjectRelation()
{
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
    return !(a==b);
}


AttributeDefinition::AttributeDefinition(const Identifier &attributeName, const Value &assignedValue):
    attribute(attributeName), value(assignedValue)
{
}

std::ostream& operator<<(std::ostream &stream, const AttributeDefinition &a)
{
    return stream << a.attribute << " " << a.value;
}

}
}
