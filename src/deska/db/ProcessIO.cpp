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
#include "ProcessIO.h"

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
    // FIXME: change this to react to stderr traffic by throwing an exception, but only when there's any other traffic going on

    // The first "argument" is actually a process' name, but boost::process expects them separate
    std::string exe = arguments.front();

    childProcess = bp::launch(exe, arguments, ctx);
    using namespace boost::phoenix;
    dynamic_cast<bp::detail::systembuf*>(childProcess->get_stdout().rdbuf())->event_read_data.connect(bind(&ProcessIO::slotReadData, this, arg_names::_1));
}

ProcessIO::~ProcessIO()
{
    childProcess->terminate();
}

std::ostream *ProcessIO::writeStream()
{
    return &childProcess->get_stdin();
}

std::istream *ProcessIO::readStream()
{
    m_recentlyReadData.clear();
    return &childProcess->get_stdout();
}

void ProcessIO::slotReadData(const std::string &data)
{
    m_recentlyReadData.append(data);
}

std::string ProcessIO::recentlyReadData() const
{
    return m_recentlyReadData;
}

}
}
