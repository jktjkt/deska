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

#ifndef DESKA_DB_PROCESSIO_H
#define DESKA_DB_PROCESSIO_H

#include <string>
#include <vector>
#include <tr1/memory>

namespace boost {
namespace process {
class child;
}
}

namespace Deska {
namespace Db {

/** @short Encapsulation of a child process */
class ProcessIO
{
public:
    ProcessIO(const std::vector<std::string> &arguments);
    virtual ~ProcessIO();

    /** @short Read stuff from the child process

    Note that the usual semantics of the read() apply here as well, notably that this call could easily return no data even
    though more are on the fly. This one is to be fixed later.
    */
    std::string readData();

    /** @short Write data to the child process */
    void writeData(const std::string &data);

private:
    std::tr1::shared_ptr<boost::process::child> childProcess;
};

}
}

#endif // DESKA_DB_PROCESSIO_H
