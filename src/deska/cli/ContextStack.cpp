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
#include <boost/spirit/include/qi.hpp>

#include "ParserIterator.h"
#include "ContextStack.h"


namespace Deska {
namespace Cli {


ContextStackItem::ContextStackItem()
{
}



ContextStackItem::ContextStackItem(const Db::Identifier &kindName, const Db::Identifier &objectName):
    kind(kindName), name(objectName), filter(boost::optional<Db::Filter>()), itemType(CONTEXT_STACK_ITEM_TYPE_OBJECT)
{
}



ContextStackItem::ContextStackItem(const Db::Identifier &kindName, const boost::optional<Db::Filter> &objectsFilter):
    kind(kindName), name(""), filter(objectsFilter), itemType(CONTEXT_STACK_ITEM_TYPE_FILTER)
{
}



ContextStackConversionError::ContextStackConversionError(const std::string &message): std::runtime_error(message)
{
}



std::ostream& operator<<(std::ostream &stream, const ContextStackItem &i)
{
    stream << i.kind;
    if (i.itemType == ContextStackItem::CONTEXT_STACK_ITEM_TYPE_FILTER)
        if (i.filter)
            stream << " where " << *(i.filter);
        else
            stream << " where *";
    else if (i.itemType == ContextStackItem::CONTEXT_STACK_ITEM_TYPE_OBJECT)
        stream << " " << i.name;
    else
        throw std::domain_error("ContextStackItemType out of range");
    return stream;
}



bool operator==(const ContextStackItem &a, const ContextStackItem &b)
{
    if ((a.itemType == ContextStackItem::CONTEXT_STACK_ITEM_TYPE_FILTER) && (b.itemType == ContextStackItem::CONTEXT_STACK_ITEM_TYPE_FILTER)) {
        if ((a.filter) && (b.filter))
            return a.kind == b.kind && *(a.filter) == *(b.filter);
        else if (!(a.filter) && !(b.filter))
            return a.kind == b.kind;
        else return false;
    } else if (!(a.filter) && !(b.filter)) {
        return a.kind == b.kind && a.name == b.name;
    } else {
        return false;
    }
}



bool operator!=(const ContextStackItem &a, const ContextStackItem &b)
{
    return !(a == b);
}



Db::Identifier contextStackToPath(const ContextStack &contextStack)
{
    std::ostringstream ss;
    for (ContextStack::const_iterator it = contextStack.begin(); it != contextStack.end(); ++it) {
        if (it != contextStack.begin())
            ss << "->";
        if (it->itemType == ContextStackItem::CONTEXT_STACK_ITEM_TYPE_FILTER)
            throw ContextStackConversionError("Deska::Cli::contextStackToPath: Can not convert context stack with filters to path");
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
        if (it->itemType == ContextStackItem::CONTEXT_STACK_ITEM_TYPE_FILTER)
            ss << "filter on " << it->kind;
        else
            ss << *it;
    }
    return ss.str();
}



std::string dumpContextStack(const ContextStack &contextStack)
{
    std::ostringstream ss;
    for (ContextStack::const_iterator it = contextStack.begin(); it != contextStack.end(); ++it) {
        if (it != contextStack.begin())
            ss << "->";
        ss << *it;
    }
    return ss.str();
}



std::vector<Db::Identifier> pathToVector(const Db::Identifier &path)
{
    namespace ascii = boost::spirit::ascii;
    namespace qi = boost::spirit::qi;

    std::string::const_iterator first = path.begin();
    std::string::const_iterator last = path.end();

    std::vector<Db::Identifier> identifiers;

    qi::rule<iterator_type, std::string(), ascii::space_type> tIdentifier;
    tIdentifier %= qi::raw[qi::lexeme[+(*qi::lit('-') >> +(ascii::alnum | qi::lit('_')))]];

    bool r = qi::phrase_parse(first,last, (tIdentifier % "->"),ascii::space, identifiers);
    if (!r)
        throw ContextStackConversionError("Deska::Cli::pathToVector: Conversion failed while parsing " + path);

    if (first != last) {
        bool r2 = qi::phrase_parse(first, last, qi::lit("->"), ascii::space);
        if (!r2 || (first != last))
            throw ContextStackConversionError("Deska::Cli::pathToVector: Conversion failed while parsing " + path);
        else
            identifiers.push_back(Db::Identifier());
    }
    
    return identifiers;
}



Db::Identifier vectorToPath(const std::vector<Db::Identifier> &identifiers)
{
    std::ostringstream ss;
    for (std::vector<Db::Identifier>::const_iterator it = identifiers.begin(); it != identifiers.end(); ++it) {
        if (it != identifiers.begin())
            ss << "->";
        ss << *it;
    }
    return ss.str();
}


}
}
