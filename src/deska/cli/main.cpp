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
#include "cli-config.h"

#if DESKA_HAVE_GITVERSION
#include "gitversion-include.cpp"
const char *deskaVersion = deskaGitVersion;
#else
const char *deskaVersion = DESKA_VERSION_STR;
#endif

void sigpipeHandler(int signum)
{
    std::cerr << "Deska server not responding, or dead." << std::endl
              << "Exitting Deska CLI." << std::endl;
    exit(2);
}


int main(int argc, char **argv)
{
    signal(SIGPIPE, sigpipeHandler);

    const char configFile[] = "deska.ini";
    try {
        Deska::Cli::CliConfig config(configFile, argc, argv);

        std::vector<std::string> unregConfFileOpt = config.unregistredConfigFileOptions();
        std::vector<std::string> unregCmdLineOpt = config.unregistredCommandLineOptions();
        if (!unregConfFileOpt.empty() || !unregCmdLineOpt.empty()) {
            std::cerr << "Uknown options found:" << std::endl;
            if (!unregConfFileOpt.empty()) {
                std::cerr << "In config file " << configFile << ":" << std::endl;
                for (std::vector<std::string>::iterator it = unregConfFileOpt.begin(); it != unregConfFileOpt.end(); ++it) {
                    if (it != unregConfFileOpt.begin())
                        std::cerr << ", ";
                    std::cerr << *it;
                }
                std::cerr << std::endl;
            }
            if (!unregCmdLineOpt.empty()) {
                std::cerr << "On command line:" << std::endl;
                for (std::vector<std::string>::iterator it = unregCmdLineOpt.begin(); it != unregCmdLineOpt.end(); ++it) {
                    if (it != unregCmdLineOpt.begin())
                        std::cerr << ", ";
                    std::cerr << *it;
                }
                std::cerr << std::endl;
            }
            std::cerr << "Use option --help for information about supported options and usage." << std::endl;
            return 2;
        }

        if (config.defined(Deska::Cli::CmdLine_Help)) {
            std::cout << "Deska, a tool for central administration of a grid site" << std::endl << std::endl
                      << "Usage: " << DESKA_EXECUTABLE << " [OPTION]" << std::endl << std::endl
                      << config.usage() << std::endl
                      << "This is free software, dual-licensed under the GPLv2 and GPLv3." << std::endl << std::endl
                      << "Please report all bugs to: deska@lists.flaska.net" << std::endl
                      << "Deska home page: http://projects.flaska.net/projects/show/deska" << std::endl;
            return 0;
        }

        if (config.defined(Deska::Cli::CmdLine_Version)) {
            std::cout << "Deska, a tool for central administration of a grid site" << std::endl
                      << "Version " << deskaVersion << std::endl << std::endl
                      << "Copyright (C) 2011 Tomas Hubik <hubik.tomas@gmail.com>" << std::endl
                      << "Copyright (C) 2011 Lukas Kerpl <lukas.kerpl@gmail.com>" << std::endl
                      << "Copyright (C) 2011 Martina Krejcova <martinka.krejcova@seznam.cz>" << std::endl
                      << "Copyright (C) 2011 Jan Kundrat <kundratj@fzu.cz>" << std::endl << std::endl
                      << "This is free software, dual-licensed under the GPLv2 and GPLv3." << std::endl << std::endl
                      << "Please report all bugs to: deska@lists.flaska.net" << std::endl
                      << "Deska home page: http://projects.flaska.net/projects/show/deska" << std::endl;
            return 0;
        }

        Deska::Db::Connection conn(config.getVar<std::vector<std::string> >(Deska::Cli::DBConnection_Server));
        Deska::Cli::Parser parser(&conn);
        Deska::Cli::DbInteraction db(&conn);
        Deska::Cli::UserInterfaceIO io(&parser, &config);
        Deska::Cli::UserInterface ui(&db, &parser, &io, &config);
        Deska::Cli::SignalsHandler(&parser, &ui);
        
        if (config.defined(Deska::Cli::CmdLine_Dump)) {
            if (ui.executeCommand(Deska::Cli::CmdLine_Dump, config.getVar<std::string>(Deska::Cli::CmdLine_Dump)))
                return 0;
            else
                return 3;
        }
        if (config.defined(Deska::Cli::CmdLine_Backup)) {
            if (ui.executeCommand(Deska::Cli::CmdLine_Backup, config.getVar<std::string>(Deska::Cli::CmdLine_Backup)))
                return 0;
            else
                return 3;
        }
        if (config.defined(Deska::Cli::CmdLine_Restore)) {
            if (ui.executeCommand(Deska::Cli::CmdLine_Restore, config.getVar<std::string>(Deska::Cli::CmdLine_Restore)))
                return 0;
            else
                return 3;
        }
        if (config.defined(Deska::Cli::CmdLine_Execute)) {
            if (ui.executeCommand(Deska::Cli::CmdLine_Execute, config.getVar<std::string>(Deska::Cli::CmdLine_Execute)))
                return 0;
            else
                return 3;
        }

        ui.run();
        return 0;
    } catch (boost::program_options::error &e) {
        std::cerr << "Error while obtaining program options: " << e.what() << std::endl;
        return 1;
    }
}
