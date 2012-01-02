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

#include "Exceptions.h"
#include <sstream>
#include <typeinfo>

namespace Deska {
namespace Cli {


bool operator==(const ParserException &a, const ParserException &b)
{
    return a.eq(b);
}

ParserException::ParserException(const std::string &message): m(message), pos(input.end())
{
}

ParserException::ParserException(const std::string &message, const std::string &input_, const std::string::const_iterator &where):
    m(message), input(input_), pos(input.begin())
{
    pos += where - input_.begin();
}

ParserException::~ParserException() throw ()
{
}

std::string ParserException::dump() const
{
    return dumpHelper("ParserException");
}

int ParserException::offset() const
{
    return static_cast<int>(pos - input.begin());
}

std::string ParserException::dumpHelper(const std::string &className) const
{
    std::ostringstream ss;
    ss << className << ": " << m;
    if (!input.empty()) {
        ss << " Error in string \"" << input << "\" at offset " << offset() << ".";
    }
    ss.flush();
    return ss.str();
}

#define DESKA_ECBODY(Class, Parent) \
Class::Class(const std::string &message): Parent(message) {}\
Class::Class(const std::string &message, const std::string &input_, const std::string::const_iterator &where): \
        Parent(message, input_, where) {} \
bool Class::eq(const std::exception &other) const \
{ \
    try { \
        const Class &e = dynamic_cast<const Class&>(other); \
        return e.m == m && e.input == input && e.pos - e.input.begin() == pos - input.begin(); \
    } catch (const std::bad_cast&) { \
        return false; \
   } } \
std::string Class::dump() const { return Parent::dumpHelper(#Class); }


DESKA_ECBODY(UndefinedAttributeError, ParserException);
DESKA_ECBODY(InvalidAttributeDataTypeError, ParserException);
DESKA_ECBODY(NestingError, ParserException);
DESKA_ECBODY(InvalidObjectKind, ParserException);
DESKA_ECBODY(MalformedIdentifier, ParserException);
DESKA_ECBODY(ObjectDefinitionNotFound, ParserException);
DESKA_ECBODY(ObjectNotFound, ParserException);
DESKA_ECBODY(NoAttributesOrKindsDefined, ParserException);
DESKA_ECBODY(MassCreatingEmbeddedError, ParserException);

#undef DESKA_ECBODY

}
}

