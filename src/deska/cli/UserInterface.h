/*
* Copyright (C) 2011 Jan Kundrát <kundratj@fzu.cz>
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

#ifndef DESKA_USER_INTERFACE_H
#define DESKA_USER_INTERFACE_H

#include <string>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/noncopyable.hpp>


#include "UserInterfaceIO.h"
#include "DbInteraction.h"
#include "Parser.h"
#include "Exceptions.h"


namespace Deska
{
namespace Cli
{


/** @short Class for communication with the user.
*
*   User interface uses class Parser for parsing lines, that does not match any keyword, class DbInteraction for
*   communication with the database and with the Parser communicates through SignalsHandler, that is actively calling
*   functions for confirmation and applying actions connected with each signal that parser emits. For  all IO
*   operations is used class UserInterfaceIO.
*/
class UserInterface: public boost::noncopyable//, public rlmm::readline
{
public:

    /** @short Constructor initializes stream for communication with the user and pointers for parsing
    *          input and communication with the database.
    *
    *   @param dbInteraction Pointer to the class used for communication with the database
    *   @param parser Pointer to the parser used for parsing commands that are not any known keyword
    *   @param _io Pointer to the UserInterfaceIO class for IO oparations
    */
    UserInterface(DbInteraction *dbInteraction, Parser* parser, UserInterfaceIO *_io);

    //@{
    /** @short Functions for confirmation and applying actions connected with parser signals.
    *
    *   @see DbInteraction
    *   @see ParserSignals
    *   @see Parser
    */
    void applyCategoryEntered(const Db::ContextStack &context,
                         const Db::Identifier &kind, const Db::Identifier &object);
    void applySetAttribute(const Db::ContextStack &context,
                      const Db::Identifier &attribute, const Db::Value &value);
    void applyFunctionShow(const Db::ContextStack &context);
    void applyFunctionDelete(const Db::ContextStack &context);

    bool confirmCategoryEntered(const Db::ContextStack &context,
                         const Db::Identifier &kind, const Db::Identifier &object);
    bool confirmSetAttribute(const Db::ContextStack &context,
                      const Db::Identifier &attribute, const Db::Value &value);
    bool confirmFunctionShow(const Db::ContextStack &context);
    bool confirmFunctionDelete(const Db::ContextStack &context);
    //@}

    /** @short Reports any error to the user (error output).
    *
    *   @param errorMessage Error message to report
    */
    void reportError(const std::string &errorMessage);

    /** @short Function for listening to users input and calling appropriate actions. */
    void run();

private:

    /** @short Displays list of pending changesets, lets user to pick one and connects to it. */
    void resumeChangeset();

    /** @short Dump everything in the DB */
    void dumpDbContents();

    /** @short Prints help for CLI usage. */
    void printHelp();

    /** Pointer to the class used for communication with the database. */
    DbInteraction *m_dbInteraction;
    /** Pointer to the parser used for parsing commands that are not any known keyword. */
    Parser* m_parser;

    std::string prompt;
    UserInterfaceIO *io;

    bool inChangeset;
};


}
}


#endif // DESKA_USER_INTERFACE_H
