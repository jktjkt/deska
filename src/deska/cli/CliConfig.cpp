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


#include <fstream>
#include "CliConfig.h"


namespace Deska {
namespace Cli {



CliConfig::CliConfig(const std::string &configFile, int argc, char **argv)
{
    namespace po = boost::program_options;

    boost::program_options::options_description options("Deska CLI Options");
    options.add_options()
        (DBConnection_Server.c_str(), po::value<std::string>(), "path to executable for connection to Deska server")
        (DBConnection_User.c_str(), po::value<std::string>(), "Deska user")
        (DBConnection_DB.c_str(), po::value<std::string>(), "Deska DB to connect to")
        (CLI_HistoryFilename.c_str(), po::value<std::string>(), "name of file with history")
        (CLI_HistoryLimit.c_str(), po::value<int>()->default_value(64), "number of lines stored in history");

    std::ifstream configStream(configFile.c_str());
    po::store(po::parse_command_line(argc, argv, options), configVars);
    po::store(po::parse_config_file(configStream, options), configVars);
}



template <typename T>
T CliConfig::getVar(const std::string &name)
{
    if (configVars.count(name)) {
        return configVars[name].as<T>();
    } else {
        std::ostringstream ss;
        ss << "Configuration error: Option '" << name << "' not found neither in config file nor on command line.";
        throw std::runtime_error(ss.str());
    }
}



/////////////////////////Template instances for linker//////////////////////////

template int CliConfig::getVar(const std::string &name);

template std::string CliConfig::getVar(const std::string &name);


}
}
