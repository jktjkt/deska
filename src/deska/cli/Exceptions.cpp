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

namespace Deska {
namespace CLI {

ParserException::ParserException(const std::string &message): m(message), pos(input.end())
{
}

ParserException::ParserException(const std::string &message, const std::string &input_, const std::string::const_iterator &where):
    m(message), input(input_), pos()
{
    // FIXME: fix the iterator!
}

ParserException::~ParserException() throw ()
{
}

#define DESKA_ECBODY(Class, Parent) \
Class::Class(const std::string &message): Parent(message) {}\
Class::Class(const std::string &message, const std::string &input_, const std::string::const_iterator &where): \
        Parent(message, input_, where) {}


DESKA_ECBODY(UndefinedAttributeError, ParserException);
DESKA_ECBODY(InvalidAttributeDataTypeError, ParserException);
DESKA_ECBODY(NestingError, ParserException);

#undef DESKA_ECBODY

}
}

