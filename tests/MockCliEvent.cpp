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


#include <iostream>
#include "MockCliEvent.h"


MockCliEvent MockCliEvent::reportError(const std::string &errorMessage)
{
    MockCliEvent res(EVENT_REPORT_ERROR);
    res.str1 = errorMessage;
    return res;
}



MockCliEvent MockCliEvent::printMessage(const std::string &message)
{
    MockCliEvent res(EVENT_PRINT_MESSAGE);
    res.str1 = message;
    return res;
}



MockCliEvent MockCliEvent::confirmDeletion(const Deska::Db::ObjectDefinition &object)
{
    MockCliEvent res(EVENT_CONFIRM_DELETION);
    res.object = object;
    return res;
}



MockCliEvent MockCliEvent::returnConfirmDeletion(bool confirm)
{
    MockCliEvent res(RETURN_CONFIRM_DELETION);
    res.boolean = confirm;
    return res;
}



MockCliEvent MockCliEvent::confirmCreation(const Deska::Db::ObjectDefinition &object)
{
    MockCliEvent res(EVENT_CONFIRM_CREATION);
    res.object = object;
    return res;
}



MockCliEvent MockCliEvent::returnConfirmCreation(bool confirm)
{
    MockCliEvent res(RETURN_CONFIRM_CREATION);
    res.boolean = confirm;
    return res;
}



MockCliEvent MockCliEvent::confirmRestoration(const Deska::Db::ObjectDefinition &object)
{
    MockCliEvent res(EVENT_CONFIRM_RESTORATION);
    res.object = object;
    return res;
}



MockCliEvent MockCliEvent::returnConfirmRestoration(bool confirm)
{
    MockCliEvent res(RETURN_CONFIRM_RESTORATION);
    res.boolean = confirm;
    return res;
}



MockCliEvent MockCliEvent::askForCommitMessage()
{
    return MockCliEvent(EVENT_ASK_FOR_COMMIT_MESSAGE);
}



MockCliEvent MockCliEvent::returnAskForCommitMessage(const std::string &message)
{
    MockCliEvent res(RETURN_ASK_FOR_COMMIT_MESSAGE);
    res.str1 = message;
    return res;
}



MockCliEvent MockCliEvent::askForDetachMessage()
{
    return MockCliEvent(EVENT_ASK_FOR_DETACH_MESSAGE);
}



MockCliEvent MockCliEvent::returnAskForDetachMessage(const std::string &message)
{
    MockCliEvent res(RETURN_ASK_FOR_DETACH_MESSAGE);
    res.str1 = message;
    return res;
}



MockCliEvent MockCliEvent::printHelp(const std::map<std::string, std::string> &cliCommands,
                                     const std::map<std::string, std::string> &parserKeywords)
{
    MockCliEvent res(EVENT_PRINT_HELP);
    res.map1 = cliCommands;
    res.map2 = parserKeywords;
    return res;
}



MockCliEvent MockCliEvent::printHelpCommand(const std::string &cmdName, const std::string &cmdDscr)
{
    MockCliEvent res(EVENT_PRINT_HELP_COMMAND);
    res.str1 = cmdName;
    res.str2 = cmdDscr;
    return res;
}



MockCliEvent MockCliEvent::printHelpKeyword(const std::string &keywordName, const std::string &keywordDscr)
{
    MockCliEvent res(EVENT_PRINT_HELP_KEYWORD);
    res.str1 = keywordName;
    res.str2 = keywordDscr;
    return res;
}



MockCliEvent MockCliEvent::printHelpKind(const std::string &kindName,
                                         const std::vector<std::pair<std::string, std::string> > &kindAttrs,
                                         const std::vector<std::string> &nestedKinds)
{
    MockCliEvent res(EVENT_PRINT_HELP_KIND);
    res.str1 = kindName;
    res.vectpair = kindAttrs;
    res.vect = nestedKinds;
    return res;
}



MockCliEvent MockCliEvent::printHelpShowKinds(const std::vector<std::string> &kinds)
{
    MockCliEvent res(EVENT_PRINT_HELP_SHOW_KINDS);
    res.vect = kinds;
    return res;
}



MockCliEvent MockCliEvent::chooseChangeset(const std::vector<Deska::Db::PendingChangeset> &pendingChangesets)
{
    MockCliEvent res(EVENT_CHOOSE_CHANGESET);
    res.changesets = pendingChangesets;
    return res;
}



MockCliEvent MockCliEvent::returnChooseChangeset(int changeset)
{
    MockCliEvent res(RETURN_CHOOSE_CHANGESET);
    res.integer = changeset;
    return res;
}



MockCliEvent MockCliEvent::readLine(const std::string &prompt)
{
    MockCliEvent res(EVENT_READ_LINE);
    res.str1 = prompt;
    return res;
}



MockCliEvent MockCliEvent::returnReadLine(const std::string &line)
{
    MockCliEvent res(RETURN_READ_LINE);
    res.str1 = line;
    return res;
}



MockCliEvent MockCliEvent::printAttributes(const std::vector<Deska::Db::AttributeDefinition> &attributes, int indentLevel,
                                           std::ostream &out)
{
    MockCliEvent res(EVENT_PRINT_ATTRIBUTES);
    res.attrs = attributes;
    res.integer = indentLevel;
    return res;
}



