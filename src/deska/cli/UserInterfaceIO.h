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

#include "deska/db/Objects.h"
#include "deska/db/Revisions.h"


namespace Deska
{
namespace Cli
{


/** @short Class for IO oparetions needed in a command line user interface with a standard iostream implementation. */
class UserInterfaceIO: public boost::noncopyable
{
public:

    /** @short Constructor initializes streams for communication with the user.
    *
    *   @param outStream Stream for standart output
    *   @param errStream Stream for error output
    *   @param inStream Stream for input
    */
    UserInterfaceIO(std::ostream &outStream, std::ostream &errStream, std::istream &inStream);

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

    /** @short Displays prompt with ending >.
    *
    *   @param prompt Prompt string.
    */
    void printPrompt(const std::string &prompt);

    /** @short Gets one line from the input stream.
    *
    *   @return Read line
    */
    std::string readLine();

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
    void printObjects(const std::vector<Db::ObjectDefinition> &objects, int indentLevel);  

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
    void printObject(const Db::ObjectDefinition &object, int indentLevel);

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

    /** Stream for standart output. */
    std::ostream out;
    /** Stream for error output. */
    std::ostream err;
    /** Stream for input. */
    std::istream in;

    /** Number of spaces for indenting an output. */
    unsigned int tabSize;
    /** Ending string of the prompt. */
    std::string promptEnd;
};

}
}

#endif  // DESKA_DB_INTERACTION_IO_H
