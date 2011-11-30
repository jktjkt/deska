/*
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


#include "CliConfigTest.h"


CliConfigTest::CliConfigTest(const std::string &configFile, int argc, char **argv): CliConfigBase()
{
    namespace po = boost::program_options;

    boost::program_options::options_description options("Deska CLI Test Options");
    options.add_options()
        // FIXME: Proper handling of required options is not available in Boost 1.41
        (Deska::Cli::DBConnection_Server.c_str(), po::value<std::vector<std::string> >()->multitoken()/*->required()*/, "path to executable for connection to Deska server including arguments")
        (Deska::Cli::CLI_NonInteractive.c_str(), po::value<bool>()->default_value(false), "flag singalising, that all questions concerning object deletion, creation, etc. will be automaticly confirmed");

    loadOptions(options, configFile, argc, argv);
}
