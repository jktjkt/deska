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

#include "ContextStack.h"


namespace Deska {
namespace Cli {


ContextStackItem::ContextStackItem()
{
}



ContextStackItem::ContextStackItem(const Db::Identifier &kindName, const Db::Identifier &objectName):
    kind(kindName), name(objectName), filter(boost::optional<Db::Filter>())
{
}



ContextStackItem::ContextStackItem(const Db::Identifier &kindName, const Db::Filter &objectsFilter):
    kind(kindName), name(""), filter(objectsFilter)
{
}



std::ostream& operator<<(std::ostream &stream, const ContextStackItem &i)
{
    stream << i.kind;
    if (i.filter)
        stream << " where " << *(i.filter);
    else
        stream << " " << i.name;
    return stream;
}



bool operator==(const ContextStackItem &a, const ContextStackItem &b)
{
    if ((a.filter) && (b.filter))
        return a.kind == b.kind && *(a.filter) == *(b.filter);
    else if (!(a.filter) && !(b.filter))
        return a.kind == b.kind && a.name == b.name;
    else
        return false;
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
        if (it->filter)
            throw std::runtime_error("Deska::Cli::contextStackToPath: Can not convert context stack with filters to path");
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
        if (it->filter)
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
    std::string::const_iterator first = path.begin();
    std::string::const_iterator last = path.end();

    std::vector<Db::Identifier> identifiers;

    bool r = boost::spirit::qi::phrase_parse(first,last,
                                             +(boost::spirit::ascii::alnum | '_') % "->",
                                             boost::spirit::ascii::space, identifiers);
    if (!r)
        throw std::runtime_error("Deska::Cli::pathToVector: Conversion failed while parsing " + path);

    if (first != last) {
        bool r2 = boost::spirit::qi::phrase_parse(first, last, boost::spirit::qi::lit("->"), boost::spirit::ascii::space);
        if (!r2 || (first != last))
            throw std::runtime_error("Deska::Cli::pathToVector: Conversion failed while parsing " + path);
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
