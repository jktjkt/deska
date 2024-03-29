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

#ifndef DESKA_CLI_CLICOMMANDS_H
#define DESKA_CLI_CLICOMMANDS_H

#include <string>
#include <vector>

#include "CliObjects.h"
#include "deska/db/Revisions.h"
#include "deska/db/ObjectModification.h"


namespace Deska {
namespace Cli {

std::string readableAttrPrinter(const std::string &prefixMessage, const Db::Value &v);

class UserInterface;
class DbInteraction;


/** @short Type of object modification for sorting purposes */
typedef enum {
    /** @short Db::DeleteObjectModification */
    OBJECT_MODIFICATION_TYPE_DELETE = 0,
    /** @short Db::RenameObjectModification */
    OBJECT_MODIFICATION_TYPE_RENAME = 1,
    /** @short Db::CreateObjectModification */
    OBJECT_MODIFICATION_TYPE_CREATE = 2,
    /** @short Db::SetAttributeModification */
    OBJECT_MODIFICATION_TYPE_SETATTR = 3
} ModificationType;



/** @short Visitor for printing object modifications. */
struct ModificationBackuper: public boost::static_visitor<std::string>
{
    //@{
    /** @short Function for converting single object modification to string for purposes of backup.
    *
    *   @param modification Instance of modifications from Db::ObjectModification variant.
    *   @return Parser readable string representation of the modification.
    */
    std::string operator()(const Db::CreateObjectModification &modification) const;
    std::string operator()(const Db::DeleteObjectModification &modification) const;
    std::string operator()(const Db::RenameObjectModification &modification) const;
    std::string operator()(const Db::SetAttributeModification &modification) const;
    //@}
};



/** @short Visitor for obtaining type of each object modification */
struct ModificationTypeGetter: public boost::static_visitor<ModificationType>
{
    //@{
    /** @short Function for obtaining ModificationType for each modification
    *
    *   @param modification Instance of modifications from Db::ObjectModification variant.
    *   @return Type of the modification
    */
    ModificationType operator()(const Db::CreateObjectModification &modification) const;
    ModificationType operator()(const Db::DeleteObjectModification &modification) const;
    ModificationType operator()(const Db::RenameObjectModification &modification) const;
    ModificationType operator()(const Db::SetAttributeModification &modification) const;
    //@}
};



/** @short Visitor for checking if the modification should be backed up or not
*
*   For example read-only attributes should not be backed up as they are assigned automaticaly by the DB.
*/
class ModificationBackupChecker: public boost::static_visitor<bool>
{
public:
    /** @short Constructor assignes DbInteraction to the checker so it can check if some attributes are read-only or not
    *
    *   @param m_dbInteraction Pointer to the DbInteraction to allow communication with the DB
    */
    ModificationBackupChecker(DbInteraction *dbInteraction);
    //@{
    /** @short Function for obtaining ModificationType for each modification
    *
    *   @param modification Instance of modifications from Db::ObjectModification variant.
    *   @return Type of the modification
    */
    bool operator()(const Db::CreateObjectModification &modification) const;
    bool operator()(const Db::DeleteObjectModification &modification) const;
    bool operator()(const Db::RenameObjectModification &modification) const;
    bool operator()(const Db::SetAttributeModification &modification) const;
    //@}
    
private:
    /** Pointer to the DbInteraction to allow communication with the DB */
    DbInteraction *m_dbInteraction;
};



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
    virtual bool operator()(const std::string &params) = 0;

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

    /** @short Splits string with parameters divided by whitespace into a vector.
    *
    *   @param params String with parameters divided using spaces od tabs.
    *   @return Vector of single parameters.
    */
    std::vector<std::string> extractParams(const std::string &params);

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
    virtual bool operator()(const std::string &params);
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
    virtual bool operator()(const std::string &params);
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
    virtual bool operator()(const std::string &params);
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
    virtual bool operator()(const std::string &params);
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
    virtual bool operator()(const std::string &params);
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
    virtual bool operator()(const std::string &params);
};



/** @short Cli command.
*
*   Toggles non-interactive mode
*
*   @see Command
*/
class NonInteractive: public Command
{
public:
    /** @short Constructor sets command name and completion pattern.
    *
    *   @param userInterface Pointer to the UserInterface
    */
    NonInteractive(UserInterface *userInterface);

