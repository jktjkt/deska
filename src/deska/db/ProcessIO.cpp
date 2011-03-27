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

#include "ProcessIO.h"
#include "3rd-party/process/boost/process.hpp"

namespace Deska {
namespace Db {

ProcessIO::ProcessIO(const std::vector<std::string> &arguments)
{
    namespace bp = boost::process;
    BOOST_ASSERT(!arguments.empty());
    bp::context ctx;
    ctx.environment = bp::self::get_environment();
    ctx.stdout_behavior = bp::capture_stream();
    ctx.stdin_behavior = bp::capture_stream();
    ctx.stderr_behavior = bp::inherit_stream();
    // FIXME: change this to react to stderr traffic by throwing an expcetion, but only when there's any other traffic going on

    // The first "argument" is actually a process' name, but boost::process expects them separate
    std::string exe = arguments.front();

    // FIXME: this is extremely ugly, but apparently required because there's no default constructor for the bp::process
    // [add a link to the boost-users post here when it makes it to archives]
    childProcess.reset(new bp::child(bp::launch(exe, arguments, ctx)));
}

ProcessIO::~ProcessIO()
{
    childProcess->terminate();
}

void ProcessIO::writeData(const std::string &data)
{
    childProcess->get_stdin() << data;
}

std::string ProcessIO::readData()
{
    std::string res;
    childProcess->get_stdout() >> res;
    return res;
}

}
}
