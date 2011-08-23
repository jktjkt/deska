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
#include "boost/process.hpp"

#include "CliObjects.h"
#include "deska/db/Revisions.h"
#include "ReadlineWrapper.h"
#include "Parser.h"
#include "UserInterfaceIOBase.h"


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
    
    /** @short Adds completion string to the completions vector in the CliCompleter.
    *
    *   @param completion Completion to add.
    */
    void addCommandCompletion(const std::string &completion);

private:
    /** Pointer to the parser for obtaining line completions. */
    Parser* m_parser;
    /** Vector with completions for all CLI commands. */
    std::vector<std::string> commandCompletions;
};


/** @short Visitor for printing oject modifications. */
struct ModificationPrinter: public boost::static_visitor<void> {
    //@{
    /** @short Function for printing single object modification.
    *
    *   @param modification Instance of modifications from Db::ObjectModification variant.
    */
    void operator()(const Db::CreateObjectModification &modification) const;
    void operator()(const Db::DeleteObjectModification &modification) const;
    void operator()(const Db::RenameObjectModification &modification) const;
    void operator()(const Db::SetAttributeModification &modification) const;
    //@}
};



/** @short Class for IO operations needed in a command line user interface with a standard iostream implementation. */
class UserInterfaceIO: public UserInterfaceIOBase
{
public:

    /** @short Constructor for an empty object for testing purposes. */
    UserInterfaceIO();

    /** @short Constructor initializes input reader and CLI constants.
    *
    *   @param parser Pointer to the parser
    *   @see Parser
    */
    UserInterfaceIO(Parser* parser);
    
    /** @short Destroys custom completer and line reader. */
    virtual ~UserInterfaceIO();

    /** @short Reports any error to the user (error output).
    *
    *   @param errorMessage Error message to report
    */
    virtual void reportError(const std::string &errorMessage);

    /** @short Prints some message to output stream.
    *
    *   @param message Message to print
    */
    virtual void printMessage(const std::string &message);

    /** @short Displays some message in pager like less.
    *
    *   @param message Message to display
    */
    virtual void displayInPager(const std::string &message);

    /** @short Runs an editor and opens file for editting
    *
    *   @param editFile File to edit
    */
    virtual void editFile(const std::string &fileName);

    /** @short Displays confirmation message for deletion of a object and returns users choice.
    *
    *   @param object Object to be deleted
    *   @return True if the deletion was confirmed, else false
    */
    virtual bool confirmDeletion(const ObjectDefinition &object);

    /** @short Displays confirmation message for creation of a object and returns users choice.
    *
    *   @param object Object to be created
    *   @return True if the creation was confirmed, else false
    */
    virtual bool confirmCreation(const ObjectDefinition &object);

    /** @short Displays confirmation message for restoration of a deleted object and returns users choice.
    *
    *   @param object Object to be restored
    *   @return True if the restoration was confirmed, else false
    */
    virtual bool confirmRestoration(const ObjectDefinition &object);

    /** @short Asks user to enter a commit message.
    *
    *   @return Entered message
    */
    virtual std::string askForCommitMessage();

    /** @short Asks user to enter a detach message.
    *
    *   @return Entered message
    */
    virtual std::string askForDetachMessage();

    /** @short Prints help for CLI usage.
    *
    *   @param cliCommands Map of CLI commands with their usages.
    *   @param parserKeywords Map of Parser commands with their usages.
    */
    virtual void printHelp(const std::map<std::string, std::string> &cliCommands,
                           const std::map<std::string, std::string> &parserKeywords);

    /** @short Prints help for CLI command.
    *
    *   @param cmdName Command name
    *   @param cmdDscr Command description
    */
    virtual void printHelpCommand(const std::string &cmdName, const std::string &cmdDscr);

    /** @short Prints help for Parser keyword.
    *
    *   @param keywordName Keyword name
    *   @param keywordDscr Keyword description
    */
    virtual void printHelpKeyword(const std::string &keywordName, const std::string &keywordDscr);

    /** @short Prints help for DB kind.
    *
    *   @param kindName Kind name
    *   @param kindAttrs Kind attributes
    *   @param nestedKinds Nested kinds
    */
    virtual void printHelpKind(const std::string &kindName,
                               const std::vector<std::pair<std::string, std::string> > &kindAttrs,
                               const std::vector<std::string> &nestedKinds);

