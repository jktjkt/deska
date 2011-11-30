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

#ifndef DESKA_CLI_CLICONFIG_H
#define DESKA_CLI_CLICONFIG_H

#include "CliConfigBase.h"


namespace Deska {
namespace Cli {



/** @short Class for storing, parsing and obtaining program configuration variables. */
class CliConfig: public CliConfigBase
{
public:

    /** @short Constructor loads configuration from command line and configuration file.
    *
    *   @param configFile Name of file with configuration
    *   @param argc Number of parameters from the command line
    *   @param argv Parameters from the command line
    */
    CliConfig(const std::string &configFile, int argc, char **argv);
};



}
}

#endif // DESKA_CLI_CLICONFIG_H
