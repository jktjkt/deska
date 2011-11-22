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

#include "JsonException.h"

namespace Deska {
namespace Db {

JsonParseError::JsonParseError(const std::string &message): std::runtime_error(message)
{
}

JsonParseError::~JsonParseError() throw ()
{
}

void JsonParseError::addRawJsonData(const std::string &data)
{
    std::ostringstream ss;
    ss << std::runtime_error::what() << std::endl << "Raw JSON data read from the process: '" << data << "'";
    m_completeError = ss.str();
}

const char* JsonParseError::what() const throw()
{
    return m_completeError.empty() ? std::runtime_error::what() : m_completeError.c_str();
}

std::string JsonParseError::whatWithBacktrace() const throw()
{
    // We're required not to throw, so we have to use a generic catch-all block here
    try {
        std::ostringstream ss;
        ss << "* " << backtrace("\n * ") << what() << std::endl;
        return ss.str();
    } catch (...) {
        return what();
    }
}

JsonCommandContext::JsonCommandContext(const std::string &ctx):
    m_apiContext(std::string("In ") + ctx + " API method"),
    m_jsonContext(std::string("In ") + ctx + " API method")
{
}

JsonSyntaxError::JsonSyntaxError(const std::string &message): JsonParseError(message)
{
}

JsonSyntaxError::~JsonSyntaxError() throw ()
{
}

JsonStructureError::JsonStructureError(const std::string &message): JsonParseError(message)
{
}

JsonStructureError::~JsonStructureError() throw ()
{
}

}
}
