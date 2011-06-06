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

#include "UserInterfaceIO.h"


namespace Deska
{
namespace Cli
{



UserInterfaceIO::UserInterfaceIO():
    tabSize(4), promptEnd("> ")//, reader(".deska_cli_history", 64)
{
    // FIXME: Completitions will be obtained from the UserInterface dynamicly
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
    completitions.push_back("rename");
    reader.RegisterCompletions(completitions);
}



void UserInterfaceIO::reportError(const std::string &errorMessage)
{
    std::cerr << errorMessage << std::endl;
}



void UserInterfaceIO::printMessage(const std::string &message)
{
    std::cout << message << std::endl;
}



bool UserInterfaceIO::confirmDeletion(const Db::ObjectDefinition &object)
{
    std::ostringstream ss;
    ss << "Are you sure you want to delete object " << object << "?";
    return askForConfirmation(ss.str());
}



bool UserInterfaceIO::confirmCreation(const Db::ObjectDefinition &object)
{
    std::ostringstream ss;
    ss << object << " does not exist. Create?";
    return askForConfirmation(ss.str());
}



bool UserInterfaceIO::askForConfirmation(const std::string &prompt)
{
    std::cout << prompt << " ";
    std::string answer;
    getline(std::cin, answer);
    boost::algorithm::to_lower(answer);
    return answer == "yes" || answer == "y";
}



std::string UserInterfaceIO::askForCommitMessage()
{
    std::string message;
    std::cout << "Commit message: ";
    getline(std::cin, message);
    return message;
}



std::string UserInterfaceIO::askForDetachMessage()
{
    std::string message;
    std::cout << "Log message for detaching: ";
    getline(std::cin, message);
    return message;
}



void UserInterfaceIO::printHelp()
{
    std::cout << "CLI commands:" << std::endl;
    std::cout << "start  - Starts new changeset" << std::endl;
    std::cout << "resume - Displays list of pending changesets with ability to connect to one." << std::endl;
    std::cout << "commit - Displays promt for commit message and commits current changeset." << std::endl;
    std::cout << "detach - Displays promt for detach message and detaches from current changeset." << std::endl;
    std::cout << "abort  - Aborts current changeset." << std::endl;
    std::cout << "status - Shows if you are connected to any changeset or not." << std::endl;
    std::cout << "exit   - Exits the CLI." << std::endl;
    std::cout << "quit   - Exits the CLI." << std::endl;
    std::cout << "dump   - Prints everything in the DB." << std::endl;
    std::cout << "help   - Displays this list of commands." << std::endl;
    std::cout << std::endl;
    std::cout << "Parser keywords:" << std::endl;
    std::cout << "delete - Deletes object given as parameter (e.g. delete hardware hp456). Longer" << std::endl;
    std::cout << "         parameters are also allowed (e.g. delete host golias120 interface eth0)" << std::endl;
    std::cout << "         This will delete only interface eth0 in the object host golias120." << std::endl;
    std::cout << "show   - Shows attributes and nested kinds of the object. Parameter is here" << std::endl;
    std::cout << "         optional and works in the same way as for delete. When executed without" << std::endl;
    std::cout << "         parameter at top-level, it shows all object kinds and names." << std::endl;
    std::cout << "rename - Renames an object to name given as parameter" << std::endl;
    std::cout << "         (e.g. rename hardware hp456 hp567). Object in current context could be" << std::endl;
    std::cout << "         renamed also using short form (e.g. rename hp567)." << std::endl;
    std::cout << "end    - Leaves one level of current context" << std::endl;
}



int UserInterfaceIO::chooseChangeset(const std::vector<Db::PendingChangeset> &pendingChangesets)
{
    std::cout << "Pending changesets: " << std::endl << std::endl;
    if (pendingChangesets.empty()) {
        std::cout << "No pending changesets." << std::endl;
        return -2;
    } else {
        for (unsigned int i = 0; i < pendingChangesets.size(); ++i) {
            std::cout << i << ": " << pendingChangesets[i] << std::endl;
        }
    }
    std::cout << "n: No changset" << std::endl << std::endl;    
    // Waiting until user enteres correct input.
    for (;;)
    {
        std::cout << "Changeset to attach to: ";
        std::string choice;
        getline(std::cin, choice);
        boost::algorithm::to_lower(choice);
        if (choice == "n") {
            return -1;
        } else {
            std::istringstream ss(choice);
            unsigned int res;
            ss >> res;
            // Check whether the input is a number and represents any pending changeset
            if (!ss.fail() && res < pendingChangesets.size()) {
                return res;
            }
        }
        reportError("Bad choice input. Try againg.");
    }
}



std::string UserInterfaceIO::readLine(const std::string &prompt)
{
    return reader.GetLine(prompt + promptEnd);
}



void UserInterfaceIO::printAttributes(const std::vector<Db::AttributeDefinition> &attributes, int indentLevel)
{
    if (attributes.empty())
        return;
    for (std::vector<Db::AttributeDefinition>::const_iterator it = attributes.begin(); it != attributes.end(); ++it) {
        printAttribute(*it, indentLevel);
    }
    if (indentLevel > 0)
        printEnd(indentLevel - 1);
}



void UserInterfaceIO::printObjects(const std::vector<Db::ObjectDefinition> &objects, int indentLevel, bool fullName)
{
    if (objects.empty())
        return;
    for (std::vector<Db::ObjectDefinition>::const_iterator it = objects.begin(); it != objects.end(); ++it) {
        printObject(*it, indentLevel, fullName);
    }
    if (indentLevel > 0)
        printEnd(indentLevel - 1);
}



void UserInterfaceIO::printAttribute(const Db::AttributeDefinition &attribute, int indentLevel)
{
    std::cout << indent(indentLevel) << attribute << std::endl;
}



void UserInterfaceIO::printObject(const Db::ObjectDefinition &object, int indentLevel, bool fullName)
{
    if (fullName)
        std::cout << indent(indentLevel) << object << std::endl;
    else
        std::cout << indent(indentLevel) << Db::ObjectDefinition(object.kind, Db::PathToVector(object.name).back()) << std::endl;
}



void UserInterfaceIO::printEnd(int indentLevel)
{
    std::cout << indent(indentLevel) << "end" << std::endl;
}



std::string UserInterfaceIO::indent(int indentLevel)
{
    std::ostringstream ss;
    for (unsigned int i = 0; i < (indentLevel * tabSize); ++i) {
        ss << " ";
    }
    return ss.str();
}



}
}
