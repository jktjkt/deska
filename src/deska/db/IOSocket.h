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

#ifndef DESKA_DB_IOSOCKET_H
#define DESKA_DB_IOSOCKET_H

#include <iosfwd>
#include <string>

namespace Deska {
namespace Db {

/** @short Encapsulation of a child process

This class encapsulates access to a newly launched child process, and sets up several debugging hooks for retrieving
the data read from the child process.
*/
class IOSocket
{
public:
    IOSocket();
    virtual ~IOSocket();

    /** @short Obtain a stream for reading and clear the reading debug buffer

    In addition to returning an istream instance, this function will clear our internal buffer which contains data
    that anyone retrieved from that stream.

    @see recentlyReadData()
    */
    virtual std::istream *readStream() = 0;

    /** @short Obtain a stream for writing

    No catching of debug data is performed at this point, because it is not needed anywhere (yet).
    */
    virtual std::ostream *writeStream() = 0;

    /** @short Return the data read since the last call to readStream()

    The "data read" refer to a sequence of bytes really obtained from the underlying pipe and not those actually
    retrieved from the istream. For now, this limitation, or a bug, is considered to be of little relevance.
    */
    virtual std::string recentlyReadData() const = 0;
};

}
}

#endif // DESKA_DB_IOSOCKET_H
