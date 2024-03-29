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

#ifndef DESKA_CLI_CLICONFIG_BASE_H
#define DESKA_CLI_CLICONFIG_BASE_H

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
static const std::string CmdLine_NonInteractive = "non-interactive";
static const std::string CmdLine_Help = "help";
static const std::string CmdLine_Version = "version";
static const std::string CmdLine_Dump = "dump";
static const std::string CmdLine_Backup = "backup";
static const std::string CmdLine_Restore = "restore";
static const std::string CmdLine_Execute = "execute";
//@}


/** @short Class for storing, parsing and obtaining program configuration variables. */
class CliConfigBase
{
public:

    /** @short Constructor does nothing special */
    CliConfigBase();

    /** @short Destroys this class. */
    virtual ~CliConfigBase();

    /** @short Function for obtaining values loaded from config file.
    *
    *   For obtaining value of variable "var" stored in section "Section" call this function with "Section.var".
    *   In config file we have something like var=value in [Section].
    *
    *   @param name Name of the var being obtained.
    *   @tparam T Type of the var being obtained.
    *   @return Variable value casted to the given type.
    */
    template <typename T>
    T getVar(const std::string &name);

    /** @short Function for checking if some war war defined in config file or on command line.
    *
    *   @param name Name of the var being checked.
    *   @return True if variable was found, else false.
    */
    bool defined(const std::string &name);

    //@{
    /** @short Functions for obtaining lists of unregistred options. */
    std::vector<std::string> unregistredConfigFileOptions();
    std::vector<std::string> unregistredCommandLineOptions();
    //@}

    /** @short Produces well formated string with list of all parameters */
    std::string usage();

protected:
    /** @short Only updates usage
    *
    *   @param options Options definitions
    */
    void loadOptions(const boost::program_options::options_description &options);
    /** @short Loads configuration values from file
    *
    *   @param options Options definitions
    *   @param configFile Name of file with configuration
    */
    void loadOptions(const boost::program_options::options_description &options,
                     const std::string &configFile);
    /** @short Loads configuration values from command line
    *
    *   @param options Options definitions
    *   @param argc Number of parameters from the command line
    *   @param argv Parameters from the command line
    */
    void loadOptions(const boost::program_options::options_description &options,
                     int argc, char **argv);
    /** @short Loads configuration values from command line and from file
    *
    *   @param options Options definitions
    *   @param configFile Name of file with configuration
    *   @param argc Number of parameters from the command line
    *   @param argv Parameters from the command line
    */
    void loadOptions(const boost::program_options::options_description &options,
                     const std::string &configFile, int argc, char **argv);

private:
    /** Map of parsed registered options. */
    boost::program_options::variables_map configVars;
    //@{
    /** Vectors of unregistred options */
    std::vector<std::string> unregCmdLineOptions;
    std::vector<std::string> unregConfigFileOptions;
    //@}
    /** Well formated string with list of all parameters */
    std::string configUsage;
};



}
}

#endif // DESKA_CLI_CLICONFIG_BASE_H