MockCliEvent MockCliEvent::printAttribute(const Deska::Db::AttributeDefinition &attribute, int indentLevel,
                                          std::ostream &out)
{
    MockCliEvent res(EVENT_PRINT_ATTRIBUTE);
    res.attr = attribute;
    res.integer = indentLevel;
    return res;
}                                  



MockCliEvent MockCliEvent::printObjects(const std::vector<Deska::Db::ObjectDefinition> &objects, int indentLevel,
                                        bool fullName, std::ostream &out)
{
    MockCliEvent res(EVENT_PRINT_OBJECTS);
    res.objects = objects;
    res.integer = indentLevel;
    return res;
}



MockCliEvent MockCliEvent::printObject(const Deska::Db::ObjectDefinition &object, int indentLevel, bool fullName,
                                       std::ostream &out)
{
    MockCliEvent res(EVENT_PRINT_OBJECT);
    res.object = object;
    res.integer = indentLevel;
    return res;
}



MockCliEvent MockCliEvent::printEnd(int indentLevel, std::ostream &out)
{
    MockCliEvent res(EVENT_PRINT_END);
    res.integer = indentLevel;
    return res;
}



MockCliEvent MockCliEvent::addCommandCompletion(const std::string &completion)
{
    MockCliEvent res(EVENT_ADD_COMMAND_COMPLETION);
    res.str1 = completion;
    return res;
}



MockCliEvent MockCliEvent::invalid()
{
    return MockCliEvent(EVENT_INVALID);
}



bool MockCliEvent::inputEvent(const MockCliEvent &m) const
{
    return ((m.eventKind == RETURN_CONFIRM_DELETION) || (m.eventKind == RETURN_CONFIRM_CREATION) ||
            (m.eventKind == RETURN_CONFIRM_RESTORATION) || (m.eventKind == RETURN_ASK_FOR_COMMIT_MESSAGE) ||
            (m.eventKind == RETURN_ASK_FOR_DETACH_MESSAGE) || (m.eventKind == RETURN_CHOOSE_CHANGESET) ||
            (m.eventKind == RETURN_READ_LINE));
}



bool MockCliEvent::outputEvent(const MockCliEvent &m) const
{
    return !(inputEvent(m));
}



bool MockCliEvent::myReturn(const MockCliEvent &other) const
{
    // Note that we're comparing EVENT_* against a matching RETURN_*
    switch (eventKind) {
    case EVENT_CONFIRM_DELETION:
        return other.eventKind == RETURN_CONFIRM_DELETION;
    case EVENT_CONFIRM_CREATION:
        return other.eventKind == RETURN_CONFIRM_CREATION;
    case EVENT_CONFIRM_RESTORATION:
        return other.eventKind == RETURN_CONFIRM_RESTORATION;
    case EVENT_ASK_FOR_COMMIT_MESSAGE:
        return other.eventKind == RETURN_ASK_FOR_COMMIT_MESSAGE;
    case EVENT_ASK_FOR_DETACH_MESSAGE:
        return other.eventKind == RETURN_ASK_FOR_DETACH_MESSAGE;
    case EVENT_CHOOSE_CHANGESET:
        return other.eventKind == RETURN_CHOOSE_CHANGESET;
    case EVENT_READ_LINE:
        return other.eventKind == RETURN_READ_LINE;
    default:
        return false;
    } 
}



bool MockCliEvent::operator==(const MockCliEvent &other) const
{
    return eventKind == other.eventKind && str1 == other.str1 && str2 == other.str2 && integer == other.integer &&
           boolean == other.boolean && object == other.object && attr == other.attr &&
           std::equal(map1.begin(), map1.end(), other.map1.begin()) &&
           std::equal(map2.begin(), map2.end(), other.map2.begin()) &&
           std::equal(vectpair.begin(), vectpair.end(), other.vectpair.begin()) &&
           std::equal(vect.begin(), vect.end(), other.vect.begin()) &&
           std::equal(changesets.begin(), changesets.end(), other.changesets.begin()) &&
           std::equal(attrs.begin(), attrs.end(), other.attrs.begin()) &&
           std::equal(objects.begin(), objects.end(), other.objects.begin());
}



MockCliEvent::MockCliEvent(Event e): eventKind(e), integer(int()), boolean(bool())
{
}



std::ostream& operator<<(std::ostream &out, const std::map<std::string, std::string> &m)
{
    out << "[";
    for (std::map<std::string, std::string>::const_iterator it = m.begin(); it != m.end(); ++it) {
        if (it != m.begin())
            out << ", ";
        out << "\"" << it->first << "\":\"" << it->second << "\"";
    }
    out << "]";
    return out;
}



std::ostream& operator<<(std::ostream &out, const std::vector<std::pair<std::string, std::string> > &v)
{
    out << "[";
    for (std::vector<std::pair<std::string, std::string> >::const_iterator it = v.begin(); it != v.end(); ++it) {
        if (it != v.begin())
            out << ", ";
        out << "\"" << it->first << "\":\"" << it->second << "\"";
    }
    out << "]";
    return out;
}



