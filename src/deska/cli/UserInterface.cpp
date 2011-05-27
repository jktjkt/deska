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
#include <boost/foreach.hpp>

#include "UserInterface.h"
#include "deska/db/JsonApi.h"

#include "SReadline/SReadline.h"



namespace Deska
{
namespace Cli
{



UserInterface::UserInterface(DbInteraction *dbInteraction, Parser *parser, UserInterfaceIO *_io):
    m_dbInteraction(dbInteraction), m_parser(parser), prompt(""), io(_io), inChangeset(false)
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



bool UserInterface::confirmFunctionShow(const Db::ContextStack &context)
{
    return true;
}



bool UserInterface::confirmFunctionDelete(const Db::ContextStack &context)
{
    return io->confirmDeletion(context.back());
}



void UserInterface::reportError(const std::string &errorMessage)
{
    io->reportError(errorMessage);
}



void UserInterface::dumpDbContents()
{
    BOOST_FOREACH(const Deska::Db::Identifier &kindName, m_dbInteraction->kindNames()) {
        BOOST_FOREACH(const Deska::Db::ObjectDefinition &object, m_dbInteraction->kindInstances(kindName)) {
            io->printObject(object, 0, true);
            io->printAttributes(m_dbInteraction->allAttributes(object), 1);
        }
    }
}



void UserInterface::resumeChangeset()
{
    try {
        // Print list of pending changesets, so user can choose one
        std::vector<Db::PendingChangeset> pendingChangesets = m_dbInteraction->allPendingChangesets();
        int choice = io->chooseChangeset(pendingChangesets);

        if (choice >= 0) {
            // Some changeset was choosen
            m_dbInteraction->resumeChangeset(pendingChangesets[choice].revision);
            inChangeset = true;
            io->printMessage("Changeset resumed.");
        }
        
    } catch (Deska::Db::NotFoundError &e) {
        reportError("Server reports an error:\nObject not found:\n\n" + e.whatWithBacktrace() + "\n");
    } catch (Deska::Db::NoChangesetError &e) {
        reportError("Server reports an error:\nYou aren't associated to a changeset:\n\n" + e.whatWithBacktrace() + "\n");
    } catch (Deska::Db::SqlError &e) {
        reportError("Server reports an error:\nError in executing an SQL statement:\n\n" + e.whatWithBacktrace() + "\n");
    } catch (Deska::Db::ServerError &e) {
        reportError("Server reports an error:\nInternal server error:\n\n" + e.whatWithBacktrace() + "\n");
    } catch (Deska::Db::JsonSyntaxError &e) {
        reportError("Cannot parse JSON data.\n " + e.whatWithBacktrace() + "\n");
    } catch (Deska::Db::JsonStructureError &e) {
        reportError("Received malformed JSON data:\n " + e.whatWithBacktrace() + "\n");
    }
}



void UserInterface::run()
{
    // TODO: Rewrite this function using Redline--

    // FIXME: Only temporary usage of SReadline.
    swift::SReadline reader(".cli_history",64);
    std::vector<std::string> completitions;
    completitions.push_back("exit");
    completitions.push_back("quit");
    completitions.push_back("dump");
    completitions.push_back("commit");
    completitions.push_back("detach");
    completitions.push_back("abort");
    completitions.push_back("start");
    completitions.push_back("resume");
    completitions.push_back("status");
    completitions.push_back("help");
    completitions.push_back("show");
    completitions.push_back("delete");
    reader.RegisterCompletions(completitions);

    io->printMessage("Deska CLI started. For usage info try typing \"help\".");
    std::string line;
    Db::ContextStack context;
    for (;;) {
        //io->printPrompt(prompt);
        //line = io->readLine();
        line = reader.GetLine(prompt + "> ");


        if (line == "exit" || line == "quit") {
            break;
        } else if (line == "dump") {
            dumpDbContents();
        } else if (line == "commit") {
            if (inChangeset) {
                m_dbInteraction->commitChangeset(io->askForCommitMessage());
                inChangeset = false;
                io->printMessage("Changeset commited.");
            } else {
                reportError("Error: You are not in any changeset!");
            }
        } else if (line == "detach") {
            if (inChangeset) {
                m_dbInteraction->detachFromChangeset(io->askForDetachMessage());
                inChangeset = false;
                io->printMessage("Changeset detached.");
            } else {
                reportError("Error: You are not in any changeset!");
            }
        } else if (line == "abort") {
            if (inChangeset) {
                m_dbInteraction->abortChangeset();
                inChangeset = false;
                io->printMessage("Changeset aborted.");
            } else {
                reportError("Error: You are not in any changeset!");
            }
        } else if (line == "start") {
            if (inChangeset) {
                reportError("Error: You are already in the changeset!");
            } else {
                m_dbInteraction->createNewChangeset(); 
                inChangeset = true;
                io->printMessage("Changeset started.");
            }
        } else if (line == "resume") {
            if (inChangeset) {
                reportError("Error: You are already in the changeset!");
            } else {
                resumeChangeset();
            }
        } else if (line == "status") {
            if (inChangeset) {
                io->printMessage("You are connected to a changeset.");
            } else {
                io->printMessage("You are not connected to any changeset.");
            }
        } else if (line == "help") {
            io->printHelp();
        } else {
            m_parser->parseLine(line);
        }

        context = m_parser->currentContextStack();
        prompt = Db::contextStackToString(context);
    }
}



}
}
