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

#include <sstream>
#include <fstream>
#include <boost/foreach.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/tokenizer.hpp>
#include <boost/regex.hpp>

#include "DbInteraction.h"
#include "Exceptions.h"
#include "Parser.h"
#include "UserInterface.h"
#include "UserInterfaceIOBase.h"
#include "deska/db/JsonApi.h"



namespace Deska
{
namespace Cli
{



Command::Command(UserInterface *userInterface): ui(userInterface)
{
}



Command::~Command()
{
}



std::vector<std::string> Command::completionPatterns()
{
    return complPatterns;
}



std::string Command::name()
{
    return cmdName;
}



std::string Command::usage()
{
    return cmdUsage;
}



std::vector<std::string> Command::extractParams(const std::string &params)
{
    std::vector<std::string> paramsList;
    boost::char_separator<char> separators(" \t");
    boost::tokenizer<boost::char_separator<char> > tokenizer(params, separators);
    std::string token;
    
    for (boost::tokenizer<boost::char_separator<char> >::const_iterator it = tokenizer.begin();
         it != tokenizer.end(); ++it) {
        token = *it;
        boost::algorithm::trim(token);
        paramsList.push_back(token);
    }

    return paramsList;
}



Start::Start(UserInterface *userInterface): Command(userInterface)
{
    cmdName = "start";
    cmdUsage = "Starts new changeset.";
    complPatterns.push_back("start");
}



Start::~Start()
{
}



void Start::operator()(const std::string &params)
{
    if (!params.empty()) {
        ui->io->reportError("Error: No parameters expected for command " + cmdName + ".");
        return;
    }
    if (ui->currentChangeset) {
        std::ostringstream ostr;
        ostr << "Error: You are already in the changeset " << *(ui->currentChangeset) << "!";
        ui->io->reportError(ostr.str());
        return;
    }
    ui->currentChangeset = ui->m_dbInteraction->createNewChangeset();
    std::ostringstream ostr;
    ostr << "Changeset " << *(ui->currentChangeset) << " started.";
    ui->io->printMessage(ostr.str());
}



Resume::Resume(UserInterface *userInterface): Command(userInterface)
{
    cmdName = "resume";
    cmdUsage = "Displays list of pending changesets with ability to connect to one.";
    complPatterns.push_back("resume");
}



Resume::~Resume()
{
}



void Resume::operator()(const std::string &params)
{
    if (!params.empty()) {
        ui->io->reportError("Error: No parameters expected for command " + cmdName + ".");
        return;
    }
    if (ui->currentChangeset) {
        std::ostringstream ostr;
        ostr << "Error: You are already in the changeset " << *(ui->currentChangeset) << "!";
        ui->io->reportError(ostr.str());
        return;
    }
    try {
        // Print list of pending changesets, so user can choose one
        std::vector<Db::PendingChangeset> pendingChangesets = ui->m_dbInteraction->allPendingChangesets();
        int choice = ui->io->chooseChangeset(pendingChangesets);

        if (choice >= 0) {
            // Some changeset was choosen
            ui->m_dbInteraction->resumeChangeset(pendingChangesets[choice].revision);
            ui->currentChangeset = pendingChangesets[choice].revision;
            std::ostringstream ostr;
            ostr << "Changeset " << *(ui->currentChangeset) << " resumed.";
            ui->io->printMessage(ostr.str());
        }
        
    } catch (Deska::Db::NotFoundError &e) {
        ui->io->reportError("Server reports an error:\nObject not found:\n\n" + e.whatWithBacktrace() + "\n");
    } catch (Deska::Db::NoChangesetError &e) {
        ui->io->reportError("Server reports an error:\nYou aren't associated to a changeset:\n\n" + e.whatWithBacktrace() + "\n");
    } catch (Deska::Db::ChangesetAlreadyOpenError &e) {
        ui->io->reportError("Server reports an error:\nChangeset is already open:\n\n" + e.whatWithBacktrace() + "\n");
    } catch (Deska::Db::SqlError &e) {
        ui->io->reportError("Server reports an error:\nError in executing an SQL statement:\n\n" + e.whatWithBacktrace() + "\n");
    } catch (Deska::Db::ServerError &e) {
        ui->io->reportError("Server reports an error:\nInternal server error:\n\n" + e.whatWithBacktrace() + "\n");
    } catch (Deska::Db::JsonSyntaxError &e) {
        ui->io->reportError("Cannot parse JSON data.\n " + e.whatWithBacktrace() + "\n");
    } catch (Deska::Db::JsonStructureError &e) {
        ui->io->reportError("Received malformed JSON data:\n " + e.whatWithBacktrace() + "\n");
    }
}



Commit::Commit(UserInterface *userInterface): Command(userInterface)
{
    cmdName = "commit";
    cmdUsage = "Displays promt for commit message and commits current changeset.";
    complPatterns.push_back("commit");
}



Commit::~Commit()
{
}



void Commit::operator()(const std::string &params)
{
    if (!(ui->currentChangeset)) {
        ui->io->reportError("Error: You are not in any changeset!");
        return;
    }
    std::string commitMessage;
    if (!params.empty()) {
        commitMessage = params;
    } else {
        commitMessage = ui->io->askForCommitMessage();
    }
    try {
        ui->m_dbInteraction->commitChangeset(commitMessage);
        std::ostringstream ostr;
        ostr << "Changeset " << *(ui->currentChangeset) << " commited.";
        ui->io->printMessage(ostr.str());
        ui->currentChangeset = boost::optional<Db::TemporaryChangesetId>();
        ui->m_parser->clearContextStack();
    } catch (Deska::Db::RemoteDbError &e) {
        // FIXME: quick & durty "fix" for the demo
        std::ostringstream ss;
        ss << "Error: commit failed: " << e.what();
        ui->io->reportError(ss.str());
    }
}



Detach::Detach(UserInterface *userInterface): Command(userInterface)
{
    cmdName = "detach";
    cmdUsage = "Displays promt for detach message and detaches from current changeset.";
    complPatterns.push_back("detach");
}



Detach::~Detach()
{
}



void Detach::operator()(const std::string &params)
{
    if (!(ui->currentChangeset)) {
        ui->io->reportError("Error: You are not in any changeset!");
        return;
    }
    std::string detachMessage;
    if (!params.empty()) {
        detachMessage = params;
    } else {
        detachMessage = ui->io->askForDetachMessage();
    }
    ui->m_dbInteraction->detachFromChangeset(detachMessage);
    std::ostringstream ostr;
    ostr << "Changeset " << *(ui->currentChangeset) << " detached.";
    ui->io->printMessage(ostr.str());
    ui->currentChangeset = boost::optional<Db::TemporaryChangesetId>();
    ui->m_parser->clearContextStack();
}



Abort::Abort(UserInterface *userInterface): Command(userInterface)
{
    cmdName = "abort";
    cmdUsage = "Aborts current changeset.";
    complPatterns.push_back("abort");
}



Abort::~Abort()
{
}



void Abort::operator()(const std::string &params)
{
    if (!params.empty()) {
        ui->io->reportError("Error: No parameters expected for command " + cmdName + ".");
        return;
    }
    if (!(ui->currentChangeset)) {
        ui->io->reportError("Error: You are not in any changeset!");
        return;
    }
    ui->m_dbInteraction->abortChangeset();
    std::ostringstream ostr;
    ostr << "Changeset " << *(ui->currentChangeset) << " aborted.";
    ui->io->printMessage(ostr.str());
    ui->currentChangeset = boost::optional<Db::TemporaryChangesetId>();
    ui->m_parser->clearContextStack();
}



Status::Status(UserInterface *userInterface): Command(userInterface)
{
    cmdName = "status";
    cmdUsage = "Shows if you are connected to any changeset or not.";
    complPatterns.push_back("status");
}



Status::~Status()
{
}



void Status::operator()(const std::string &params)
{
    if (!params.empty()) {
        ui->io->reportError("Error: No parameters expected for command " + cmdName + ".");
        return;
    }
    if (ui->currentChangeset) {
        std::ostringstream ostr;
        ostr << "You are connected to changeset " << *(ui->currentChangeset) << ".";
        ui->io->printMessage(ostr.str());
    } else {
        ui->io->printMessage("You are not connected to any changeset.");
    }
}



Log::Log(UserInterface *userInterface): Command(userInterface)
{
    cmdName = "log";
    cmdUsage = "Command for operations with revisions and history.";
    complPatterns.push_back("log");
}



Log::~Log()
{
}



void Log::operator()(const std::string &params)
{
    if (params.empty()) {
        std::vector<Db::RevisionMetadata> revisions = ui->m_dbInteraction->allRevisions();
        ui->io->printRevisions(revisions);
        return;
    }

    std::vector<std::string> paramsList = extractParams(params);
    if (paramsList.size() > 2)
        ui->io->reportError("Too many parameters entered!");
    // FIXME: Some linker error
    /*boost::regex revRegex("^r[0-9]+$");
    for (std::vector<std::string>::iterator it = paramsList.begin(); it != paramsList.end(); ++it) {
        if (!boost::regex_match(*it, revRegex)) {
            ui->io->reportError("Invalid parameters entered!");
            return;
        }
    }*/

    std::vector<Db::ObjectModification> modifications;
    if (paramsList.size() == 1) {
        
    } else if (paramsList.size() == 2) {
        std::vector<Db::ObjectModification> modifications = ui->m_dbInteraction->revisionsDifference(
            stringToRevision(paramsList[0]), stringToRevision(paramsList[1]));
        ui->io->printDiff(modifications);
    }
}



Db::RevisionId Log::stringToRevision(const std::string &rev)
{
    if (rev.size() < 2)
        throw std::invalid_argument("Deska::Cli::Log::stringToRevision: Error while converting string to revision ID.");
    std::string revStr(rev.begin() + 1, rev.end());
    unsigned int revInt;
    std::istringstream iss(revStr);
    iss >> revInt;
    if (iss.fail())
        throw std::invalid_argument("Deska::Cli::Log::stringToRevision: Error while converting string to revision ID.");
    return Db::RevisionId(revInt);
}



Exit::Exit(UserInterface *userInterface): Command(userInterface)
{
    cmdName = "exit";
    cmdUsage = "Exits the CLI.";
    complPatterns.push_back("exit");
    complPatterns.push_back("quit");
}



Exit::~Exit()
{
}



void Exit::operator()(const std::string &params)
{
    if (!params.empty()) {
        ui->io->reportError("Error: No parameters expected for command " + cmdName + ".");
        return;
    }
    ui->exitLoop = true;
}



Dump::Dump(UserInterface *userInterface): Command(userInterface)
{
    cmdName = "dump";
    cmdUsage = "Prints everything in the DB. Optional parameter is filename, where to save the dump.";
    complPatterns.push_back("dump %file");
}



Dump::~Dump()
{
}



void Dump::operator()(const std::string &params)
{
    if (params.empty()) {
        // FIXME: Dump recursively
        //BOOST_FOREACH(const Deska::Db::Identifier &kindName, ui->m_dbInteraction->topLevelKinds()) {
        BOOST_FOREACH(const Deska::Db::Identifier &kindName, ui->m_dbInteraction->kindNames()) {
            BOOST_FOREACH(const Deska::Db::ObjectDefinition &object, ui->m_dbInteraction->kindInstances(kindName)) {
                //ui->io->printObject(object, 0, false);
                ui->io->printObject(object, 0, true);
                dumpObjectRecursive(object, 1);
            }
        }
    } else {
        std::ofstream ofs(params.c_str());
        if (!ofs) {
            ui->io->reportError("Error while dumping DB to file \"" + params + "\".");
            return;
        }
        // FIXME: Dump recursively
        //BOOST_FOREACH(const Deska::Db::Identifier &kindName, ui->m_dbInteraction->topLevelKinds()) {
        BOOST_FOREACH(const Deska::Db::Identifier &kindName, ui->m_dbInteraction->kindNames()) {
            BOOST_FOREACH(const Deska::Db::ObjectDefinition &object, ui->m_dbInteraction->kindInstances(kindName)) {
                //ui->io->printObject(object, 0, false, ofs);
                ui->io->printObject(object, 0, true, ofs);
                dumpObjectRecursive(object, 1, ofs);
            }
        }
        ui->io->printMessage("DB successfully dumped into file \"" + params + "\".");
    }  
}



void Dump::dumpObjectRecursive(const Db::ObjectDefinition &object, unsigned int depth, std::ostream &out)
{
    std::vector<Db::AttributeDefinition> attributes = ui->m_dbInteraction->allAttributes(object);
    ui->io->printAttributes(attributes, depth, out);
    std::vector<Db::ObjectDefinition> nestedObjs = ui->m_dbInteraction->allNestedObjects(object);
    for (std::vector<Db::ObjectDefinition>::iterator it = nestedObjs.begin(); it != nestedObjs.end(); ++it) {
        ui->io->printObject(*it, depth, false, out);
        dumpObjectRecursive(*it, depth + 1, out);
    }
    if (depth > 0)
        ui->io->printEnd(depth - 1, out);
}



Restore::Restore(UserInterface *userInterface): Command(userInterface)
{
    cmdName = "restore";
    cmdUsage = "Executes commands from a file. Can be used for restoring the DB from a dump. Requires file name with commands as a parameter. Lines with # at the beginning are comments and will not be parsed.";
    complPatterns.push_back("restore %file");
}



Restore::~Restore()
{
}



void Restore::operator()(const std::string &params)
{
    if (params.empty()) {
        ui->io->reportError("Error: This command requires file name as a parameter.");
        return;
    }
    if (!ui->currentChangeset) {
        ui->io->reportError("Error: Wou have to be connected to a changeset to perform restoration. Use commands \"start\" or \"resume\". Use \"help\" for more info.");
        return;
    }
    std::ifstream ifs(params.c_str());
    if (!ifs) {
        ui->io->reportError("Error while opening commands file \"" + params + "\".");
        return;
    }
    ui->nonInteractiveMode = true;
    std::string line;
    unsigned int lineNumber = 0;
    while (!getline(ifs, line).eof()) {
        ++lineNumber;
        if (!line.empty() && line[0] == '#')
            continue;
        ui->m_parser->parseLine(line);
        if (ui->parsingFailed)
            break;
    }
    ui->nonInteractiveMode = false;
    if (ui->parsingFailed) {
        std::ostringstream ostr;
        ostr << "Parsing of commands file failed on line " << lineNumber << ".";
        ui->io->reportError(ostr.str());
    } else {
        ui->io->printMessage("All commands successfully executed.");
    }
}



Help::Help(UserInterface *userInterface): Command(userInterface)
{
    cmdName = "help";
    cmdUsage = "Displays this list of commands with usages. Accepts parameter. For command name or parser keyword as parametr, usage is displayed, for kind name is displayed content of the kind and for word \"kinds\" are all defined kind names printed.";
    complPatterns.push_back("help kinds");
    // Help is not in commands map, because we are constructing it now.
    complPatterns.push_back("help help");
    for (UserInterface::CommandMap::iterator it = ui->commandsMap.begin(); it != ui->commandsMap.end(); ++it) {
        complPatterns.push_back("help " + it->first);
    }
    std::vector<std::string> kinds = ui->m_dbInteraction->kindNames();
    for (std::vector<std::string>::iterator it = kinds.begin(); it != kinds.end(); ++it) {
        complPatterns.push_back("help " + *it);
    }
    std::map<std::string, std::string> keywords = ui->m_parser->parserKeywordsUsage();
    for (std::map<std::string, std::string>::iterator it = keywords.begin(); it != keywords.end(); ++it) {
        complPatterns.push_back("help " + it->first);
    }
}



Help::~Help()
{
}



void Help::operator()(const std::string &params)
{
    if (!params.empty()) {
        if (params == "kinds") {
            ui->io->printHelpShowKinds(ui->m_dbInteraction->kindNames());
            return;
        }
        UserInterface::CommandMap::iterator itc = ui->commandsMap.find(params);
        if ( itc != ui->commandsMap.end()) {
            ui->io->printHelpCommand(params, itc->second->usage());
            return;
        }
        std::map<std::string, std::string> keywords = ui->m_parser->parserKeywordsUsage();
        std::map<std::string, std::string>::iterator itke = keywords.find(params);
        if ( itke != keywords.end()) {
            ui->io->printHelpKeyword(params, itke->second);
            return;
        }
        std::vector<std::string> kinds = ui->m_dbInteraction->kindNames();
        std::vector<std::string>::iterator itki = std::find(kinds.begin(), kinds.end(), params);
        if ( itki != kinds.end()) {
            ui->io->printHelpKind(params, ui->m_parser->parserKindsAttributes(params),
                                  ui->m_parser->parserKindsEmbeds(params));
            return;
        }
        ui->io->reportError("Error: No help entry for \"" + params + "\".");
        return;
    }
    std::map<std::string, std::string> cliCommands;
    for (UserInterface::CommandMap::iterator it = ui->commandsMap.begin(); it != ui->commandsMap.end(); ++it) {
        cliCommands[it->first] = it->second->usage();
    }
    ui->io->printHelp(cliCommands, ui->m_parser->parserKeywordsUsage());
}



UserInterface::UserInterface(DbInteraction *dbInteraction, Parser *parser, UserInterfaceIOBase *_io):
    m_dbInteraction(dbInteraction), m_parser(parser), io(_io), currentChangeset()
{
    // Register all commands
    typedef std::tr1::shared_ptr<Command> Ptr;
    commandsMap["start"] = Ptr(new Start(this));
    commandsMap["resume"] = Ptr(new Resume(this));
    commandsMap["commit"] = Ptr(new Commit(this));
    commandsMap["detach"] = Ptr(new Detach(this));
    commandsMap["abort"] = Ptr(new Abort(this));
    commandsMap["status"] = Ptr(new Status(this));
    commandsMap["log"] = Ptr(new Log(this));
    commandsMap["exit"] = Ptr(new Exit(this));
    commandsMap["quit"] = commandsMap["exit"];
    commandsMap["dump"] = Ptr(new Dump(this));
    commandsMap["restore"] = Ptr(new Restore(this));
    // Help has to be constructed last because of completions generating
    commandsMap["help"] = Ptr(new Help(this));

    // Register all commands completions
    for (CommandMap::iterator it = commandsMap.begin(); it != commandsMap.end(); ++it) {
        std::vector<std::string> cmdCompletions = it->second->completionPatterns();
        for (std::vector<std::string>::iterator itc = cmdCompletions.begin(); itc != cmdCompletions.end(); ++itc)
            io->addCommandCompletion(*itc);
    }
    nonInteractiveMode = false;
}



UserInterface::~UserInterface()
{
}



bool UserInterface::applyCategoryEntered(const ContextStack &context,
                                         const Db::Identifier &kind, const Db::Identifier &object)
{
    if (m_dbInteraction->objectExists(context))
        return true;

    // Object does not exist -> try to create it
    try {
        m_dbInteraction->createObject(context);
        return true;
    } catch (Deska::Db::ReCreateObjectError &e) {
        if (io->confirmRestoration(Db::ObjectDefinition(kind,object))) {
            m_dbInteraction->restoreDeletedObject(context);
            return true;
        } else {
            return false;
        }
    }
}



bool UserInterface::applySetAttribute(const ContextStack &context,
                                      const Db::Identifier &attribute, const Db::Value &value)
{
    try {
        m_dbInteraction->setAttribute(context, Db::AttributeDefinition(attribute, value));
        return true;
    } catch (Deska::Db::RemoteDbError &e) {
        // FIXME: potemkin's fix for the demo
        io->reportError(e.what());
        return false;
    }
}



bool UserInterface::applyRemoveAttribute(const ContextStack &context, const Db::Identifier &attribute)
{
    m_dbInteraction->removeAttribute(context, attribute);
    return true;
}



bool UserInterface::applyObjectsFilter(const ContextStack &context, const Db::Identifier &kind, 
                                       const Db::Filter &filter)
{
    // TODO
    return true;
}



bool UserInterface::applyFunctionShow(const ContextStack &context)
{
    if (context.empty()) {
        // Print top level objects if we are not in any context
        BOOST_FOREACH(const Deska::Db::Identifier &kindName, m_dbInteraction->topLevelKinds()) {
             io->printObjects(m_dbInteraction->kindInstances(kindName), 0, true);
        }
    } else {
        // If we are in some context, print all attributes and kind names
        showObjectRecursive(Db::ObjectDefinition(context.back().kind, contextStackToPath(context)), 0);
    }
    return true;
}



bool UserInterface::applyFunctionDelete(const ContextStack &context)
{
    m_dbInteraction->deleteObject(context);
    return true;
}



bool UserInterface::applyFunctionRename(const ContextStack &context, const Db::Identifier &newName)
{
    m_dbInteraction->renameObject(context, newName);
    return true;
}



bool UserInterface::confirmCategoryEntered(const ContextStack &context,
                                           const Db::Identifier &kind, const Db::Identifier &object)
{
    // We're entering into some context, so we should check whether the object in question exists, and if it does not,
    // ask the user whether to create it.
    if (m_dbInteraction->objectExists(context))
        return true;

    if (!currentChangeset) {
        io->reportError("Error: You have to be connected to a changeset to create an object. Use commands \"start\" or \"resume\". Use \"help\" for more info.");
        return false;
    }

    if (nonInteractiveMode)
        return true;

    // Object does not exist -> ask the user here
    return io->confirmCreation(Db::ObjectDefinition(kind,object));
}



bool UserInterface::confirmSetAttribute(const ContextStack &context,
                                        const Db::Identifier &attribute, const Db::Value &value)
{
    if (!currentChangeset) {
        io->reportError("Error: You have to be connected to a changeset to sat an attribue. Use commands \"start\" or \"resume\". Use \"help\" for more info.");
        return false;
    }
    return true;
}



bool UserInterface::confirmRemoveAttribute(const ContextStack &context, const Db::Identifier &attribute)
{
    if (!currentChangeset) {
        io->reportError("Error: You have to be connected to a changeset to remove an attribute. Use commands \"start\" or \"resume\". Use \"help\" for more info.");
        return false;
    }
    return true;
}



bool UserInterface::confirmObjectsFilter(const ContextStack &context, const Db::Identifier &kind,
                                         const Db::Filter &filter)
{
    return true;
}



bool UserInterface::confirmFunctionShow(const ContextStack &context)
{
    return true;
}



bool UserInterface::confirmFunctionDelete(const ContextStack &context)
{
    if (!currentChangeset) {
        io->reportError("Error: You have to be connected to a changeset to delete an object. Use commands \"start\" or \"resume\". Use \"help\" for more info.");
        return false;
    }

    if (nonInteractiveMode)
        return true;
    return io->confirmDeletion(context.back());
}



bool UserInterface::confirmFunctionRename(const ContextStack &context, const Db::Identifier &newName)
{
    if (!currentChangeset) {
        io->reportError("Error: You have to be connected to a changeset to rename an object. Use commands \"start\" or \"resume\". Use \"help\" for more info.");
        return false;
    }

    return true;
}



void UserInterface::reportParseError(const ParserException &error)
{
    parsingFailed = true;
    if (error.offset() == 0) {
        io->reportError("Error while parsing CLI command or " + error.dump());
    } else {
        io->reportError(error.dump());
    }
}



void UserInterface::run()
{
    io->printMessage("Deska CLI started. For usage info try typing \"help\".");
    std::string line;
    exitLoop = false;
    while (!exitLoop) {
        parsingFailed = false;
        line = io->readLine(contextStackToString(m_parser->currentContextStack()));

        // Split line to command and arguments
        std::string::iterator commandEnd = line.begin();
        while ((commandEnd != line.end()) && (*commandEnd != ' '))
            ++commandEnd;
        std::string parsedCommand(line.begin(), commandEnd);
        std::string parsedArguments((commandEnd == line.end() ? commandEnd : (commandEnd +1)), line.end());
        
        if (commandsMap.find(parsedCommand) == commandsMap.end()) {
            // Command not found -> use CLI parser
            m_parser->parseLine(line);
        } else {
            // Command found -> run it
            (*(commandsMap[parsedCommand]))(parsedArguments);
        }
    }
}



void UserInterface::showObjectRecursive(const Db::ObjectDefinition &object, unsigned int depth)
{
    bool printEnd = false;
    std::vector<std::pair<Db::AttributeDefinition, Db::Identifier> > attributes =
        m_dbInteraction->allAttributesResolvedWithOrigin(object);
    printEnd = printEnd || !attributes.empty();
    io->printAttributesWithOrigin(attributes, depth);
    std::vector<Db::ObjectDefinition> nestedObjs = m_dbInteraction->allNestedObjects(object);
    printEnd = printEnd || !nestedObjs.empty();
    for (std::vector<Db::ObjectDefinition>::iterator it = nestedObjs.begin(); it != nestedObjs.end(); ++it) {
        io->printObject(*it, depth, false);
        showObjectRecursive(*it, depth + 1);
    }
    if (printEnd && (depth > 0))
        io->printEnd(depth - 1);
}



}
}
