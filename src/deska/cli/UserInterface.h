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
#include <tr1/memory>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/noncopyable.hpp>

namespace Deska
{
namespace Cli
{

class DbInteraction;
class Parser;
class ParserException;
class UserInterfaceIO;
class UserInterface;



/** @short Abstract class for each command.
*
*   These classes are stored in the map in the UserInterface and called based on user input.
*/
class Command
{
public:
    /** Sets pointer to the UserInterface for performing actions.
    *
    *   @param userInterface Pointer to the UserInterface
    */
    Command(UserInterface *userInterface);

    virtual ~Command();

    /** @short This function is invoken when right command name is parsed.
    *
    *   @param params Parameters of the command.
    */
    virtual void operator()(const std::string &params) = 0;

    /** @short Gets command completion patterns.
    *
    *   @return Vector with command completion patterns.
    */
    virtual std::vector<std::string> completionPatterns();
    
    /** @short Gets command name.
    *
    *   @return Command name.
    */
    virtual std::string name();

    /** @short Gets command usage description.
    *
    *   @return Command usage.
    */
    virtual std::string usage();

protected:
    /** Patterns for tab completition purposes. */
    std::vector<std::string> complPatterns;
    /** Name of the command. Command will be invoked typinh this stirng into the CLI. */
    std::string cmdName;
    /** Description of usage of the command. */
    std::string cmdUsage;
    /** Pointer to the UserInterface for performing actions. */
    UserInterface *ui;
};



/** @short Cli command.
*
*   Starts new changeset.
*
*   @see Command
*/
class Start: public Command
{
public:
    /** @short Constructor sets command name and completion pattern.
    *
    *   @param userInterface Pointer to the UserInterface
    */
    Start(UserInterface *userInterface);

    virtual ~Start();

    /** @short Starts new changeset.
    *
    *   @param params Unused here.
    */
    virtual void operator()(const std::string &params);
};



/** @short Cli command.
*
*   Displays list of pending changesets with ability to connect to one.
*
*   @see Command
*/
class Resume: public Command
{
public:
    /** @short Constructor sets command name and completion pattern.
    *
    *   @param userInterface Pointer to the UserInterface
    */
    Resume(UserInterface *userInterface);

    virtual ~Resume();

    /** @short Displays list of pending changesets with ability to connect to one.
    *
    *   @param params Unused here.
    */
    virtual void operator()(const std::string &params);
};



/** @short Cli command.
*
*   Displays promt for commit message and commits current changeset.
*
*   @see Command
*/
class Commit: public Command
{
public:
    /** @short Constructor sets command name and completion pattern.
    *
    *   @param userInterface Pointer to the UserInterface
    */
    Commit(UserInterface *userInterface);

    virtual ~Commit();

    /** @short Displays promt for commit message and commits current changeset.
    *
    *   @param params Commit message. Will be prompted, when omitted.
    */
    virtual void operator()(const std::string &params);
};



/** @short Cli command.
*
*   Displays promt for detach message and detaches from current changeset.
*
*   @see Command
*/
class Detach: public Command
{
public:
    /** @short Constructor sets command name and completion pattern.
    *
    *   @param userInterface Pointer to the UserInterface
    */
    Detach(UserInterface *userInterface);

    virtual ~Detach();

    /** @short Displays promt for detach message and detaches from current changeset.
    *
    *   @param params Detach message. Will be prompted, when omitted.
    */
    virtual void operator()(const std::string &params);
};



/** @short Cli command.
*
*   Aborts current changeset.
*
*   @see Command
*/
class Abort: public Command
{
public:
    /** @short Constructor sets command name and completion pattern.
    *
    *   @param userInterface Pointer to the UserInterface
    */
    Abort(UserInterface *userInterface);

    virtual ~Abort();

