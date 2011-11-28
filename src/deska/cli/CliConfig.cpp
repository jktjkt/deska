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
        ((CmdLine_Help + ",h").c_str(), "produces this help message")
        ((CmdLine_Version + ",v").c_str(), "prints version information")
        ((CmdLine_NonInteractive + ",n").c_str(), "flag singalising, that all questions concerning object deletion, creation, etc. will be automaticly confirmed")
        (DBConnection_Server.c_str(), po::value<std::vector<std::string> >()->multitoken(), "path to executable for connection to Deska server including arguments")
        (CLI_HistoryFilename.c_str(), po::value<std::string>(), "name of file with history")
        (CLI_HistoryLimit.c_str(), po::value<unsigned int>()->default_value(64), "number of lines stored in history")
        (CLI_LineWidth.c_str(), po::value<unsigned int>()->default_value(0), "width of line for wrapping")
        (CLI_NonInteractive.c_str(), po::value<bool>()->default_value(false), "flag singalising, that all questions concerning object deletion, creation, etc. will be automaticly confirmed");

    std::ifstream configStream(configFile.c_str());
    po::parsed_options commandLineOptions = po::command_line_parser(argc, argv).options(options).allow_unregistered().run();
    po::store(commandLineOptions, configVars);
    po::parsed_options configFileOptions = po::parse_config_file(configStream, options, true);
    po::store(configFileOptions, configVars);
    unregCmdLineOptions = po::collect_unrecognized(commandLineOptions.options, po::include_positional);
    unregConfigFileOptions = po::collect_unrecognized(configFileOptions.options, po::include_positional);

    std::ostringstream ostr;
    ostr << options;
    configUsage = ostr.str();
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



bool CliConfig::defined(const std::string &name)
{
    return configVars.count(name);
}



std::vector<std::string> CliConfig::unregistredConfigFileOptions()
{
    return unregConfigFileOptions;
}



std::vector<std::string> CliConfig::unregistredCommandLineOptions()
{
    return unregCmdLineOptions;
}



std::string CliConfig::usage()
{
    return configUsage;
}


/////////////////////////Template instances for linker//////////////////////////

template int CliConfig::getVar(const std::string &name);

template unsigned int CliConfig::getVar(const std::string &name);

template bool CliConfig::getVar(const std::string &name);

template std::string CliConfig::getVar(const std::string &name);

template std::vector<std::string> CliConfig::getVar(const std::string &name);


}
}
