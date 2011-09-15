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

#ifndef DESKA_CLI_CLICOMMANDS_LOG_H
#define DESKA_CLI_CLICOMMANDS_LOG_H

#include <string>
#include <vector>

#include "CliCommands.h"
#include "PredefinedRules.h"
#include "ParserIterator.h"
#include "deska/db/Revisions.h"
#include "deska/db/Filter.h"


namespace Deska {
namespace Cli {


class UserInterface;
class Log;


template<typename T> class LogFilterParseError;
template<typename T> class LogFilterParser;

/** @short Cli command.
*
*   Command for operations with revisions and history.
*
*   @see Command
*/
class Log: public Command
{
public:
    /** @short Constructor sets command name and completion pattern.
    *
    *   @param userInterface Pointer to the UserInterface
    */
    Log(UserInterface *userInterface);

    virtual ~Log();

    /** @short Function for operations with revisions and history.
    *
    *   @param params Unused here.
    */
    virtual void operator()(const std::string &params);

    /** @short Adds parse error to stack while parsing filter for revisions.
    *
    *   @param error Error recognised by some of error handlers
    */
    void reportParseError(const LogFilterParseError<iterator_type> &error);

private:
    /** Parser of parameters */
    LogFilterParser<iterator_type> *filterParser;
    /** Stack with errors, that occures during parsing filter for revisions. */
    std::vector<LogFilterParseError<iterator_type> > parseErrors;
};


}
}

#endif // DESKA_CLI_CLICOMMANDS_LOG_H
