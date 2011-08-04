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
#include "PredefinedRules.h"
#include "ParserIterator.h"
#include "deska/db/Revisions.h"
#include "deska/db/Filter.h"


namespace Deska {
namespace Cli {

class UserInterface;



template <typename Iterator>
class LogFilterParser: public qi::grammar<Iterator, Db::Filter(), ascii::space_type>
{

public:

    /** @short Constructor only initializes the grammar with empty symbols table.
    *
    *   @param kindName Name of top-level object type, to which the attributes belong.
    *   @param ownAttrsExpressionsParser Parser for parsing expressions containing own attributes, not nested ones.
    *   @param parent Pointer to main parser for error reporting purposes.
    */
    LogFilterParser();

    ~LogFilterParser();

    /** @short Function used for filling of symbols table of the parser.
    *
    *   @param nestedKindName Name of nested kind.
    *   @param expressionsParser Filter expressions parser for the nested kind.
    */
    void addKind(const Db::Identifier &kindName);

private:

    /** Symbols table with comparison operators. */
    qi::symbols<char, Db::ComparisonOperator> operators;
    /** Kind name - object name parser pairs for purposes of Nabialek trick. */
    qi::symbols<char, qi::rule<Iterator, Db::Identifier(), ascii::space_type> > kinds;
    /** Metadata name - metadata value parser pairs for purposes of Nabialek trick. */
    qi::symbols<char, qi::rule<Iterator, Db::MetadataValue(), ascii::space_type> > metadatas;

    /** Main rule. */
    qi::rule<Iterator, Db::Filter(), ascii::space_type> start;

    qi::rule<Iterator, Db::Filter(), ascii::space_type> expr;

    qi::rule<Iterator, Db::Filter(), ascii::space_type> andFilter;
    qi::rule<Iterator, Db::Filter(), ascii::space_type> orFilter;

    qi::rule<Iterator, Db::Filter(), ascii::space_type, qi::locals<bool> > kindExpr;
    qi::rule<Iterator, Db::Filter(), ascii::space_type, qi::locals<bool> > metadataExpr;

    /** Rule for parsing expressions for kinds. */
    qi::rule<Iterator, Db::Filter(), ascii::space_type,
             qi::locals<qi::rule<Iterator, Db::Identifier(), ascii::space_type>,
                        Db::ComparisonOperator > > kindDispatch;
    /** Rule for parsing expressions for metadata. */
    qi::rule<Iterator, Db::Filter(), ascii::space_type,
             qi::locals<qi::rule<Iterator, Db::MetadataValue(), ascii::space_type>,
                        Db::ComparisonOperator > > metadataDispatch;

    PredefinedRules<Iterator> *predefinedRules;

    Db::Identifier currentKindName;
    Db::Identifier currentMetadataName;
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
*   Rebases current changeset.
*
*   @see Command
*/
class Rebase: public Command
{
public:
    /** @short Constructor sets command name and completion pattern.
    *
    *   @param userInterface Pointer to the UserInterface
    */
    Rebase(UserInterface *userInterface);

    virtual ~Rebase();

    /** @short Rebases current changeset.
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
*   Command for operations with revisions and history.
*
*   @see Command
*/
class Log: public Command
{
public:
    /** @short Constructor sets command name and completion pattern.
    *
    *   @param userInterface Pointer to the UserInterface
    */
    Log(UserInterface *userInterface);

    virtual ~Log();

    /** @short Function for operations with revisions and history.
    *
    *   @param params Unused here.
    */
    virtual void operator()(const std::string &params);

private:
    /** Parser of parameters */
    LogFilterParser<iterator_type> *filterParser;
};



/** @short Cli command.
*
*   Command for showing differences between revisions.
*
*   @see Command
*/
class Diff: public Command
{
public:
    /** @short Constructor sets command name and completion pattern.
    *
    *   @param userInterface Pointer to the UserInterface
    */
    Diff(UserInterface *userInterface);

    virtual ~Diff();

    /** @short Function for showing diffs between revisions.
    *
    *   @param params Unused here.
    */
    virtual void operator()(const std::string &params);

private:

    /** Converts string to Db::RevisionId.
    *
    *   @param rev String in format r123.
    *   @return Revision id.
    */
    Db::RevisionId stringToRevision(const std::string &rev);
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
*   Runs commands from file.
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

    /** @short Restores DB from a dump.
    *
    *   @param params File name where the dump is stored.
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


}
}

#endif // DESKA_CLI_CLICOMMANDS_H