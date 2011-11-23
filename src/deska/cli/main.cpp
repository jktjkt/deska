/*
* Copyright (C) 2011 Tomáš Hubík <hubik.tomas@gmail.com>
* Copyright (C) 2011 Jan Kundrát <kundratj@fzu.cz>
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


#include <iostream>
#include <string>
#include "deska/db/Connection.h"
#include "deska/db/JsonException.h"
#include "DbInteraction.h"
#include "ParserSignals.h"
#include "UserInterface.h"
#include "UserInterfaceIO.h"
#include "Parser.h"
#include "CliConfig.h"

int main(int argc, char **argv)
{
    Deska::Cli::CliConfig config("deska.ini", argc, argv);
    std::vector<std::string> args;
    args.push_back(std::string(CMAKE_CURRENT_SOURCE_DIR) + "/src/deska/server/app/deska_server.py");
    // FIXME: switch to boost::program_options, see redmine #179
    Deska::Db::Connection conn(args);
    Deska::Cli::Parser parser(&conn);
    Deska::Cli::DbInteraction db(&conn);
    Deska::Cli::UserInterfaceIO io(&parser, &config);
    Deska::Cli::UserInterface ui(&db, &parser, &io, &config);
    Deska::Cli::SignalsHandler(&parser, &ui);
    ui.run();
    return 0;
}
