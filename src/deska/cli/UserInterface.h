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
#include <tr1/memory>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>

#include "CliObjects.h"
#include "ContextStack.h"

namespace Deska
{
namespace Cli
{

class Command;
class DbInteraction;
class Parser;
class ParserException;
class UserInterfaceIOBase;
class UserInterface;
class CliConfig;


/** @short Class for communication with the user.
*
*   User interface uses class Parser for parsing lines, that does not match any keyword, class DbInteraction for
*   communication with the database and with the Parser communicates through SignalsHandler, that is actively calling
*   functions for confirmation and applying actions connected with each signal that parser emits. For  all IO
*   operations is used class UserInterfaceIO.
*/
class UserInterface: public boost::noncopyable
{
public:

    /** @short Constructor initializes stream for communication with the user and pointers for parsing
    *          input and communication with the database.
    *
    *   @param dbInteraction Pointer to the class used for communication with the database
    *   @param parser Pointer to the parser used for parsing commands that are not any known keyword
    *   @param _io Pointer to the UserInterfaceIO class for IO oparations
    *   @param _config Pointer to the CliConfig class for configuration parameters parsed from command line and config file
    */
    UserInterface(DbInteraction *dbInteraction, Parser* parser, UserInterfaceIOBase *_io, CliConfig* _config);

    /** @short Deletes commands from commands map. */
    ~UserInterface();

    //@{
    /** @short Functions for confirmation and applying actions connected with parser signals.
    *
    *   @see DbInteraction
    *   @see ParserSignals
    *   @see Parser
    */
    bool applyCreateObject(const ContextStack &context,
                           const Db::Identifier &kind, const Db::Identifier &object, ContextStackItem &newItem);
    bool applyCategoryEntered(const ContextStack &context,
                              const Db::Identifier &kind, const Db::Identifier &object, ContextStackItem &newItem);
    bool applySetAttribute(const ContextStack &context, const Db::Identifier &kind,
                           const Db::Identifier &attribute, const Db::Value &value);
    bool applySetAttributeInsert(const ContextStack &context, const Db::Identifier &kind,
                           const Db::Identifier &attribute, const Db::Identifier &value);
    bool applySetAttributeRemove(const ContextStack &context, const Db::Identifier &kind,
                           const Db::Identifier &attribute, const Db::Identifier &value);
    bool applyRemoveAttribute(const ContextStack &context, const Db::Identifier &kind, const Db::Identifier &attribute);
    bool applyObjectsFilter(const ContextStack &context, const Db::Identifier &kind, const boost::optional<Db::Filter> &filter);
    bool applyFunctionShow(const ContextStack &context);
    bool applyFunctionDelete(const ContextStack &context);
    bool applyFunctionRename(const ContextStack &context, const Db::Identifier &newName);

    bool confirmCreateObject(const ContextStack &context,
                             const Db::Identifier &kind, const Db::Identifier &object);
    bool confirmCategoryEntered(const ContextStack &context,
                                const Db::Identifier &kind, const Db::Identifier &object, bool &autoCreate);
    bool confirmSetAttribute(const ContextStack &context, const Db::Identifier &kind,
                             const Db::Identifier &attribute, const Db::Value &value);
    bool confirmSetAttributeInsert(const ContextStack &context, const Db::Identifier &kind,
                             const Db::Identifier &attribute, const Db::Identifier &value);
    bool confirmSetAttributeRemove(const ContextStack &context, const Db::Identifier &kind,
                             const Db::Identifier &attribute, const Db::Identifier &value);
    bool confirmRemoveAttribute(const ContextStack &context, const Db::Identifier &kind, const Db::Identifier &attribute);
    bool confirmObjectsFilter(const ContextStack &context, const Db::Identifier &kind, const boost::optional<Db::Filter> &filter);
    bool confirmFunctionShow(const ContextStack &context);
    bool confirmFunctionDelete(const ContextStack &context);
    bool confirmFunctionRename(const ContextStack &context, const Db::Identifier &newName);
    //@}

    /** @short Reports parse error generated by the Parser to the user (error output).
    *
    *   @param error Error to report
    *   @see ParserException
    */
    void reportParseError(const ParserException &error);

    /** @short Function for listening to users input and calling appropriate actions. */
    void run();

private:

    /** @short Recursively shows kind with attributes and nested kinds and origins.
    *
    *   @param object Object which attributes and nested kinds will be printed recursively.
    *   @param depth Depth of nesting for indentation.
    */
    void showObjectRecursive(const ObjectDefinition &object, unsigned int depth);

    friend class Start;
    friend class Resume;
    friend class Commit;
    friend class Detach;
    friend class Abort;
    friend class Rebase;
    friend class Status;
    friend class Log;
    friend class Diff;
    friend class Configdiff;
    friend class Exit;
    friend class Context;
    friend class Dump;
    friend class Restore;
    friend class Help;

    /** Pointer to the class used for communication with the database. */
    DbInteraction *m_dbInteraction;
    /** Pointer to the parser used for parsing commands that are not any known keyword. */
    Parser* m_parser;
    /** Pointer to the class for performing IO operations. */
    UserInterfaceIOBase *io;

    typedef std::map<std::string, std::tr1::shared_ptr<Command> > CommandMap;
    /** Map for commands indexed by their names. */
    CommandMap commandsMap;

    /** ID of currently connected changeset, or null when not in changeset. */
    boost::optional<Db::TemporaryChangesetId> currentChangeset;
    /** Flag singalising, that the event loop will end after this cycle. */
    bool exitLoop;
    /** Flag signalising, that parsing current line using Parser failed. */
    bool parsingFailed;
    /** Flag singalising, that all questions concerning object deletion, creation, etc. will be automaticly confirmed. */
    bool nonInteractiveMode;
    /** Flag singalising, that all questions concerning object deletion, creation, etc. will be automaticly confirmed.
    *   This flag overrides flag nonInteractiveMode. This is to be set using config file or program parameter.
    */
    bool forceNonInteractive;
};


}
}


#endif // DESKA_USER_INTERFACE_H
