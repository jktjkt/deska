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


#ifndef DESKA_TEST_MOCKCLIEVENT_H
#define DESKA_TEST_MOCKCLIEVENT_H

#include <iosfwd>
#include <map>
#include <vector>
#include <string>
#include "deska/db/Objects.h"
#include "deska/db/Revisions.h"



/** @short Helper struct representing a signal emitted by the Parser being tested */
struct MockCliEvent
{
    /** @short Handler for the reportError() IO function */
    static MockCliEvent reportError(const std::string &errorMessage);

    /** @short Handler for the printMessage() IO function */
    static MockCliEvent printMessage(const std::string &message);

    /** @short Handler for the confirmDeletion() IO function */
    static MockCliEvent confirmDeletion(const Deska::Db::ObjectDefinition &object);
    
    /** @short Handler for return of the confirmDeletion() IO function */
    static MockCliEvent returnConfirmDeletion(bool confirm);
    
    /** @short Handler for the confirmCreation() IO function */
    static MockCliEvent confirmCreation(const Deska::Db::ObjectDefinition &object);
    
    /** @short Handler for return of the confirmCreation() IO function */
    static MockCliEvent returnConfirmCreation(bool confirm);

    /** @short Handler for the confirmRestoration() IO function */
    static MockCliEvent confirmRestoration(const Deska::Db::ObjectDefinition &object);
    
    /** @short Handler for return of the confirmRestoration() IO function */
    static MockCliEvent returnConfirmRestoration(bool confirm);
    
    /** @short Handler for the askForCommitMessage() IO function */
    static MockCliEvent askForCommitMessage();
    
    /** @short Handler for return of the askForCommitMessage() IO function */
    static MockCliEvent returnAskForCommitMessage(const std::string &message);

    /** @short Handler for the askForDetachMessage() IO function */
    static MockCliEvent askForDetachMessage();
    
    /** @short Handler for return of the askForDetachMessage() IO function */
    static MockCliEvent returnAskForDetachMessage(const std::string &message);

    /** @short Handler for the printHelp() IO function */
    static MockCliEvent printHelp(const std::map<std::string, std::string> &cliCommands,
                                       const std::map<std::string, std::string> &parserKeywords);
    
    /** @short Handler for the printHelpCommand() IO function */
    static MockCliEvent printHelpCommand(const std::string &cmdName, const std::string &cmdDscr);
    
    /** @short Handler for the printHelpKeyword() IO function */
    static MockCliEvent printHelpKeyword(const std::string &keywordName, const std::string &keywordDscr);
    
    /** @short Handler for the printHelpKind() IO function */
    static MockCliEvent printHelpKind(const std::string &kindName,
                                      const std::vector<std::pair<std::string, std::string> > &kindAttrs,
                                      const std::vector<std::string> &nestedKinds);
    
    /** @short Handler for the printHelpShowKinds() IO function */
    static MockCliEvent printHelpShowKinds(const std::vector<std::string> &kinds);
    
    /** @short Handler for the chooseChangeset() IO function */
    static MockCliEvent chooseChangeset(const std::vector<Deska::Db::PendingChangeset> &pendingChangesets);
    
    /** @short Handler for return of the chooseChangeset() IO function */
    static MockCliEvent returnChooseChangeset(int changeset);
    
    /** @short Handler for the readLine() IO function */
    static MockCliEvent readLine(const std::string &prompt);
    
    /** @short Handler for return of the readLine() IO function */
    static MockCliEvent returnReadLine(const std::string &line);
    
    /** @short Handler for the printAttributes() IO function */
    static MockCliEvent printAttributes(const std::vector<Deska::Db::AttributeDefinition> &attributes, int indentLevel,
                                        std::ostream &out = std::cout);
    
    /** @short Handler for the printAttribute() IO function */
    static MockCliEvent printAttribute(const Deska::Db::AttributeDefinition &attribute, int indentLevel,
                                       std::ostream &out = std::cout);
                                       
    /** @short Handler for the printObjects() IO function */
    static MockCliEvent printObjects(const std::vector<Deska::Db::ObjectDefinition> &objects, int indentLevel,
                                     bool fullName, std::ostream &out = std::cout);
    
