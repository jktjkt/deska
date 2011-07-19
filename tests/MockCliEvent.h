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
#include "deska/db/ObjectModification.h"


#define FORWARD_0_RETURN(FUNC, EFUNC, RET_TYPE, RET_VAR) \
    static MockCliEvent FUNC(); \
    static MockCliEvent return##EFUNC(boost::call_traits<RET_TYPE>::param_type);
#define FORWARD_1(FUNC, EFUNC, TYPE_1) static MockCliEvent FUNC(boost::call_traits<TYPE_1>::param_type);
#define FORWARD_1_RETURN(FUNC, EFUNC, RET_TYPE, RET_VAR, TYPE_1) \
    static MockCliEvent FUNC(boost::call_traits<TYPE_1>::param_type); \
    static MockCliEvent return##EFUNC(boost::call_traits<RET_TYPE>::param_type);
#define FORWARD_2(FUNC, EFUNC, TYPE_1, TYPE_2) static MockCliEvent FUNC(boost::call_traits<TYPE_1>::param_type, boost::call_traits<TYPE_2>::param_type);
#define FORWARD_2_RAW_ARGS(FUNC, EFUNC, TYPE_1, TYPE_2) static MockCliEvent FUNC(TYPE_1, TYPE_2);
#define FORWARD_3(FUNC, EFUNC, TYPE_1, TYPE_2, TYPE_3) static MockCliEvent FUNC(boost::call_traits<TYPE_1>::param_type, boost::call_traits<TYPE_2>::param_type, boost::call_traits<TYPE_3>::param_type);
#define FORWARD_3_OSTREAM(FUNC, EFUNC, TYPE_1, TYPE_2) static MockCliEvent FUNC(boost::call_traits<TYPE_1>::param_type, boost::call_traits<TYPE_2>::param_type, std::ostream&);
#define FORWARD_4_OSTREAM(FUNC, EFUNC, TYPE_1, TYPE_2, TYPE_3) static MockCliEvent FUNC(boost::call_traits<TYPE_1>::param_type, boost::call_traits<TYPE_2>::param_type, boost::call_traits<TYPE_3>::param_type, std::ostream&);

/** @short Helper struct representing a signal emitted by the Parser being tested */
struct MockCliEvent
{
#include "CliFunctionDefinitions.h"
    
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
        /** @short The printAttributesWithOrigin() event */
        EVENT_PRINT_ATTRIBUTES_WITH_ORIGIN,
        /** @short The printAttributeWithOrigin() event */
        EVENT_PRINT_ATTRIBUTE_WITH_ORIGIN,
        /** @short The printObjects() event */
        EVENT_PRINT_OBJECTS,
        /** @short The printObject() event */
        EVENT_PRINT_OBJECT,
        /** @short The printEnd() event */
        EVENT_PRINT_END,
        /** @short The printRevisions() event */
        EVENT_PRINT_REVISIONS,
        /** @short The printDiff() event */
        EVENT_PRINT_DIFF,
        /** @short The addCommandCompletion() event */
        EVENT_ADD_COMMAND_COMPLETION,
        /** @short Fake, invalid event */
        EVENT_INVALID
    } Event;

    MockCliEvent(Event e);

    Event eventKind;
    std::string str1;
    std::string str2;
    Deska::Db::Identifier ident;
    int integer;
    bool boolean;
    boost::optional<Deska::Db::ObjectDefinition> object;
    boost::optional<Deska::Db::AttributeDefinition> attr;
    std::map<std::string, std::string> map1;
    std::map<std::string, std::string> map2;
    std::vector<std::pair<std::string, std::string> > vectpair;
    std::vector<std::string> vect;
    std::vector<Deska::Db::PendingChangeset> changesets;
    std::vector<Deska::Db::RevisionMetadata> revisions;
    std::vector<Deska::Db::ObjectModification> modifications;
    std::vector<Deska::Db::AttributeDefinition> attrs;
    std::vector<std::pair<Deska::Db::AttributeDefinition, Deska::Db::Identifier> > attrsorig;
    std::vector<Deska::Db::ObjectDefinition> objects;
};


std::ostream& operator<<(std::ostream &out, const MockCliEvent &m);


#endif // DESKA_TEST_MOCKCLIEVENT_H
