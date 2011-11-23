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

#include <boost/lexical_cast.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <cstdlib>
#include "Connection_p.h"
#include "ProcessIO.h"
#include "UnixFdIO.h"

namespace Deska {
namespace Db {

#define DESKA_VIA_FD_R "DESKA_VIA_FD_R"
#define DESKA_VIA_FD_W "DESKA_VIA_FD_W"

Connection_p::Connection_p(const int rfd, const int wfd)
{
    if (rfd < 0) {
        throw std::runtime_error("Deska::Db::Connection_p: environment: illegal " DESKA_VIA_FD_R);
    }
    if (wfd < 0) {
        throw std::runtime_error("Deska::Db::Connection_p: environment: illegal " DESKA_VIA_FD_W);
    }
    io = new UnixFdIO(rfd, wfd);
    willRead.connect(boost::phoenix::bind(&IOSocket::readStream, *io));
    willWrite.connect(boost::phoenix::bind(&IOSocket::writeStream, *io));
    wantJustReadData.connect(boost::phoenix::bind(&IOSocket::recentlyReadData, *io));
}

Connection_p::Connection_p(const std::vector<std::string> &args)
{
    io = new ProcessIO(args);
    willRead.connect(boost::phoenix::bind(&IOSocket::readStream, *io));
    willWrite.connect(boost::phoenix::bind(&IOSocket::writeStream, *io));
    wantJustReadData.connect(boost::phoenix::bind(&IOSocket::recentlyReadData, *io));
}

Connection_p::~Connection_p()
{
    delete io;
}

}
}
