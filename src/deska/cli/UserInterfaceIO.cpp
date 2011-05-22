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



UserInterfaceIO::UserInterfaceIO(std::ostream &outStream, std::ostream &errStream, std::istream &inStream):
    out(outStream.rdbuf()), err(errStream.rdbuf()), in(inStream.rdbuf()), tabSize(4), promptEnd("> ")
{
}



void UserInterfaceIO::reportError(const std::string &errorMessage)
{
    err << errorMessage << std::endl;
}



void UserInterfaceIO::printMessage(const std::string &message)
{
    out << message << std::endl;
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
    out << prompt << " ";
    std::string answer;
    getline(in, answer);
    boost::algorithm::to_lower(answer);
    return answer == "yes" || answer == "y";
}



std::string UserInterfaceIO::askForCommitMessage()
{
    std::string message;
    out << "Commit message: ";
    getline(in, message);
    return message;
}



std::string UserInterfaceIO::askForDetachMessage()
{
    std::string message;
    out << "Log message for detaching: ";
    getline(in, message);
    return message;
}



void UserInterfaceIO::printHelp()
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



int UserInterfaceIO::chooseChangeset(const std::vector<Db::PendingChangeset> &pendingChangesets)
{
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
            return -2;
        } else if (choice == "c") {
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



void UserInterfaceIO::printPrompt(const std::string &prompt)
{
    out << prompt << promptEnd;
}



std::string UserInterfaceIO::readLine()
{
    std::string line;
    getline(in, line);
    return line;
}



void UserInterfaceIO::printAttributes(const std::vector<Db::AttributeDefinition> &attributes, int indentLevel)
{
    for (std::vector<Db::AttributeDefinition>::const_iterator it = attributes.begin(); it != attributes.end(); ++it) {
        printAttribute(*it, indentLevel);
    }
    if (indentLevel > 0)
        printEnd(indentLevel - 1);
}



void UserInterfaceIO::printObjects(const std::vector<Db::ObjectDefinition> &objects, int indentLevel)
{
    for (std::vector<Db::ObjectDefinition>::const_iterator it = objects.begin(); it != objects.end(); ++it) {
        printObject(*it, indentLevel);
    }
    if (indentLevel > 0)
        printEnd(indentLevel - 1);
}



void UserInterfaceIO::printAttribute(const Db::AttributeDefinition &attribute, int indentLevel)
{
    out << indent(indentLevel) << attribute << std::endl;
}



void UserInterfaceIO::printObject(const Db::ObjectDefinition &object, int indentLevel)
{
    out << indent(indentLevel) << object << std::endl;
}



void UserInterfaceIO::printEnd(int indentLevel)
{
    out << indent(indentLevel) << "end" << std::endl;
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