    /** @short Handler for the printObject() IO function */
    static MockCliEvent printObject(const Deska::Db::ObjectDefinition &object, int indentLevel, bool fullName,
                                    std::ostream &out = std::cout);
    
    /** @short Handler for the printEnd() IO function */
    static MockCliEvent printEnd(int indentLevel, std::ostream &out = std::cout);
    
    /** @short Handler for the addCommandCompletion() IO function */
    static MockCliEvent addCommandCompletion(const std::string &completion);
    
    /** @short An empty event for debug printing */
    static MockCliEvent invalid();

    bool operator==(const MockCliEvent &other) const;
    
    bool inputEvent(const MockCliEvent &m) const;

    bool outputEvent(const MockCliEvent &m) const;
    
    bool myReturn(const MockCliEvent &other) const;

    friend std::ostream& operator<<(std::ostream &out, const MockCliEvent &m);


    typedef enum {
        /** @short The reportError() event */
        EVENT_REPORT_ERROR,
        /** @short The printMessage() event */
        EVENT_PRINT_MESSAGE,
        /** @short The confirmDeletion() event */
        EVENT_CONFIRM_DELETION,
        /** @short The confirmDeletion() return */
        RETURN_CONFIRM_DELETION,
        /** @short The confirmCreation() event */
        EVENT_CONFIRM_CREATION,
        /** @short The confirmCreation() return */
        RETURN_CONFIRM_CREATION,
        /** @short The confirmRestoration() event */
        EVENT_CONFIRM_RESTORATION,
        /** @short The confirmRestoration() return */
        RETURN_CONFIRM_RESTORATION,
        /** @short The askForCommitMessage() event */
        EVENT_ASK_FOR_COMMIT_MESSAGE,
        /** @short The askForCommitMessage() return */
        RETURN_ASK_FOR_COMMIT_MESSAGE,
        /** @short The askForDetachMessage() event */
        EVENT_ASK_FOR_DETACH_MESSAGE,
        /** @short The askForDetachMessage() return */
        RETURN_ASK_FOR_DETACH_MESSAGE,
        /** @short The printHelp() event */
        EVENT_PRINT_HELP,
        /** @short The printHelpCommand() event */
        EVENT_PRINT_HELP_COMMAND,
        /** @short The printHelpKeyword() event */
        EVENT_PRINT_HELP_KEYWORD,
        /** @short The printHelpKind() event */
        EVENT_PRINT_HELP_KIND,
        /** @short The printHelpShowKinds() event */
        EVENT_PRINT_HELP_SHOW_KINDS,
        /** @short The chooseChangeset() event */
        EVENT_CHOOSE_CHANGESET,
        /** @short The chooseChangeset() return */
        RETURN_CHOOSE_CHANGESET,
        /** @short The readLine() event */
        EVENT_READ_LINE,
        /** @short The readLine() return */
        RETURN_READ_LINE,
        /** @short The printAttributes() event */
        EVENT_PRINT_ATTRIBUTES,
        /** @short The printAttribute() event */
        EVENT_PRINT_ATTRIBUTE,
        /** @short The printObjects() event */
        EVENT_PRINT_OBJECTS,
        /** @short The printObject() event */
        EVENT_PRINT_OBJECT,
        /** @short The printEnd() event */
        EVENT_PRINT_END,
        /** @short The addCommandCompletion() event */
        EVENT_ADD_COMMAND_COMPLETION,
        /** @short Fake, invalid event */
        EVENT_INVALID
    } Event;

    MockCliEvent(Event e);

    Event eventKind;
    std::string str1;
    std::string str2;
    int integer;
    bool boolean;
    Deska::Db::ObjectDefinition object;
    Deska::Db::AttributeDefinition attr;
    std::map<std::string, std::string> map1;
    std::map<std::string, std::string> map2;
    std::vector<std::pair<std::string, std::string> > vectpair;
    std::vector<std::string> vect;
    std::vector<Deska::Db::PendingChangeset> changesets;
    std::vector<Deska::Db::AttributeDefinition> attrs;
    std::vector<Deska::Db::ObjectDefinition> objects;
};


std::ostream& operator<<(std::ostream &out, const MockCliEvent &m);


#endif // DESKA_TEST_MOCKCLIEVENT_H
