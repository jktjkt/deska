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

#ifndef DESKA_USER_INTERFACE_IO_BASE_H
#define DESKA_USER_INTERFACE_IO_BASE_H

#include <string>
#include <iosfwd>
#include <vector>

#include <boost/noncopyable.hpp>

#include "CliObjects.h"
#include "deska/db/Revisions.h"
#include "deska/db/ObjectModification.h"


namespace Deska
{
namespace Cli
{


/** @short Base class for IO operations needed in a command line user interface with a standard iostream implementation. */
class UserInterfaceIOBase: public boost::noncopyable
{
public:

    /** @short Constructor for an empty object for testing purposes. */
    UserInterfaceIOBase() {};

    /** @short Destroys custom completer and line reader. */
    virtual ~UserInterfaceIOBase() {};

    /** @short Reports any error to the user (error output).
    *
    *   @param errorMessage Error message to report
    */
    virtual void reportError(const std::string &errorMessage) = 0;

    /** @short Prints some message to output stream.
    *
    *   @param message Message to print
    */
    virtual void printMessage(const std::string &message) = 0;

    /** @short Displays some message in pager like less.
    *
    *   @param message Message to display
    */
    virtual void displayInPager(const std::string &message) = 0;

    /** @short Runs an editor and opens file for editting
    *
    *   @param editFile File to edit
    */
    virtual void editFile(const std::string &fileName) = 0;

    /** @short Displays confirmation message for deletion of a object and returns users choice.
    *
    *   @param object Object to be deleted
    *   @return True if the deletion was confirmed, else false
    */
    virtual bool confirmDeletion(const ObjectDefinition &object) = 0;

    /** @short Displays confirmation message for deletion of nested objects and returns users choice.
    *
    *   @param objects Objects to be deleted
    *   @return True if the deletion was confirmed, else false
    */
    virtual bool confirmDeletionNested(const std::vector<ObjectDefinition> &nestedObjects) = 0;

    /** @short Displays confirmation message for deletion of contained objects and returns users choice.
    *
    *   @param objects Objects to be deleted
    *   @return True if the deletion was confirmed, else false
    */
    virtual bool confirmDeletionConnected(const std::vector<ObjectDefinition> &connectedObjects) = 0;

    /** @short Displays confirmation message for rename of contained objects and returns users choice.
    *
    *   @param objects Objects to be renamed
    *   @return True if the rename was confirmed, else false
    */
    virtual bool confirmRenameConnected(const std::vector<ObjectDefinition> &connectedObjects) = 0;

    /** @short Displays confirmation message for creation of a object and returns users choice.
    *
    *   @param object Object to be created
    *   @return True if the creation was confirmed, else false
    */
    virtual bool confirmCreation(const ObjectDefinition &object) = 0;

    /** @short Displays confirmation message for creation of a object when the object will be connected with another
    *          and returns users choice.
    *
    *   @param object Object to be created
    *   @return True if the creation was confirmed, else false
    */
    virtual bool confirmCreationConnection(const ObjectDefinition &object) = 0;

    /** @short Displays confirmation message for creation of a object when the object will be connected with another
    *          and returns users choice.
    *
    *   @param object Object to be created
    *   @param connectedObjects Objects, that will be connected with one being created
    *   @return True if the creation was confirmed, else false
    */
    virtual bool confirmCreationConnection(const ObjectDefinition &object,
                                           const std::vector<ObjectDefinition> &connectedObjects) = 0;

    /** @short Displays confirmation message for restoration of a deleted object and returns users choice.
    *
    *   @param object Object to be restored
    *   @return True if the restoration was confirmed, else false
    */
    virtual bool confirmRestoration(const ObjectDefinition &object) = 0;

    /** @short Displays confirmation message and returns users choice.
    *
    *   @param prompt Message to confirm
    *   @return True if the message was confirmed, else false
    */
    virtual bool askForConfirmation(const std::string &prompt) = 0;

    /** @short Asks user to enter a commit message.
    *
    *   @return Entered message
    */
    virtual std::string askForCommitMessage() = 0;

    /** @short Asks user to enter a detach message.
    *
    *   @return Entered message
    */
    virtual std::string askForDetachMessage() = 0;

    /** @short Prints help for CLI usage.
    *
    *   @param cliCommands Map of CLI commands with their usages.
    *   @param parserKeywords Map of Parser commands with their usages.
    */
    virtual void printHelp(const std::map<std::string, std::string> &cliCommands,
                           const std::map<std::string, std::string> &parserKeywords) = 0;

    /** @short Prints help for CLI command.
    *
    *   @param cmdName Command name
    *   @param cmdDscr Command description
    */
    virtual void printHelpCommand(const std::string &cmdName, const std::string &cmdDscr) = 0;

