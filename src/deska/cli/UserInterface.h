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
#include <iostream>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/noncopyable.hpp>

//#include "rlmm/readline.hh"

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
*   functions for confirmation and applying actions connected with each signal that parser emits.
*/
class UserInterface: public boost::noncopyable//, public rlmm::readline
{
public:

    /** @short Constructor initializes stream for communication with the user and pointers for parsing
    *          input and communication with the database.
    *
    *   @param outStream Stream for standart output
    *   @param errStream Stream for error output
    *   @param inStream Stream for input
    *   @param dbInteraction Pointer to the class used for communication with the database
    *   @param parser Pointer to the parser used for parsing commands that are not any known keyword
    */
    UserInterface(std::ostream &outStream, std::ostream &errStream, std::istream &inStream,
                  DbInteraction *dbInteraction, Parser* parser);

    //@{
    /** @short Functions for confirmation and applying actions connected with parser signals.
    *
    *   @see DbInteraction
    *   @see ParserSignals
    *   @see Parser
    */
    void applyCategoryEntered(const std::vector<Db::ObjectDefinition> &context,
                         const Db::Identifier &kind, const Db::Identifier &object);
    void applySetAttribute(const std::vector<Db::ObjectDefinition> &context,
                      const Db::Identifier &attribute, const Db::Value &value);
    void applyFunctionShow(const std::vector<Db::ObjectDefinition> &context);
    void applyFunctionDelete(const std::vector<Db::ObjectDefinition> &context);

    bool confirmCategoryEntered(const std::vector<Db::ObjectDefinition> &context,
                         const Db::Identifier &kind, const Db::Identifier &object);
    bool confirmSetAttribute(const std::vector<Db::ObjectDefinition> &context,
                      const Db::Identifier &attribute, const Db::Value &value);
    bool confirmFunctionShow(const std::vector<Db::ObjectDefinition> &context);
    bool confirmFunctionDelete(const std::vector<Db::ObjectDefinition> &context);
    //@}

    /** @short Reports any error to the user (error output).
    *
    *   @param errorMessage Error message to report
    */
    void reportError(const std::string &errorMessage);

    /** @short Displays confirmation message and returns users choice.
    *
    *   @param prompt Message to confirm
    *   @return True if the message was confirmed, else false
    */
    bool askForConfirmation(const std::string &prompt);

    /** @short Dump everything in the DB */
    void dumpDbContents();
    /** @short Print attributes of given object.
    *
    *   @param object Object for which the attributes are printed
    */
    void printAttributes(const Db::ObjectDefinition &object);

    /** @short Make all actions needed to commit current changeset including commit message request. */
    void commitChangeset();
    /** @short Detaches from current changeset. */
    void detachFromChangeset();
    /** @short Aborts current changeset. */
    void abortChangeset();

    /** @short Prints help for CLI usage. */
    void printHelp();

    /** @short Starts the cli after construction.
    *
    *   Displays list of pending changesets, connects to one, or creates new and starts event loop.
    */
    void run();
    /** @short Function for listening to users input and calling appropriate actions. */
    void eventLoop();

private:

    /** Stream for standart output. */
    std::ostream out;
    /** Stream for error output. */
    std::ostream err;
    /** Stream for input. */
    std::istream in;
    
    /** Pointer to the class used for communication with the database. */
    DbInteraction *m_dbInteraction;
    /** Pointer to the parser used for parsing commands that are not any known keyword. */
    Parser* m_parser;

    std::string prompt;
};


}
}


#endif // DESKA_USER_INTERFACE_H
