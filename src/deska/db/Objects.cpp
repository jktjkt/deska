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
#include <boost/spirit/include/qi.hpp>

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
    case TYPE_IPV4_ADDRESS:
        return stream << "TYPE_IPV4_ADDRESS";
    case TYPE_IPV6_ADDRESS:
        return stream << "TYPE_IPV6_ADDRESS";
    case TYPE_MAC_ADDRESS:
        return stream << "MAC_ADDRESS";
    case TYPE_DATE:
        return stream << "TYPE_DATE";
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

std::ostream& operator<<(std::ostream &stream, const ObjectRelation& o)
{
    switch (o.kind) {
    case RELATION_MERGE_WITH:
        return stream << "mergeWith(" << o.target << ")";
    case RELATION_EMBED_INTO:
        return stream << "embedInto(" << o.target << ")";
    case RELATION_IS_TEMPLATE:
        return stream << "isTemplate(" << o.target << ")";
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
    ObjectRelation res;
    res.kind = RELATION_MERGE_WITH;
    res.target = target;
    return res;
}

ObjectRelation ObjectRelation::embedInto(const Identifier &target)
{
    ObjectRelation res;
    res.kind = RELATION_EMBED_INTO;
    res.target = target;
    return res;
}

ObjectRelation ObjectRelation::isTemplate(const Identifier &target)
{
    ObjectRelation res;
    res.kind = RELATION_IS_TEMPLATE;
    res.target = target;
    return res;
}

ObjectRelation ObjectRelation::templatized(const Identifier &target)
{
    ObjectRelation res;
    res.kind = RELATION_TEMPLATIZED;
    res.target = target;
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
    return !(a == b);
}


AttributeDefinition::AttributeDefinition(const Identifier &attributeName, const Value &assignedValue):
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


Identifier contextStackToPath(const ContextStack &contextStack)
{
    std::ostringstream ss;
    for (ContextStack::const_iterator it = contextStack.begin(); it != contextStack.end(); ++it) {
        if (it != contextStack.begin())
            ss << "->";
        ss << it->name;
    }
    return ss.str();
}

std::string contextStackToString(const ContextStack &contextStack)
{
    std::ostringstream ss;
    for (ContextStack::const_iterator it = contextStack.begin(); it != contextStack.end(); ++it) {
        if (it != contextStack.begin())
            ss << "->";
        ss << *it;
    }
    return ss.str();
}

std::vector<Identifier> PathToVector(const std::string &path)
{
    std::string::const_iterator first = path.begin();
    std::string::const_iterator last = path.end();

    std::vector<Identifier> identifiers;

    bool r = boost::spirit::qi::phrase_parse(first,last,
                                             +(boost::spirit::ascii::alnum | '_') % "->",
                                             boost::spirit::ascii::space, identifiers);
    if (!r || first != last)
        throw std::runtime_error("Deska::Db::PathToVector conversion failed while parsing " + path);
    
    return identifiers;
}

}
}
