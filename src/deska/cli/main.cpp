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
#include "DbInteraction.h"
#include "ParserSignals.h"
#include "UserInterface.h"
#include "UserInterfaceIO.h"
#include "Parser.h"

int main()
{
    Deska::Db::Connection conn;
    Deska::Cli::Parser parser(&conn);
    Deska::Cli::DbInteraction db(&conn);
    Deska::Cli::UserInterfaceIO io(&parser);
    Deska::Cli::UserInterface ui(&db, &parser, &io);
    Deska::Cli::SignalsHandler(&parser, &ui);
    ui.run();
    return 0;
}
