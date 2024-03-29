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


#include <cstdlib>
#include <boost/algorithm/string.hpp>
#include "boost/process.hpp"
#include "ChildProcess.h"


namespace Deska
{
namespace Cli
{

std::string maybe_add_full_path(std::string exe)
{
    if (!boost::algorithm::starts_with(exe, "/") &&
        !boost::algorithm::starts_with(exe, "./") &&
        !boost::algorithm::starts_with(exe, "../")) {
        exe = boost::process::find_executable_in_path(exe);
    }
    return exe;
}

Editor::Editor(const std::string &fileName)
{
    namespace bp = boost::process;

    std::string exe = maybe_add_full_path(std::getenv("EDITOR") ? std::getenv("EDITOR") : "vim");
    std::vector<std::string> args;
    args.push_back(exe);
    args.push_back(fileName);
    
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
    namespace bp = boost::process;

    std::string exe = maybe_add_full_path(std::getenv("PAGER") ? std::getenv("PAGER") : "less");
    std::vector<std::string> args;
    args.push_back(exe);
    
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
