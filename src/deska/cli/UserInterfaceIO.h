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

#ifndef DESKA_USER_INTERFACE_IO_H
#define DESKA_USER_INTERFACE_IO_H

#include <string>
#include <iostream>
#include <vector>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/noncopyable.hpp>

#include "ReadlineWrapper.h"

#include "deska/db/Objects.h"
#include "deska/db/Revisions.h"
#include "Parser.h"


namespace Deska
{
namespace Cli
{


/** @brief Custom completions generator */
class CliCompleter: public ReadlineWrapper::Completer
{
public:
    /** @short Sets pointer to the praser for obtaining line completions.
    *
    *   @param parser Pointer to the parser
    *   @see Parser
    */
    CliCompleter(Parser* parser);

    virtual ~CliCompleter();
    /** @short Function for obtaining all possible lines.
    *
    *   @param line Line to be completed.
    *   @param start Beginning of the last word in the input.
    *   @param end Position of the input in the line.
    *   @return All possible lines, that can occur at this point. Whole lines will be generated, not only the endings.
    */
    virtual std::vector<std::string> getCompletions(const std::string &line,
                                            std::string::const_iterator start,
                                            std::string::const_iterator end);

private:
    /** Pointer to the parser for obtaining line completions. */
    Parser* m_parser;
};



/** @short Class for IO oparetions needed in a command line user interface with a standard iostream implementation. */
class UserInterfaceIO: public boost::noncopyable
{
public:

    /** @short Constructor initializes input reader and CLI constants.
    *
    *   @param parser Pointer to the parser
    *   @see Parser
    */
    UserInterfaceIO(Parser* parser);
    
    /** @short Destroys custom completer and line reader. */
    ~UserInterfaceIO();

    /** @short Reports any error to the user (error output).
    *
    *   @param errorMessage Error message to report
    */
    void reportError(const std::string &errorMessage);

    /** @short Prints some message to output stream.
    *
    *   @param message Message to print
    */
    void printMessage(const std::string &message);

    /** @short Displays confirmation message for deletion of a object and returns users choice.
    *
    *   @param object Object to be deleted
    *   @return True if the deletion was confirmed, else false
    */
    bool confirmDeletion(const Db::ObjectDefinition &object);

    /** @short Displays confirmation message for creation of a object and returns users choice.
    *
    *   @param object Object to be created
    *   @return True if the creation was confirmed, else false
    */
    bool confirmCreation(const Db::ObjectDefinition &object);

    /** @short Asks user to enter a commit message.
    *
    *   @return Entered message
    */
    std::string askForCommitMessage();

    /** @short Asks user to enter a detach message.
    *
    *   @return Entered message
    */
    std::string askForDetachMessage();

    /** @short Prints help for CLI usage. */
    void printHelp();

    /** @short Displays list of pending changesets and lets user to pick one.
    *
    *   @param pendingChangesets Vector of pending changesets
    *   @return Index number of the changeset in the vector of pending changesets or
    *           -1 for creating new changeset or -2 for no chnageset.
    */
    int chooseChangeset(const std::vector<Db::PendingChangeset> &pendingChangesets);

    /** @short Displays prompt and gets one line from the input.
    *
    *   @param prompt Prompt string.
    *   @return Read line
    */
    std::string readLine(const std::string &prompt);

    /** @short Prints list of attribute definitions with indentation.
    *
    *   @param attributes Vector of attributes to print
    *   @param indentLevel Level of indentation (number of "tabs")
    */
    void printAttributes(const std::vector<Db::AttributeDefinition> &attributes, int indentLevel);

    /** @short Prints list of object definitions with indentation.
    *
    *   @param objects Vector of objects to print
    *   @param indentLevel Level of indentation (number of "tabs")
    */
    void printObjects(const std::vector<Db::ObjectDefinition> &objects, int indentLevel, bool fullName);

    /** @short Prints an attribute definition with indentation.
    *
    *   @param attribute Attribute to print
    *   @param indentLevel Level of indentation (number of "tabs")
    */
    void printAttribute(const Db::AttributeDefinition &attribute, int indentLevel);

    /** @short Prints a object definition with indentation.
    *
    *   @param object Object to print
    *   @param indentLevel Level of indentation (number of "tabs")
    */
    void printObject(const Db::ObjectDefinition &object, int indentLevel, bool fullName);

    /** @short Prints "end" keyword with indentation.
    *
    *   @param indentLevel Level of indentation (number of "tabs")
    */
    void printEnd(int indentLevel);

private:

    /** @short Displays confirmation message and returns users choice.
    *
    *   @param prompt Message to confirm
    *   @return True if the message was confirmed, else false
    */
    bool askForConfirmation(const std::string &prompt);

    /** @short Construct string for indenting an output.
    *
    *   @param indentLevel Level of indentation (number of "tabs")
    *   @return String constructed from spaces for indenting
    */
    std::string indent(int indentLevel);

    /** Number of spaces for indenting an output. */
    unsigned int tabSize;
    /** Ending string of the prompt. */
    std::string promptEnd;

    /** Class used for history, line editting and tab completion. */
    ReadlineWrapper::Readline *reader;
    /** Custom completions generator. */
    CliCompleter *completer;
};



}
}

#endif  // DESKA_DB_INTERACTION_IO_H
