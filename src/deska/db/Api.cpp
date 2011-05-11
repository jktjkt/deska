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
#include "Api.h"

namespace Deska {
namespace Db {

Api::~Api()
{
}

RemoteDbError::RemoteDbError(const std::string &message): std::runtime_error(message)
{
}

/** @short Virtual destructor

The has to be defined and declared in order to force the vtable construction, which is needed for selectively catching
these eceptions.
*/
RemoteDbError::~RemoteDbError() throw ()
{
}

std::string RemoteDbError::whatWithBacktrace() const throw()
{
    // We're required not to throw, so we have to use a generic catch-all block here
    try {
        std::ostringstream ss;
        ss << "* " << backtrace("\n * ") << what() << std::endl;
        if (m_rawResponseData) {
            ss << "Server response was: " << *m_rawResponseData << std::endl;
        }
        return ss.str();
    } catch (...) {
        return what();
    }
}

void RemoteDbError::setRawResponseData(const std::string &data)
{
    m_rawResponseData = data;
}

#define REMOTEEXCEPTION(CLASS) \
CLASS::CLASS(const std::string &message): RemoteDbError(message) {} \
CLASS::~CLASS() throw () {}

REMOTEEXCEPTION(NotFoundError)
REMOTEEXCEPTION(NoChangesetError)
REMOTEEXCEPTION(SqlError)
REMOTEEXCEPTION(ServerError)

#undef REMOTEEXCEPTION

}
}
