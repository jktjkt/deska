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
    std::string exe = getenv("EDITOR");
    if (exe.empty()) {
        exe = "vim";
    }

    std::vector<std::string> args;
    args.push_back(exe);
    args.push_back(fileName);

    namespace bp = boost::process;
    bp::context ctx;
    ctx.environment = bp::self::get_environment();
    ctx.stdout_behavior = bp::inherit_stream();
    ctx.stdin_behavior = bp::inherit_stream();
    ctx.stderr_behavior = bp::inherit_stream();

    bp::child proc = bp::launch(args.front(), args, ctx);
    proc.wait();
}


Pager::Pager(const std::string &message)
{
    std::string exe = getenv("PAGER");
    if (exe.empty()) {
        exe = "less";
    }

    std::vector<std::string> args;
    args.push_back(exe);

    namespace bp = boost::process;
    bp::context ctx;
    ctx.environment = bp::self::get_environment();
    ctx.stdout_behavior = bp::inherit_stream();
    ctx.stdin_behavior = bp::capture_stream();
    ctx.stderr_behavior = bp::inherit_stream();

    bp::child proc = bp::launch(args.front(), args, ctx);
    proc.get_stdin() << message;
    proc.get_stdin().close();
    proc.wait();
}

}
}
