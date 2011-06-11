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

#include "DbInteraction.h"
#include "Exceptions.h"
#include "Parser.h"
#include "UserInterface.h"
#include "UserInterfaceIO.h"
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
    if (ui->inChangeset) {
        ui->io->reportError("Error: You are already in the changeset!");
    } else {
        ui->m_dbInteraction->createNewChangeset(); 
        ui->inChangeset = true;
        ui->io->printMessage("Changeset started.");
    }
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
    if (ui->inChangeset) {
        ui->io->reportError("Error: You are already in the changeset!");
    } else {
        try {
            // Print list of pending changesets, so user can choose one
            std::vector<Db::PendingChangeset> pendingChangesets = ui->m_dbInteraction->allPendingChangesets();
            int choice = ui->io->chooseChangeset(pendingChangesets);

            if (choice >= 0) {
                // Some changeset was choosen
                ui->m_dbInteraction->resumeChangeset(pendingChangesets[choice].revision);
                ui->inChangeset = true;
                ui->io->printMessage("Changeset resumed.");
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
    if (ui->inChangeset) {
        std::string commitMessage;
        if (!params.empty()) {
            commitMessage = params;
        } else {
            commitMessage = ui->io->askForCommitMessage();
        }
        ui->m_dbInteraction->commitChangeset(commitMessage);
        ui->inChangeset = false;
        ui->io->printMessage("Changeset commited.");
        ui->m_parser->clearContextStack();
    } else {
        ui->io->reportError("Error: You are not in any changeset!");
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
    if (ui->inChangeset) {
        std::string detachMessage;
        if (!params.empty()) {
            detachMessage = params;
        } else {
            detachMessage = ui->io->askForDetachMessage();
        }
        ui->m_dbInteraction->detachFromChangeset(detachMessage);
        ui->inChangeset = false;
        ui->io->printMessage("Changeset detached.");
        ui->m_parser->clearContextStack();
    } else {
        ui->io->reportError("Error: You are not in any changeset!");
    }
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
    if (ui->inChangeset) {
        ui->m_dbInteraction->abortChangeset();
        ui->inChangeset = false;
        ui->io->printMessage("Changeset aborted.");
        ui->m_parser->clearContextStack();
    } else {
        ui->io->reportError("Error: You are not in any changeset!");
    }
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
    if (ui->inChangeset) {
        ui->io->printMessage("You are connected to a changeset.");
    } else {
        ui->io->printMessage("You are not connected to any changeset.");
    }
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
        BOOST_FOREACH(const Deska::Db::Identifier &kindName, ui->m_dbInteraction->kindNames()) {
            BOOST_FOREACH(const Deska::Db::ObjectDefinition &object, ui->m_dbInteraction->kindInstances(kindName)) {
                ui->io->printObject(object, 0, true);
                ui->io->printAttributes(ui->m_dbInteraction->allAttributes(object), 1);
                ui->io->printEnd(0);
            }
        }
    } else {
        std::ofstream ofs(params.c_str());
        if (!ofs) {
            ui->io->reportError("Error while dumping DB to file \"" + params + "\".");
            return;
        }
        BOOST_FOREACH(const Deska::Db::Identifier &kindName, ui->m_dbInteraction->kindNames()) {
            BOOST_FOREACH(const Deska::Db::ObjectDefinition &object, ui->m_dbInteraction->kindInstances(kindName)) {
                ui->io->printObject(object, 0, true, ofs);
                ui->io->printAttributes(ui->m_dbInteraction->allAttributes(object), 1, ofs);
                ui->io->printEnd(0, ofs);
            }
        }
        ui->io->printMessage("DB successfully dumped into file \"" + params + "\".");
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



UserInterface::UserInterface(DbInteraction *dbInteraction, Parser *parser, UserInterfaceIO *_io):
    m_dbInteraction(dbInteraction), m_parser(parser), io(_io), inChangeset(false)
{
    // Register all commands
    typedef std::tr1::shared_ptr<Command> Ptr;
    commandsMap["start"] = Ptr(new Start(this));
    commandsMap["resume"] = Ptr(new Resume(this));
    commandsMap["commit"] = Ptr(new Commit(this));
    commandsMap["detach"] = Ptr(new Detach(this));
    commandsMap["abort"] = Ptr(new Abort(this));
    commandsMap["status"] = Ptr(new Status(this));
    commandsMap["exit"] = Ptr(new Exit(this));
    commandsMap["quit"] = commandsMap["exit"];
    commandsMap["dump"] = Ptr(new Dump(this));
    // Help has to be constructed last because of completions generating
    commandsMap["help"] = Ptr(new Help(this));

    // Register all commands completions
    for (CommandMap::iterator it = commandsMap.begin(); it != commandsMap.end(); ++it) {
        std::vector<std::string> cmdCompletions = it->second->completionPatterns();
        for (std::vector<std::string>::iterator itc = cmdCompletions.begin(); itc != cmdCompletions.end(); ++itc)
            io->addCommandCompletion(*itc);
    }
}



UserInterface::~UserInterface()
{
}



void UserInterface::applyCategoryEntered(const Db::ContextStack &context,
                                         const Db::Identifier &kind, const Db::Identifier &object)
{
    if (!m_dbInteraction->objectExists(context))
        m_dbInteraction->createObject(context);
}



void UserInterface::applySetAttribute(const Db::ContextStack &context,
                                      const Db::Identifier &attribute, const Db::Value &value)
{
    m_dbInteraction->setAttribute(context, Db::AttributeDefinition(attribute, value));
}



void UserInterface::applyRemoveAttribute(const Db::ContextStack &context, const Db::Identifier &attribute)
{
    m_dbInteraction->removeAttribute(context, attribute);
}



void UserInterface::applyFunctionShow(const Db::ContextStack &context)
{
    if (context.empty()) {
        // Print top level objects if we are not in any context
        BOOST_FOREACH(const Deska::Db::Identifier &kindName, m_dbInteraction->kindNames()) {
             io->printObjects(m_dbInteraction->kindInstances(kindName), 0, true);
        }
    } else {
        // If we are in some context, print all attributes and kind names
        std::vector<Db::AttributeDefinition> attributes = m_dbInteraction->allAttributes(context);
        io->printAttributes(attributes, 0);
        std::vector<Db::ObjectDefinition> kinds = m_dbInteraction->allNestedKinds(context);
        io->printObjects(kinds, 0, false);
    }
}



void UserInterface::applyFunctionDelete(const Db::ContextStack &context)
{
    m_dbInteraction->deleteObject(context);
}



void UserInterface::applyFunctionRename(const Db::ContextStack &context, const Db::Identifier &newName)
{
    m_dbInteraction->renameObject(context, newName);
}



bool UserInterface::confirmCategoryEntered(const Db::ContextStack &context,
                                           const Db::Identifier &kind, const Db::Identifier &object)
{
    // We're entering into some context, so we should check whether the object in question exists, and if it does not,
    // ask the user whether to create it.
    if (m_dbInteraction->objectExists(context))
        return true;
    // Object does not exist -> ask the user here
    return io->confirmCreation(Db::ObjectDefinition(kind,object));
}



bool UserInterface::confirmSetAttribute(const Db::ContextStack &context,
                                        const Db::Identifier &attribute, const Db::Value &value)
{
    return true;
}



bool UserInterface::confirmRemoveAttribute(const Db::ContextStack &context, const Db::Identifier &attribute)
{
    return true;
}



bool UserInterface::confirmFunctionShow(const Db::ContextStack &context)
{
    return true;
}



bool UserInterface::confirmFunctionDelete(const Db::ContextStack &context)
{
    return io->confirmDeletion(context.back());
}



bool UserInterface::confirmFunctionRename(const Db::ContextStack &context, const Db::Identifier &newName)
{
    return true;
}



void UserInterface::reportError(const std::string &errorMessage)
{
    io->reportError(errorMessage);
}



void UserInterface::run()
{
    io->printMessage("Deska CLI started. For usage info try typing \"help\".");
    std::string line;
    exitLoop = false;
    while (!exitLoop) {
        line = io->readLine(Db::contextStackToString(m_parser->currentContextStack()));

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



}
}
