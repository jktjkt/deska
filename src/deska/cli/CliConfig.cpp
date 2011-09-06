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



CliConfig *CliConfig::getInstance()
{
    static CliConfig inst(0);
    return &inst;
}



template <typename T>
T CliConfig::getVar(const std::string &name)
{
    return configVars[name].as<T>();
}



CliConfig::CliConfig(int _x): x(_x)
{
    namespace po = boost::program_options;

    boost::program_options::options_description options("Deska CLI Options");
    options.add_options()
        ("DBConnection.Server", po::value<std::string>(), "path to executable for connection to Deska server");

    std::ifstream configStream("deska.ini");
    po::store(po::parse_config_file(configStream, options), configVars);
}



CliConfig::~CliConfig()
{
}


}
}