std::ostream& operator<<(std::ostream &out, const std::vector<std::string> &v)
{
    out << "[";
    for (std::vector<std::string>::const_iterator it = v.begin(); it != v.end(); ++it) {
        if (it != v.begin())
            out << ", ";
        out << "\"" << *it << "\"";
    }
    out << "]";
    return out;
}



template <typename T>
std::ostream& operator<<(std::ostream &out, const std::vector<T> &v)
{
    out << "[";
    for (typename std::vector<T>::const_iterator it = v.begin(); it != v.end(); ++it) {
        if (it != v.begin())
            out << ", ";
        out << *it;
    }
    out << "]";
    return out;
}



std::ostream& operator<<(std::ostream &out, const MockCliEvent &m)
{
    switch (m.eventKind) {
    case MockCliEvent::EVENT_REPORT_ERROR:
        out << "reportError( \"" << m.str1 << "\" )";
        break;
    case MockCliEvent::EVENT_PRINT_MESSAGE:
        out << "printMessage( \"" << m.str1 << "\" )";
        break;
    case MockCliEvent::EVENT_CONFIRM_DELETION:
        out << "confirmDeletion( " << *(m.object) << " )";
        break;
    case MockCliEvent::RETURN_CONFIRM_DELETION:
        out << "returnConfirmDeletion( " << m.boolean << " )";
        break;
    case MockCliEvent::EVENT_CONFIRM_CREATION:
        out << "confirmCreation( " << *(m.object) << " )";
        break;
    case MockCliEvent::RETURN_CONFIRM_CREATION:
        out << "returnConfirmCreation( " << m.boolean << " )";
        break;
    case MockCliEvent::EVENT_CONFIRM_RESTORATION:
        out << "confirmRestoration( " << *(m.object) << " )";
        break;
    case MockCliEvent::RETURN_CONFIRM_RESTORATION:
        out << "returnConfirmRestoration( " << m.boolean << " )";
        break;
    case MockCliEvent::EVENT_ASK_FOR_COMMIT_MESSAGE:
        out << "askForCommitMessage()";
        break;
    case MockCliEvent::RETURN_ASK_FOR_COMMIT_MESSAGE:
        out << "returnAskForCommitMessage( \"" << m.str1 << "\" )";
        break;
    case MockCliEvent::EVENT_ASK_FOR_DETACH_MESSAGE:
        out << "askForDetachMessage()";
        break;
    case MockCliEvent::RETURN_ASK_FOR_DETACH_MESSAGE:
        out << "returnAskForDetachMessage( \"" << m.str1 << "\" )";
        break;
    case MockCliEvent::EVENT_PRINT_HELP:
        out << "printHelp( " << m.map1 << ", " << m.map2 << " )";
        break;
    case MockCliEvent::EVENT_PRINT_HELP_COMMAND:
        out << "printHelpCommand( \"" << m.str1 << "\", \"" << m.str2 << "\" )";
        break;
    case MockCliEvent::EVENT_PRINT_HELP_KEYWORD:
        out << "printHelpKeyword( \"" << m.str1 << "\", \"" << m.str2 << "\" )";
        break;
    case MockCliEvent::EVENT_PRINT_HELP_KIND:
        out << "printHelpKind( \"" << m.str1 << "\", " << m.vectpair << ", " << m.vect << " )";
        break;
    case MockCliEvent::EVENT_PRINT_HELP_SHOW_KINDS:
        out << "printHelpShowKinds( " << m.vect << " )";
        break;
    case MockCliEvent::EVENT_CHOOSE_CHANGESET:
        out << "chooseChangeset( " << m.changesets << " )";
        break;
    case MockCliEvent::RETURN_CHOOSE_CHANGESET:
        out << "returnChooseChangeset( " << m.integer << " )";
        break;
    case MockCliEvent::EVENT_READ_LINE:
        out << "readLine( \"" << m.str1 << "\" )";
        break;
    case MockCliEvent::RETURN_READ_LINE:
        out << "returnReadLine( \"" << m.str1 << "\" )";
        break;
    case MockCliEvent::EVENT_PRINT_ATTRIBUTES:
        out << "printAttributes( " << m.attrs << ", " << m.integer << " )";
        break;
    case MockCliEvent::EVENT_PRINT_ATTRIBUTE:
        out << "printAttribute( " << *(m.attr) << ", " << m.integer << " )";
        break;
    case MockCliEvent::EVENT_PRINT_OBJECTS:
        out << "printObjects( " << m.objects << ", " << m.integer << " )";
        break;
    case MockCliEvent::EVENT_PRINT_OBJECT:
        out << "printObject( " << *(m.object) << ", " << m.integer << " )";
        break;
    case MockCliEvent::EVENT_PRINT_END:
        out << "printEnd( " << m.integer << " )";
        break;
    case MockCliEvent::EVENT_ADD_COMMAND_COMPLETION:
        out << "addCommandCompletion( \"" << m.str1 << "\" )";
        break;
    case MockCliEvent::EVENT_INVALID:
        out << "[no event]";
        break;
    }
    return out;
}