    /** @short Aborts current changeset.
    *
    *   @param params Unused here.
    */
    virtual void operator()(const std::string &params);
};



/** @short Cli command.
*
*   Shows if you are connected to any changeset or not.
*
*   @see Command
*/
class Status: public Command
{
public:
    /** @short Constructor sets command name and completion pattern.
    *
    *   @param userInterface Pointer to the UserInterface
    */
    Status(UserInterface *userInterface);

    virtual ~Status();

    /** @short Shows if you are connected to any changeset or not.
    *
    *   @param params Unused here.
    */
    virtual void operator()(const std::string &params);
};



/** @short Cli command.
*
*   Exits the application.
*
*   @see Command
*/
class Exit: public Command
{
public:
    /** @short Constructor sets command name and completion pattern.
    *
    *   @param userInterface Pointer to the UserInterface
    */
    Exit(UserInterface *userInterface);

    virtual ~Exit();

    /** @short Exits the application.
    *
    *   @param params Unused here.
    */
    virtual void operator()(const std::string &params);
};



/** @short Cli command.
*
*   Dumps DB contents.
*
*   @see Command
*/
class Dump: public Command
{
public:
    /** @short Constructor sets command name and completion pattern.
    *
    *   @param userInterface Pointer to the UserInterface
    */
    Dump(UserInterface *userInterface);

    virtual ~Dump();

    /** @short Dumps DB contents.
    *
    *   @param params File name where to dump the DB. Dump to standard output when ommited.
    */
    virtual void operator()(const std::string &params);
};



/** @short Cli command.
*
*   Displays this list of commands with usages.
*
*   @see Command
*/
class Help: public Command
{
public:
    /** @short Constructor sets command name and completion pattern.
    *
    *   @param userInterface Pointer to the UserInterface
    */
    Help(UserInterface *userInterface);

    virtual ~Help();

    /** @short Displays this list of commands with usages.
    *
    *   @param params Unused here.
    */
    virtual void operator()(const std::string &params);
};



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
    */
    UserInterface(DbInteraction *dbInteraction, Parser* parser, UserInterfaceIO *_io);

    /** @short Deletes commands from commands map. */
    ~UserInterface();

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
    void applyRemoveAttribute(const Db::ContextStack &context, const Db::Identifier &attribute);
    void applyFunctionShow(const Db::ContextStack &context);
    void applyFunctionDelete(const Db::ContextStack &context);
    void applyFunctionRename(const Db::ContextStack &context, const Db::Identifier &newName);

    bool confirmCategoryEntered(const Db::ContextStack &context,
                                const Db::Identifier &kind, const Db::Identifier &object);
    bool confirmSetAttribute(const Db::ContextStack &context,
                             const Db::Identifier &attribute, const Db::Value &value);
    bool confirmRemoveAttribute(const Db::ContextStack &context, const Db::Identifier &attribute);
    bool confirmFunctionShow(const Db::ContextStack &context);
    bool confirmFunctionDelete(const Db::ContextStack &context);
    bool confirmFunctionRename(const Db::ContextStack &context, const Db::Identifier &newName);
    //@}

    /** @short Reports any error to the user (error output).
    *
    *   @param errorMessage Error message to report
    */
    void reportError(const std::string &errorMessage);

    /** @short Function for listening to users input and calling appropriate actions. */
    void run();

private:

    friend class Start;
    friend class Resume;
    friend class Commit;
    friend class Detach;
    friend class Abort;
    friend class Status;
    friend class Exit;
    friend class Dump;
    friend class Help;

    /** Pointer to the class used for communication with the database. */
    DbInteraction *m_dbInteraction;
    /** Pointer to the parser used for parsing commands that are not any known keyword. */
    Parser* m_parser;
    /** Pointer to the class for performing IO operations. */
    UserInterfaceIO *io;

    typedef std::map<std::string, std::tr1::shared_ptr<Command> > CommandMap;
    /** Map for commands indexed by their names. */
    CommandMap commandsMap;

    bool inChangeset;
    bool exitLoop;
};


}
}


#endif // DESKA_USER_INTERFACE_H
