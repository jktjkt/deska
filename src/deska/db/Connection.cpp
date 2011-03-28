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
#include <boost/spirit/include/phoenix_core.hpp>
#include "Connection.h"
#include "ProcessIO.h"

namespace Deska {
namespace Db {

Connection::Connection(): io(0)
{
    // FIXME: don't hardcode these
    std::vector<std::string> args;
    // How to use this: run `socat -d -d -d PIPE:foo -` in the build dir before starting this example
    args.push_back("/usr/bin/socat");
    args.push_back("-");
    args.push_back("PIPE:foo");
    /*args.push_back("/usr/bin/python");
    args.push_back("first-conversation.py");*/

    io = new ProcessIO(args);

    using namespace boost::phoenix;
    writeString.connect(bind(&ProcessIO::writeData, io, arg_names::_1));
    readString.connect(bind(&ProcessIO::readData, io));
}

Connection::~Connection()
{
    delete io;
}


}
}
