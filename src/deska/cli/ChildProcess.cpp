/*
* Copyright (C) 2011 Jan Kundrát <kundratj@fzu.cz>
* Copyright (C) 2011 Tomáš Hubík <hubik.tomas@gmail.com>
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

#include "ChildProcess.h"


namespace Deska
{
namespace Cli
{


Editor::Editor(const std::string &fileName)
{
    namespace bp = boost::process;
    BOOST_ASSERT(!fileName.empty());
    bp::context ctx;
    ctx.environment = bp::self::get_environment();
    ctx.stdout_behavior = bp::inherit_stream();
    ctx.stdin_behavior = bp::inherit_stream();
    ctx.stderr_behavior = bp::inherit_stream();
    // FIXME: change this to react to stderr traffic by throwing an exception, but only when there's any other traffic going on

    char *defaultEditor = getenv("EDITOR");
    std::string exe;
    if (!defaultEditor)
        exe = std::string("vim");
    else
        exe = std::string(defaultEditor);

    std::vector<std::string> arguments;
    arguments.push_back(exe);
    arguments.push_back(fileName);
    childProcess = bp::launch(exe, arguments, ctx);
}



Editor::~Editor()
{
    childProcess->terminate();
}



Pager::Pager()
{
    namespace bp = boost::process;
    bp::context ctx;
    ctx.environment = bp::self::get_environment();
    ctx.stdout_behavior = bp::inherit_stream();
    ctx.stdin_behavior = bp::capture_stream();
    ctx.stderr_behavior = bp::inherit_stream();
    // FIXME: change this to react to stderr traffic by throwing an exception, but only when there's any other traffic going on

    char *defaultPager = getenv("PAGER");
    std::string exe;
    if (!defaultPager)
        exe = std::string("less");
    else
        exe = std::string(defaultPager);

    std::vector<std::string> arguments;
    arguments.push_back(exe);
    childProcess = bp::launch(exe, arguments, ctx);
}



Pager::~Pager()
{
    childProcess->terminate();
}



std::ostream *Pager::writeStream()
{
    return &childProcess->get_stdin();
}


}
}