    virtual ~NonInteractive();

    /** @short Shows if you are in non-interactive mode or not and is able to switch it
    *
    *   @param params With param "on" switches to non-interactive mode, with param "off" turns
    *                 non-interactive mode off. Without parameter shows if you are in non-interactive mode or not
    */
    virtual bool operator()(const std::string &params);
};



/** @short Cli command.
*
*   Command for showing diff of output from configuration generators.
*
*   @see Command
*/
class Configdiff: public Command
{
public:
    /** @short Constructor sets command name and completion pattern.
    *
    *   @param userInterface Pointer to the UserInterface
    */
    Configdiff(UserInterface *userInterface);

    virtual ~Configdiff();

    /** @short Function for showing diff of output from configuration generators.
    *
    *   @param params With parameter "regenerate" forces regeneration of the configuration.
    */
    virtual bool operator()(const std::string &params);
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
    virtual bool operator()(const std::string &params);
};



/** @short Cli command.
*
*   Shows current context.
*
*   @see Command
*/
class Context: public Command
{
public:
    /** @short Constructor sets command name and completion pattern.
    *
    *   @param userInterface Pointer to the UserInterface
    */
    Context(UserInterface *userInterface);

    virtual ~Context();

    /** @short Shows current context.
    *
    *   @param params For parameter "objects" shows list of objects matched by current context.
    */
    virtual bool operator()(const std::string &params);
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
    virtual bool operator()(const std::string &params);

private:
    /** @short Recursively dumps kind with attributes and nested kinds.
    *
    *   @param object Object which attributes and nested kinds will be printed recursively.
    *   @param depth Depth of nesting for indentation.
    *   @param out Output file stream where to dump objects.
    */
    void dumpObjectRecursive(const ObjectDefinition &object, unsigned int depth, std::ostream &out = std::cout);
};



/** @short Cli command.
*
*   Runs parser commands from file.
*
*   @see Command
*/
class Batch: public Command
{
public:
    /** @short Constructor sets command name and completion pattern.
    *
    *   @param userInterface Pointer to the UserInterface
    */
    Batch(UserInterface *userInterface);

    virtual ~Batch();

    /** @short Runs parser commands from file.
    *
    *   @param params File name where commands are stored.
    */
    virtual bool operator()(const std::string &params);
};



/** @short Cli command.
*
*   Backs up the whole DB to a file.
*
*   @see Command
*/
class Backup: public Command
{
public:
    /** @short Constructor sets command name and completion pattern.
    *
    *   @param userInterface Pointer to the UserInterface
    */
    Backup(UserInterface *userInterface);

    virtual ~Backup();

    /** @short Backs up the whole DB to a file.
    *
    *   @param params File name where the backup will be stored.
    */
    virtual bool operator()(const std::string &params);

private:
    /** @short Function for sorting object modifications.
    *
    *   Sorting by modification type.
    *
    *   @param a First object modification
    *   @param b Second object modification
    *   @return True if b is greater than a, else false
    */
    static bool objectModificationResultLess(const Db::ObjectModificationResult &a, const Db::ObjectModificationResult &b);
};



/** @short Cli command.
*
*   Restores whole DB from file.
*
*   @see Command
*/
class Restore: public Command
{
public:
    /** @short Constructor sets command name and completion pattern.
    *
    *   @param userInterface Pointer to the UserInterface
    */
    Restore(UserInterface *userInterface);

    virtual ~Restore();

    /** @short Restores whole DB from file.
    *
    *   @param params File name where the backup is stored.
    */
    virtual bool operator()(const std::string &params);
};



/** @short Cli command.
*
*   Runs commands from file including CLI commands.
*
*   @see Command
*/
class Execute: public Command
{
public:
    /** @short Constructor sets command name and completion pattern.
    *
    *   @param userInterface Pointer to the UserInterface
    */
    Execute(UserInterface *userInterface);

    virtual ~Execute();

    /** @short Runs commands from file including CLI commands.
    *
    *   @param params File name where commands are stored.
    */
    virtual bool operator()(const std::string &params);
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
    virtual bool operator()(const std::string &params);
};


}
}

#endif // DESKA_CLI_CLICOMMANDS_H
