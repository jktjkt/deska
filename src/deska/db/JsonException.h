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

#ifndef DESKA_JSONEXCEPTION_H
#define DESKA_JSONEXCEPTION_H

#include <stdexcept>
#include "Api.h"
#include "libebt/libebt_backtraceable.hh"

namespace Deska {
namespace Db {

/** @short INTERNAL: tag for the libebt */
struct JsonExceptionTag {};

/** @short INTERNAL: convenience typedef for exception reporting */
typedef libebt::BacktraceContext<JsonExceptionTag> JsonContext;

/** @short INTERNAL: convenience class for marking context relevant to both API and JSON */
class JsonCommandContext
{
public:
    JsonCommandContext(const std::string &ctx);
private:
    ApiContext m_apiContext;
    JsonContext m_jsonContext;
};

/** @short An error occured during parsing of the server's response */
class JsonParseError: public std::runtime_error, public libebt::Backtraceable<JsonExceptionTag>
{
    std::string m_completeError;
protected:
    JsonParseError(const std::string &message);
public:
    virtual ~JsonParseError() throw ();
    virtual const char* what() const throw();
    virtual std::string whatWithBacktrace() const throw();
    void addRawJsonData(const std::string &data);
};

/** @short The received data cannot be converted to JSON

This exception indicates that the data we received are not syntacticaly valid JSON data.
*/
class JsonSyntaxError: public JsonParseError
{
public:
    JsonSyntaxError(const std::string &message);
    virtual ~JsonSyntaxError() throw ();
};

/** @short The received JSON data does not conform to the Deska DBAPI specification */
class JsonStructureError: public JsonParseError
{
public:
    JsonStructureError(const std::string &message);
    virtual ~JsonStructureError() throw ();
};

}
}

#endif // DESKA_JSONEXCEPTION_H
