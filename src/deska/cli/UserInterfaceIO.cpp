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
    std::vector<std::string> lines = wrap(errorMessage, 80);
    for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); ++it)
        std::cerr << *it << std::endl;
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



bool UserInterfaceIO::confirmRestoration(const Db::ObjectDefinition &object)
{
    std::ostringstream ss;
    ss << object << " was deleted in current changeset. Restore?";
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
    unsigned int maxWordWidth = 0;
    for (std::map<std::string, std::string>::const_iterator it = cliCommands.begin(); it != cliCommands.end(); ++it)
        maxWordWidth = ((maxWordWidth < it->first.length()) ? it->first.length() : maxWordWidth);
    for (std::map<std::string, std::string>::const_iterator it = parserKeywords.begin(); it != parserKeywords.end(); ++it)
        maxWordWidth = ((maxWordWidth < it->first.length()) ? it->first.length() : maxWordWidth);

    std::cout << "CLI commands:" << std::endl;
    for (std::map<std::string, std::string>::const_iterator it = cliCommands.begin(); it != cliCommands.end(); ++it) {
        std::cout << fixWidth(it->first, maxWordWidth) << " - ";
        std::vector<std::string> wrappedDscr = wrap(it->second, (80 - maxWordWidth - 3));
        for (std::vector<std::string>::iterator itd = wrappedDscr.begin(); itd != wrappedDscr.end(); ++itd) {
            if (itd != wrappedDscr.begin())
                std::cout << indent(maxWordWidth + 3, 1);
            std::cout << *itd << std::endl;
        }
    }
    std::cout << std::endl;
    std::cout << "Parser keywords:" << std::endl;
    for (std::map<std::string, std::string>::const_iterator it = parserKeywords.begin(); it != parserKeywords.end(); ++it) {
        std::cout << fixWidth(it->first, maxWordWidth) << " - ";
        std::vector<std::string> wrappedDscr = wrap(it->second, (80 - maxWordWidth - 3));
        for (std::vector<std::string>::iterator itd = wrappedDscr.begin(); itd != wrappedDscr.end(); ++itd) {
            if (itd != wrappedDscr.begin())
                std::cout << indent(maxWordWidth + 3, 1);
            std::cout << *itd << std::endl;
        }
    }
}



void UserInterfaceIO::printHelpCommand(const std::string &cmdName, const std::string &cmdDscr)
{
    std::cout << "Help for CLI command " << cmdName << ":" << std::endl;
    std::vector<std::string> wrappedDscr = wrap(cmdDscr, 78);
    for (std::vector<std::string>::iterator itd = wrappedDscr.begin(); itd != wrappedDscr.end(); ++itd) {
        std::cout << indent(2, 1) << *itd << std::endl;
    }
}



void UserInterfaceIO::printHelpKeyword(const std::string &keywordName, const std::string &keywordDscr)
{
    std::cout << "Help for Parser keyword " << keywordName << ":" << std::endl;
    std::vector<std::string> wrappedDscr = wrap(keywordDscr, 78);
    for (std::vector<std::string>::iterator itd = wrappedDscr.begin(); itd != wrappedDscr.end(); ++itd) {
        std::cout << indent(2, 1) << *itd << std::endl;
    }
}



void UserInterfaceIO::printHelpKind(const std::string &kindName,
                                    const std::vector<std::pair<std::string, std::string> > &kindAttrs,
                                    const std::vector<std::string> &nestedKinds)
{
    std::cout << "Content of " << kindName << ":" << std::endl;
    std::cout << indent(2, 1) << "Attributes:" << std::endl;
    if (kindAttrs.empty()) {
        std::cout << indent(4, 1) << "No attributes" << std::endl;
    } else {
        unsigned int maxWordWidth = 0;
        for (std::vector<std::pair<std::string, std::string> >::const_iterator it = kindAttrs.begin();
             it != kindAttrs.end(); ++it)
            maxWordWidth = ((maxWordWidth < it->first.length()) ? it->first.length() : maxWordWidth);
        for (std::vector<std::pair<std::string, std::string> >::const_iterator it = kindAttrs.begin();
             it != kindAttrs.end(); ++it)
            std::cout << indent(4, 1) << fixWidth(it->first, maxWordWidth) << " : " << it->second << std::endl;
    }
    std::cout << indent(2, 1) << "Nested kinds:" << std::endl;
    if (nestedKinds.empty()) {
        std::cout << indent(4, 1) << "No nested kinds" << std::endl;
    } else {
        for (std::vector<std::string>::const_iterator it = nestedKinds.begin(); it != nestedKinds.end(); ++it)
            std::cout << indent(4, 1) << *it << std::endl;
    }
}



void UserInterfaceIO::printHelpShowKinds(const std::vector<std::string> &kinds)
{
    std::cout << "Defined kinds:" << std::endl;
    for (std::vector<std::string>::const_iterator it = kinds.begin(); it != kinds.end(); ++it)
        std::cout << indent(2, 1) << *it << std::endl;
}



int UserInterfaceIO::chooseChangeset(const std::vector<Db::PendingChangeset> &pendingChangesets)
{
    std::cout << "Pending changesets:" << std::endl << std::endl;
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



std::string UserInterfaceIO::indent(unsigned int indentLevel, unsigned int tab)
{
    if (tab == 0)
        tab = tabSize;
    std::ostringstream ss;
    for (unsigned int i = 0; i < (indentLevel * tab); ++i) {
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



std::string UserInterfaceIO::fixWidth(const std::string &text, unsigned int width)
{
    if (text.length() < width)
        return (text + indent((width - text.length()), 1));
    else
        return text;
}



}
}
