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


/** @short Singleton for storing, parsing and obtaining program configuration variables.
*
*   All variables are parsed from file "deska.ini".
*/
class CliConfig
{
public:
    /** @short Gets an instance of the singleton.
    *
    *   @return An instance of the singleton.
    */
    static CliConfig *getInstance();

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
    CliConfig(int _x);
    CliConfig(const CliConfig&);
    CliConfig& operator=(const CliConfig&);
    ~CliConfig();

    /** Map of parsed variables. */
    boost::program_options::variables_map configVars;

    int x;
};


}
}

#endif // DESKA_CLI_CLICONFIG_H
