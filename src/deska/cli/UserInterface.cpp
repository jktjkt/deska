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

#include "UserInterface.h"
#include "deska/db/JsonApi.h"



namespace Deska
{
namespace Cli
{



UserInterface::UserInterface(DbInteraction *dbInteraction, Parser *parser, UserInterfaceIO *_io):
    m_dbInteraction(dbInteraction), m_parser(parser), prompt(""), io(_io)
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
    std::vector<Db::AttributeDefinition> attributes = m_dbInteraction->allAttributes(context);
    io->printAttributes(attributes, 0);
    std::vector<Db::ObjectDefinition> kinds = m_dbInteraction->allNestedKinds(context);
    io->printObjects(kinds, 0);
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
    std::vector<Db::ObjectDefinition> objects;
    objects = m_dbInteraction->allObjects();
    for (std::vector<Db::ObjectDefinition>::iterator it = objects.begin(); it != objects.end(); ++it) {
        io->printObject(*it, 0);
        std::vector<Db::AttributeDefinition> attributes = m_dbInteraction->allAttributes(*it);
        io->printAttributes(attributes, 1);
    }
}



void UserInterface::run()
{
    try {
    // Print list of pending changesets, so user can choose one
    std::vector<Db::PendingChangeset> pendingChangesets = m_dbInteraction->allPendingChangesets();
    int choice = io->chooseChangeset(pendingChangesets);

    if (choice == -2) {
        // do nothing
    } else if (choice == -1) {
        m_dbInteraction->createNewChangeset();
    } else {
        m_dbInteraction->resumeChangeset(pendingChangesets[choice].revision);
    }
        
        // Now that we've established our preconditions, let's enter the event loop
        eventLoop();
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



void UserInterface::eventLoop()
{
    // TODO: Rewrite this function using Redline--

    std::string line;
    io->printPrompt(prompt);
    Db::ContextStack context;
    for (;;) {
        line = io->readLine();
        // FIXME: For some reason some times the line is read even though user did not enter anything. Bug #222
        // Hack for bug #222.
        if (line.empty())
            continue;

        if (line == "exit" || line == "quit") {
            break;
        } else if (line == "dump") {
            dumpDbContents();
        } else if (line == "commit") {
            m_dbInteraction->commitChangeset(io->askForCommitMessage());
        } else if (line == "detach") {
            m_dbInteraction->detachFromChangeset(io->askForDetachMessage());
        } else if (line == "abort") {
            m_dbInteraction->abortChangeset();
        } else if (line == "help") {
            io->printHelp();
        } else {
            m_parser->parseLine(line);
        }

        context = m_parser->currentContextStack();
        prompt = Db::toString(context);
        io->printPrompt(prompt);
    }
}



}
}
