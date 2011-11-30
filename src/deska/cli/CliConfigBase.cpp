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
#include "CliConfigBase.h"


namespace Deska {
namespace Cli {



CliConfigBase::CliConfigBase()
{
}



CliConfigBase::~CliConfigBase()
{
}



template <typename T>
T CliConfigBase::getVar(const std::string &name)
{
    if (configVars.count(name)) {
        return configVars[name].as<T>();
    } else {
        std::ostringstream ostr;
        ostr << "missing required option " << name;
        throw boost::program_options::error(ostr.str());
        // FIXME: Proper handling of required options is not available in Boost 1.41
        //throw boost::program_options::required_option(name);
    }
}



bool CliConfigBase::defined(const std::string &name)
{
    return configVars.count(name);
}



std::vector<std::string> CliConfigBase::unregistredConfigFileOptions()
{
    return unregConfigFileOptions;
}



std::vector<std::string> CliConfigBase::unregistredCommandLineOptions()
{
    return unregCmdLineOptions;
}



std::string CliConfigBase::usage()
{
    return configUsage;
}



void CliConfigBase::loadOptions(const boost::program_options::options_description &options,
                                const std::string &configFile, int argc, char **argv)
{
    namespace po = boost::program_options;

    std::ifstream configStream(configFile.c_str());
    po::parsed_options commandLineOptions = po::command_line_parser(argc, argv).options(options).allow_unregistered().run();
    po::store(commandLineOptions, configVars);
    po::parsed_options configFileOptions = po::parse_config_file(configStream, options, true);
    po::store(configFileOptions, configVars);
    unregCmdLineOptions = po::collect_unrecognized(commandLineOptions.options, po::include_positional);
    unregConfigFileOptions = po::collect_unrecognized(configFileOptions.options, po::include_positional);

    // FIXME: Proper handling of required options is not available in Boost 1.41
    /*if (!(configVars.count(CmdLine_Help) || configVars.count(CmdLine_Version)))
        po::notify(configVars);*/

    std::ostringstream ostr;
    ostr << options;
    configUsage = ostr.str();
}


/////////////////////////Template instances for linker//////////////////////////

template int CliConfigBase::getVar(const std::string &name);

template unsigned int CliConfigBase::getVar(const std::string &name);

template bool CliConfigBase::getVar(const std::string &name);

template std::string CliConfigBase::getVar(const std::string &name);

template std::vector<std::string> CliConfigBase::getVar(const std::string &name);


}
}
