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

#ifndef DESKA_CLI_EXCEPTIONS_H
#define DESKA_CLI_EXCEPTIONS_H

#include <exception>
#include <string>

namespace Deska {
namespace CLI {

/** @short An exception triggered by an error in user-submitted data */
class ParserException: public std::exception
{
public:
    ParserException(const std::string &message);
    ParserException(const std::string &message, const std::string &input_, const std::string::const_iterator &where);
    virtual ~ParserException() throw ();
    virtual std::string dump() const;
protected:
    std::string m;
    std::string input;
    std::string::const_iterator pos;
    friend bool operator==(const ParserException &a, const ParserException &b);
    virtual bool eq(const std::exception &other) const = 0;
    std::string dumpHelper(const std::string &className) const;
};

#define DESKA_EXCEPTION(Class, Parent) \
    class Class: public Parent {public: \
    Class(const std::string &message);\
    Class(const std::string &message, const std::string &input_, const std::string::const_iterator &where);\
    virtual std::string dump() const;\
    protected: virtual bool eq(const std::exception &other) const; \
}

/** @short Tried to specify an attribute which is not recognized at this point */
DESKA_EXCEPTION(UndefinedAttributeError, ParserException);

/** @short The attribute has another data type */
DESKA_EXCEPTION(InvalidAttributeDataTypeError, ParserException);

/** @short Attempted to embed objects of incompatible type into each other */
DESKA_EXCEPTION(NestingError, ParserException);

/** @short A top-level object has an invalid type */
DESKA_EXCEPTION(InvalidObjectKind, ParserException);

/** @short Malformed identifier */
DESKA_EXCEPTION(MalformedIdentifier, ParserException);

/** @short Compare two exceptions for being equal */
bool operator==(const ParserException &a, const ParserException &b);

}
}

#endif // DESKA_CLI_EXCEPTIONS_H
