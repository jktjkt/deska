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
#include <boost/algorithm/string/case_conv.hpp>

#include "UserInterface.h"
#include "deska/db/JsonApi.h"


namespace Deska
{
namespace Cli
{



UserInterface::UserInterface(std::ostream &outStream, std::ostream &errStream, std::istream &inStream,
                             DbInteraction *dbInteraction, Parser *parser):
    out(outStream.rdbuf()), err(errStream.rdbuf()), in(inStream.rdbuf()),
    m_dbInteraction(dbInteraction), m_parser(parser), prompt("> ")
{
}



void UserInterface::applyCategoryEntered(const Db::ContextStack &context,
                                         const Db::Identifier &kind, const Db::Identifier &object)
{
    std::vector<Db::Identifier> instances;
    instances = m_dbInteraction->kindInstances(kind);

    if (std::find(instances.begin(), instances.end(), Db::toPath(context)) == instances.end()) {
        m_dbInteraction->createObject(context);
    }
}



void UserInterface::applySetAttribute(const Db::ContextStack &context,
                                      const Db::Identifier &attribute, const Db::Value &value)
{
    m_dbInteraction->setAttribute(context, Db::AttributeDefinition(attribute, value));
}



void UserInterface::applyFunctionShow(const Db::ContextStack &context)
{
    printAttributes(context);
    printNestedKinds(context);
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
    std::vector<Db::Identifier> instances;
    instances = m_dbInteraction->kindInstances(kind);

    if (std::find(instances.begin(), instances.end(), Db::toPath(context)) != instances.end()) {
        // Object exists
        return true;
    }

    // Object does not exist -> ask the user here
    std::ostringstream ss;
    ss << Db::ObjectDefinition(kind,object) << " does not exist. Create?";
    return askForConfirmation(ss.str());    
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
    std::ostringstream ss;
    ss << "Are you sure you want to delete object " << context.back() << "?";
    return askForConfirmation(ss.str());
}



void UserInterface::reportError(const std::string &errorMessage)
{
    err << errorMessage << std::endl;
}



bool UserInterface::askForConfirmation(const std::string &prompt)
{
    out << prompt << " ";
    std::string answer;
    getline(in, answer);
    boost::algorithm::to_lower(answer);
    return answer == "yes" || answer == "y";
}



void UserInterface::dumpDbContents()
{
    std::vector<Db::ObjectDefinition> objects;
    objects = m_dbInteraction->allObjects();
    for (std::vector<Db::ObjectDefinition>::iterator it = objects.begin(); it != objects.end(); ++it) {
        out << *it << std::endl;
        std::vector<Db::AttributeDefinition> attributes;
        attributes = m_dbInteraction->allAttributes(*it);
        for (std::vector<Db::AttributeDefinition>::iterator ita = attributes.begin(); ita != attributes.end(); ++ita) {
            out << "    " << *ita << std::endl;
        }
        out << "end" << std::endl;
        out << std::endl;
    }
}



void UserInterface::printAttributes(const Db::ContextStack &context)
{
    std::vector<Db::AttributeDefinition> attributes;
    attributes = m_dbInteraction->allAttributes(context);
    for (std::vector<Db::AttributeDefinition>::iterator it = attributes.begin(); it != attributes.end(); ++it) {
        out << *it << std::endl;
    }
}



void UserInterface::printNestedKinds(const Db::ContextStack &context)
{
    std::vector<Db::ObjectDefinition> kinds;
    kinds = m_dbInteraction->allNestedKinds(context);
    for (std::vector<Db::ObjectDefinition>::iterator it = kinds.begin(); it != kinds.end(); ++it) {
        out << *it << std::endl;
    }
}



void UserInterface::commitChangeset()
{
    std::string message;
    out << "Commit message: ";
    getline(in, message);
    m_dbInteraction->commitChangeset(message);
}



void UserInterface::detachFromChangeset()
{
    std::string message;
    out << "Log message for detaching: ";
    getline(in, message);
    m_dbInteraction->detachFromChangeset(message);
}



void UserInterface::abortChangeset()
{
    m_dbInteraction->abortChangeset();
}



void UserInterface::printHelp()
{
    out << "CLI commands:" << std::endl;
    out << "exit   - Exits the CLI" << std::endl;
    out << "quit   - Exits the CLI" << std::endl;
    out << "dump   - Prints everything in the DB" << std::endl;
    out << "commit - Displays promt for commit message and commits current changeset" << std::endl;
    out << "detach - Displays promt for detach message and detaches from current changeset" << std::endl;
    out << "abort  - Aborts current changeset" << std::endl;
    out << "help   - Displays this list of commands" << std::endl;
    out << std::endl;
    out << "Parser keywords:" << std::endl;
    out << "delete - Deletes object given as parameter (e.g. delete hardware hp456)" << std::endl;
    out << "show   - Shows attributes and nested kinds of the object" << std::endl;
}



void UserInterface::run()
{
    char *userName = getenv("USER");
    if (userName != 0)
        out << "Hi " << userName << "! ";
    out << "Welcome to Deska CLI." << std::endl << std::endl;
    // TODO: Rewrite this function using Redline--
    try {
        // Print list of pending changesets, so user can choose one
        std::vector<Db::PendingChangeset> pendingChangesets = m_dbInteraction->allPendingChangesets();
        out << "Pending changesets: " << std::endl << std::endl;
        if (pendingChangesets.empty()) {
            out << "No pending changesets." << std::endl;
        } else {
            for (unsigned int i = 0; i < pendingChangesets.size(); ++i) {
                out << i << ": " << pendingChangesets[i] << std::endl;
            }
        }
        out << "n: No changset" << std::endl;
        out << "c: Create new changset" << std::endl << std::endl;
        out << "Changeset to attach to: ";
        // Waiting until user enteres correct input.
        for (;;)
        {
            std::string choice;
            getline(in, choice);
            boost::algorithm::to_lower(choice);
            if (choice == "n") {
                // do nothing
                break;
            } else if (choice == "c") {
                m_dbInteraction->createNewChangeset();
                break;
            } else {
                std::istringstream ss(choice);
                unsigned int res;
                ss >> res;
                // Check whether the input is a number and represents any pending changeset
                if (!ss.fail() && res < pendingChangesets.size()) {
                    m_dbInteraction->resumeChangeset(pendingChangesets[res].revision);
                    break;
                }
            }
            reportError("Bad choice input. Try againg.");
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
    std::string line;
    out << prompt;
    Db::ContextStack context;
    std::ostringstream promptBuilder;
    while (getline(in, line)) {
        // FIXME: For some reason some times the line is read even though user did not enter anything. Bug #222
        // Hack for bug #222.
        if (line.empty())
            continue;

        if (line == "exit" || line == "quit") {
            break;
        } else if (line == "dump") {
            dumpDbContents();
        } else if (line == "commit") {
            commitChangeset();
        } else if (line == "detach") {
            detachFromChangeset();
        } else if (line == "abort") {
            abortChangeset();
        } else if (line == "help") {
            printHelp();
        } else {
            m_parser->parseLine(line);
        }

        context = m_parser->currentContextStack();
        prompt = Db::toString(context) + "> ";
        out << prompt;
    }
}



}
}