    /** @short Prints help for Parser keyword.
    *
    *   @param keywordName Keyword name
    *   @param keywordDscr Keyword description
    */
    virtual void printHelpKeyword(const std::string &keywordName, const std::string &keywordDscr) = 0;

    /** @short Prints help for DB kind.
    *
    *   @param kindName Kind name
    *   @param kindAttrs Kind attributes
    *   @param nestedKinds Nested kinds
    *   @param nestedKinds Contained kinds with referencing attribute name
    *   @param nestedKinds Containable kinds with referencing attribute name
    */
    virtual void printHelpKind(const std::string &kindName,
                               const std::vector<std::pair<std::string, std::string> > &kindAttrs,
                               const std::vector<std::string> &nestedKinds,
                               const std::vector<std::pair<std::string, std::string> > &containedKinds,
                               const std::vector<std::pair<std::string, std::string> > &containableKinds) = 0;

    /** @short Prints help for DB defined kinds.
    *
    *   @param kinds Vector of defined kind names
    */
    virtual void printHelpShowKinds(const std::vector<std::string> &kinds) = 0;

    /** @short Displays list of pending changesets and lets user to pick one.
    *
    *   @param pendingChangesets Vector of pending changesets
    *   @return Index number of the changeset in the vector of pending changesets or
    *           -1 for creating new changeset or -2 for no chnageset.
    */
    virtual int chooseChangeset(const std::vector<Db::PendingChangeset> &pendingChangesets) = 0;

    /** @short Displays prompt and gets one line from the input.
    *
    *   @param prompt Prompt string.
     *   @return Pair of read line and bool indicating if EOF was read
    */
    virtual std::pair<std::string, bool> readLine(const std::string &prompt) = 0;

    /** @short Prints list of attribute definitions with indentation.
    *
    *   @param attributes Vector of attributes to print
    *   @param indentLevel Level of indentation (number of "tabs")
    */
    virtual void printAttributes(const std::vector<AttributeDefinition> &attributes, int indentLevel,
                                 std::ostream &out = std::cout) = 0;

    /** @short Prints list of attribute definitions with indentation and origin of templatized ones.
    *
    *   @param attributes Vector of pairs where first item is attribute to print and second is origin of the attribute
    *   @param indentLevel Level of indentation (number of "tabs")
    */
    virtual void printAttributesWithOrigin(
        const std::vector<std::pair<AttributeDefinition, Db::Identifier> > &attributes, int indentLevel,
        std::ostream &out = std::cout) = 0;

    /** @short Prints list of object definitions with indentation.
    *
    *   @param objects Vector of objects to print
    *   @param indentLevel Level of indentation (number of "tabs")
    */
    virtual void printObjects(const std::vector<ObjectDefinition> &objects, int indentLevel,
                              bool fullName, std::ostream &out = std::cout) = 0;

    /** @short Prints an attribute definition with indentation.
    *
    *   @param attribute Attribute to print
    *   @param indentLevel Level of indentation (number of "tabs")
    */
    virtual void printAttribute(const AttributeDefinition &attribute, int indentLevel,
                        std::ostream &out = std::cout) = 0;

    /** @short Prints an attribute definition with indentation and origin of templatized ones.
    *
    *   @param attribute Attribute to print
    *   @param origin Origin of the value if the attribute is inherited from some templete, else empty string
    *   @param indentLevel Level of indentation (number of "tabs")
    */
    virtual void printAttributeWithOrigin(const AttributeDefinition &attribute, const Db::Identifier &origin,
                                          int indentLevel, std::ostream &out = std::cout) = 0;

    /** @short Prints a object definition with indentation.
    *
    *   @param object Object to print
    *   @param indentLevel Level of indentation (number of "tabs")
    */
    virtual void printObject(const ObjectDefinition &object, int indentLevel, bool fullName,
                     std::ostream &out = std::cout) = 0;

    /** @short Prints "end" keyword with indentation.
    *
    *   @param indentLevel Level of indentation (number of "tabs")
    */
    virtual void printEnd(int indentLevel, std::ostream &out = std::cout) = 0;

    /** @short Displays list of revisions.
    *
    *   @param revisions Vector of revisions metadata to print.
    */
    virtual void printRevisions(const std::vector<Db::RevisionMetadata> &revisions) = 0;

    /** @short Prints difference using list of modifications.
    *
    *   @param modifications Vector of modifications representing the diff.
    */
    virtual void printDiff(const std::vector<Db::ObjectModificationResult> &modifications) = 0;

    /** @short Adds completion string to the completions vector in the CliCompleter.
    *
    *   @param completion Completion to add.
    */
    virtual void addCommandCompletion(const std::string &completion) = 0;
};



}
}

#endif  // DESKA_USER_INTERFACE_IO_BASE_H
