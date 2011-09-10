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

#ifndef DESKA_DB_UNIXFDIO_H
#define DESKA_DB_UNIXFDIO_H

#include <vector>
#include <boost/optional.hpp>
#include <tr1/memory>
#include "boost/process.hpp"
#include "IOSocket.h"

namespace Deska {
namespace Db {

/** @short Implementation of IO operations over a pair of already open file descriptors

This class provides a C++ interface to a pair of file descriptors, perhaps a tuple of pipes. It comes handy when
trying to work over a pair of FDs inherited from a parent process.
*/
class UnixFdIO: public IOSocket
{
public:
    /** @short Read from the @arg readingFd, write to @arg writingFd */
    UnixFdIO(const int readingFd, const int writingFd);
    virtual ~UnixFdIO();

    /** @short Obtain a stream for reading and clear the reading debug buffer

    In addition to returning an istream instance, this function will clear our internal buffer which contains data
    that anyone retrieved from that stream.

    @see recentlyReadData()
    */
    std::istream *readStream();

    /** @short Obtain a stream for writing

    No catching of debug data is performed at this point, because it is not needed anywhere (yet).
    */
    std::ostream *writeStream();

    void slotReadData(const std::string &data);

    /** @short Return the data read since the last call to readStream()

    The "data read" refer to a sequence of bytes really obtained from the underlying pipe and not those actually
    retrieved from the istream. For now, this limitation, or a bug, is considered to be of little relevance.
    */
    std::string recentlyReadData() const;
private:
    /** @short Stream for reading */
    boost::shared_ptr<boost::process::pistream> reading_;
    /** @short Stream for writing */
    boost::shared_ptr<boost::process::postream> writing_;

    /** @short Buffer of recently read data */
    std::string m_recentlyReadData;
};

}
}

#endif // DESKA_DB_UNIXFDIO_H
