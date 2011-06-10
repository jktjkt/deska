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

#include <boost/algorithm/string/trim.hpp>
#include <boost/tokenizer.hpp>

#include "UserInterfaceIO.h"


namespace Deska
{
namespace Cli
{


CliCompleter::CliCompleter(Parser* parser): m_parser(parser)
{
}



CliCompleter::~CliCompleter()
{
}



std::vector<std::string> CliCompleter::getCompletions(const std::string &line,
                                                       std::string::const_iterator start,
                                                       std::string::const_iterator end)
{
    // Do not pass the last incomplete token to the parser
    std::string::const_iterator space = end;
    bool noSpace = false;
    while (*space != ' ') {
        if (space == line.begin()) {
            noSpace = true;
            break;
        }
        --space;
    }
    std::vector<std::string> completions;
    if (noSpace)
        completions = m_parser->tabCompletionPossibilities(std::string());
    else
        completions = m_parser->tabCompletionPossibilities(std::string(line.begin(), (space + 1)));

    completions.insert(completions.end(), commandCompletions.begin(), commandCompletions.end());
    return completions;
}



void CliCompleter::addCommandCompletion(const std::string &completion)
{
    commandCompletions.push_back(completion);
}



UserInterfaceIO::UserInterfaceIO(Parser* parser):
    tabSize(4), promptEnd("> ")
{
    completer = new CliCompleter(parser);
    reader = new ReadlineWrapper::Readline(".deska_cli_history", 64, completer);
}



UserInterfaceIO::~UserInterfaceIO()
{
    delete reader;
    delete completer;
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



void UserInterfaceIO::printHelp(const std::map<std::string, std::string> &cliCommands,
                                const std::map<std::string, std::string> &parserKeywords)
{
    std::cout << "CLI commands:" << std::endl;
    for (std::map<std::string, std::string>::const_iterator it = cliCommands.begin(); it != cliCommands.end(); ++it) {
        std::cout << it->first << "\t- ";
        std::vector<std::string> wrappedDscr = wrap(it->second, 70);
        for (std::vector<std::string>::iterator itd = wrappedDscr.begin(); itd != wrappedDscr.end(); ++itd) {
            if (itd != wrappedDscr.begin())
                std::cout << "\t  ";
            std::cout << *itd << std::endl;
        }
    }
    std::cout << std::endl;
    std::cout << "Parser keywords:" << std::endl;
    for (std::map<std::string, std::string>::const_iterator it = parserKeywords.begin(); it != parserKeywords.end(); ++it) {
        std::cout << it->first << "\t- ";
        std::vector<std::string> wrappedDscr = wrap(it->second, 70);
        for (std::vector<std::string>::iterator itd = wrappedDscr.begin(); itd != wrappedDscr.end(); ++itd) {
            if (itd != wrappedDscr.begin())
                std::cout << "\t  ";
            std::cout << *itd << std::endl;
        }
    }
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
    return reader->getLine(prompt + promptEnd);
}



void UserInterfaceIO::printAttributes(const std::vector<Db::AttributeDefinition> &attributes, int indentLevel,
                                      std::ostream &out)
{
    for (std::vector<Db::AttributeDefinition>::const_iterator it = attributes.begin(); it != attributes.end(); ++it) {
        printAttribute(*it, indentLevel, out);
    }
}



void UserInterfaceIO::printObjects(const std::vector<Db::ObjectDefinition> &objects, int indentLevel,
                                   bool fullName, std::ostream &out)
{
    for (std::vector<Db::ObjectDefinition>::const_iterator it = objects.begin(); it != objects.end(); ++it) {
        printObject(*it, indentLevel, fullName, out);
    }
}



void UserInterfaceIO::printAttribute(const Db::AttributeDefinition &attribute, int indentLevel, std::ostream &out)
{
    out << indent(indentLevel) << attribute << std::endl;
}



void UserInterfaceIO::printObject(const Db::ObjectDefinition &object, int indentLevel, bool fullName, std::ostream &out)
{
    if (fullName)
        out << indent(indentLevel) << object << std::endl;
    else
        out << indent(indentLevel)
                  << Db::ObjectDefinition(object.kind, Db::PathToVector(object.name).back()) << std::endl;
}



void UserInterfaceIO::printEnd(int indentLevel, std::ostream &out)
{
    out << indent(indentLevel) << "end" << std::endl;
}



void UserInterfaceIO::addCommandCompletion(const std::string &completion)
{
    completer->addCommandCompletion(completion);
}



std::string UserInterfaceIO::indent(unsigned int indentLevel)
{
    std::ostringstream ss;
    for (unsigned int i = 0; i < (indentLevel * tabSize); ++i) {
        ss << " ";
    }
    return ss.str();
}



std::vector<std::string> UserInterfaceIO::wrap(const std::string &text, unsigned int width)
{
    std::vector<std::string> lines;
    boost::char_separator<char> separators(" \t\n");
    boost::tokenizer<boost::char_separator<char> > tokenizer(text, separators);
    std::string word;
    std::string line;
    
    for (boost::tokenizer<boost::char_separator<char> >::const_iterator it = tokenizer.begin();
         it != tokenizer.end(); ++it) {
        word = *it;
        boost::algorithm::trim(word);
        if ((line.length() + word.length() + 1) > width) {
            lines.push_back(line);
            line.clear();
        }
        if (!line.empty())
            line += " ";
        line += word;
    }

    if (!line.empty())
        lines.push_back(line);

    return lines;
}



}
}
