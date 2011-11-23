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

#include <string>
#include <boost/program_options.hpp>


namespace Deska {
namespace Cli {


//@{
/** Definitions of variables parsed from config file or command line */
static const std::string DBConnection_Server = "DBConnection.Server";
static const std::string CLI_HistoryFilename = "CLI.HistoryFilename";
static const std::string CLI_HistoryLimit = "CLI.HistoryLimit";
static const std::string CLI_LineWidth = "CLI.LineWidth";
static const std::string CLI_NonInteractive = "CLI.NonInteractive";
//@}


/** @short Class for storing, parsing and obtaining program configuration variables. */
class CliConfig
{
public:

    /** @short Constructor loads configuration from command line and configuration file.
    *
    *   @param configFile Name of file with configuration
    *   @param argc Number of parameters from the command line
    *   @param argv Parameters from the command line
    */
    CliConfig(const std::string &configFile, int argc, char **argv);

    /** @short Function for obtaining values loaded from config file.
    *
    *   For obtaining value of variable "var" stored in section "Section" call this function with "Section.var".
    *   In config file we have something like var=value in [Section].
    *
    *   @param name Name of the var being obtained.
    *   @tparam T Type of the var being obtained.
    *   @return Variable value castet to the given type.
    */
    template <typename T>
    T getVar(const std::string &name);

private:
    /** Map of parsed variables. */
    boost::program_options::variables_map configVars;
};


}
}

#endif // DESKA_CLI_CLICONFIG_H
