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

#include <boost/spirit/include/phoenix_bind.hpp>
#include "Connection.h"
#include "ProcessIO.h"

namespace Deska {
namespace Db {

Connection::Connection(): io(0)
{
    // FIXME: don't hardcode these
    std::vector<std::string> args;
    args.push_back(std::string(CMAKE_CURRENT_SOURCE_DIR) + "/src/deska/server/app/deska_server.py");
    io = new ProcessIO(args);
    willRead.connect(boost::phoenix::bind(&ProcessIO::readStream, *io));
    willWrite.connect(boost::phoenix::bind(&ProcessIO::writeStream, *io));
    wantJustReadData.connect(boost::phoenix::bind(&ProcessIO::recentlyReadData, *io));
}

Connection::~Connection()
{
    delete io;
}


}
}