    /** @short Prints help for DB defined kinds.
    *
    *   @param kinds Vector of defined kind names
    */
    virtual void printHelpShowKinds(const std::vector<std::string> &kinds);

    /** @short Displays list of pending changesets and lets user to pick one.
    *
    *   @param pendingChangesets Vector of pending changesets
    *   @return Index number of the changeset in the vector of pending changesets or
    *           -1 for creating new changeset or -2 for no chnageset.
    */
    virtual int chooseChangeset(const std::vector<Db::PendingChangeset> &pendingChangesets);

    /** @short Displays prompt and gets one line from the input.
    *
    *   @param prompt Prompt string.
    *   @return Pair of read line and bool indicating if EOF was read
    */
    virtual std::pair<std::string, bool> readLine(const std::string &prompt);

    /** @short Prints list of attribute definitions with indentation.
    *
    *   @param attributes Vector of attributes to print
    *   @param indentLevel Level of indentation (number of "tabs")
    */
    virtual void printAttributes(const std::vector<AttributeDefinition> &attributes, int indentLevel,
                                 std::ostream &out = std::cout);

    /** @short Prints list of attribute definitions with indentation and origin of templatized ones.
    *
    *   @param attributes Vector of pairs where first item is attribute to print and second is origin of the attribute
    *   @param indentLevel Level of indentation (number of "tabs")
    */
    virtual void printAttributesWithOrigin(
        const std::vector<std::pair<AttributeDefinition, Db::Identifier> > &attributes, int indentLevel,
        std::ostream &out = std::cout);

    /** @short Prints list of object definitions with indentation.
    *
    *   @param objects Vector of objects to print
    *   @param indentLevel Level of indentation (number of "tabs")
    */
    virtual void printObjects(const std::vector<ObjectDefinition> &objects, int indentLevel,
                              bool fullName, std::ostream &out = std::cout);

    /** @short Prints an attribute definition with indentation.
    *
    *   @param attribute Attribute to print
    *   @param indentLevel Level of indentation (number of "tabs")
    */
    virtual void printAttribute(const AttributeDefinition &attribute, int indentLevel,
                        std::ostream &out = std::cout);

    /** @short Prints an attribute definition with indentation and origin of templatized ones.
    *
    *   @param attribute Attribute to print
    *   @param origin Origin of the value if the attribute is inherited from some templete, else empty string
    *   @param indentLevel Level of indentation (number of "tabs")
    */
    virtual void printAttributeWithOrigin(const AttributeDefinition &attribute, const Db::Identifier &origin,
                                          int indentLevel, std::ostream &out = std::cout);

    /** @short Prints a object definition with indentation.
    *
    *   @param object Object to print
    *   @param indentLevel Level of indentation (number of "tabs")
    */
    virtual void printObject(const ObjectDefinition &object, int indentLevel, bool fullName,
                     std::ostream &out = std::cout);

    /** @short Prints "end" keyword with indentation.
    *
    *   @param indentLevel Level of indentation (number of "tabs")
    */
    virtual void printEnd(int indentLevel, std::ostream &out = std::cout);

    /** @short Displays list of revisions.
    *
    *   @param revisions Vector of revisions metadata to print.
    */
    virtual void printRevisions(const std::vector<Db::RevisionMetadata> &revisions);

    /** @short Prints difference using list of modifications.
    *
    *   @param modifications Vector of modifications representing the diff.
    */
    virtual void printDiff(const std::vector<Db::ObjectModificationResult> &modifications);

    /** @short Adds completion string to the completions vector in the CliCompleter.
    *
    *   @param completion Completion to add.
    */
    virtual void addCommandCompletion(const std::string &completion);

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
    *   @param tab Width of one indentation level
    *   @return String constructed from spaces for indenting
    */
    std::string indent(unsigned int indentLevel, unsigned int tab = 0);

    /** @short Construct wrapped string.
    *
    *   @param text Text to wrap.
    *   @param width Max line width.
    *   @return Vector of string wrapped to given width. Line by line.
    */
    std::vector<std::string> wrap(const std::string &text, unsigned int width);

    /** @short function for counting number of digits in an unsigned integer.
    *   
    *   @param n Number
    *   @return Number of digits
    */
    unsigned int digits(unsigned int n);

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
