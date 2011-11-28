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
#include <iomanip>
#include <cmath>
#include <cstdlib>

#include <boost/algorithm/string/trim.hpp>
#include <boost/tokenizer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "ChildProcess.h"
#include "CliCommands.h"
#include "CliConfig.h"
#include "ContextStack.h"
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
    while ((*space != ' ') && (*space != '\t') && (*space != '\n') && (*space != '(') && (*space != ')')) {
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



struct ModificationInfo
{
    /** @short Type of object modification for printing purposes */
    typedef enum {
        /** @short Db::CreateObjectModification */
        OBJECT_MODIFICATION_TYPE_CREATE = 2,
        /** @short Db::DeleteObjectModification */
        OBJECT_MODIFICATION_TYPE_DELETE = 0,
        /** @short Db::RenameObjectModification */
        OBJECT_MODIFICATION_TYPE_RENAME = 1,
        /** @short Db::SetAttributeModification */
        OBJECT_MODIFICATION_TYPE_SETATTR = 3
    } ObjectModificationType;

    ModificationInfo(ObjectModificationType type, const ObjectDefinition &object):
        modificationType(type), modifiedObject(object) {};

    ObjectModificationType modificationType;
    ObjectDefinition modifiedObject;
};



/** @short Visitor for printing object modifications. */
struct ModificationPrinter: public boost::static_visitor<std::string> {

    ModificationPrinter(const boost::optional<ModificationInfo> &prevObj,
                        const boost::optional<ModificationInfo> &nextObj,
                        bool &created, unsigned int &depth, unsigned int tab);
    //@{
    /** @short Function for printing single object modification.
    *
    *   @param modification Instance of modifications from Db::ObjectModification variant.
    */
    std::string operator()(const Db::CreateObjectModification &modification) const;
    std::string operator()(const Db::DeleteObjectModification &modification) const;
    std::string operator()(const Db::RenameObjectModification &modification) const;
    std::string operator()(const Db::SetAttributeModification &modification) const;
    //@}

private:

    /** @short Constructs string for indenting an output.
    *
    *   @param indentLevel Level of indentation (number of "tabs")
    *   @return String constructed from spaces for indenting
    */
    std::string indent(unsigned int indentLevel) const;

    boost::optional<ModificationInfo> prevObject;
    boost::optional<ModificationInfo> nextObject;
    bool &createdObj;
    unsigned int &printDepth;
    unsigned int tabSize;
};



/** @short Visitor for extracting object from modifications. */
struct ModificationObjectExtractor: public boost::static_visitor<ModificationInfo> {
    //@{
    /** @short Function for extracting object from single object modification.
    *
    *   @param modification Instance of modifications from Db::ObjectModification variant.
    */
    ModificationInfo operator()(const Db::CreateObjectModification &modification) const;
    ModificationInfo operator()(const Db::DeleteObjectModification &modification) const;
    ModificationInfo operator()(const Db::RenameObjectModification &modification) const;
    ModificationInfo operator()(const Db::SetAttributeModification &modification) const;
    //@}
};



/** @short Function for comparing two modification for purposes of sorting modifications list for diff output. */
bool modificationDiffComparatorLess(const Db::ObjectModificationResult &ma, const Db::ObjectModificationResult &mb)
{
    ModificationInfo a = boost::apply_visitor(ModificationObjectExtractor(), ma);
    ModificationInfo b = boost::apply_visitor(ModificationObjectExtractor(), mb);
    if (a.modifiedObject.kind < b.modifiedObject.kind) {
        return true;
    } else if (a.modifiedObject.kind > b.modifiedObject.kind) {
        return false;
    } else if (a.modifiedObject.name < b.modifiedObject.name) {
        return true;
    } else if (a.modifiedObject.name > b.modifiedObject.name) {
        return false;
    } else {
        return (a.modificationType < b.modificationType);
    }
}



ModificationPrinter::ModificationPrinter(const boost::optional<ModificationInfo> &prevObj,
                                         const boost::optional<ModificationInfo> &nextObj,
                                         bool &created, unsigned int &depth, unsigned int tab):
    prevObject(prevObj), nextObject(nextObj), createdObj(created), printDepth(depth), tabSize(tab)
{
};



std::string ModificationPrinter::operator()(const Db::CreateObjectModification &modification) const
{
    std::ostringstream ostr;
    ostr << "\e[32m+ " << indent(printDepth) << modification.kindName << " " << modification.objectName
         << "\e[0m" << std::endl;
    if ((nextObject) && (nextObject->modifiedObject == ObjectDefinition(modification.kindName, modification.objectName))) {
        ++printDepth;
        createdObj = true;
    } else {
        ostr << "\e[32m+ " << indent(printDepth) << "end\e[0m" << std::endl;
        createdObj = false;
    }
    return ostr.str();
}



std::string ModificationPrinter::operator()(const Db::DeleteObjectModification &modification) const
{
    std::ostringstream ostr;
    ostr << "\e[31m- " << indent(printDepth) << modification.kindName << " " << modification.objectName
         << "\e[0m" << std::endl;
    ostr << "\e[31m- " << indent(printDepth) << "end\e[0m" << std::endl;
    createdObj = false;
    return ostr.str();
}



std::string ModificationPrinter::operator()(const Db::RenameObjectModification &modification) const
{
    std::ostringstream ostr;
    ostr << "\e[31m- " << indent(printDepth) << modification.kindName << " " << modification.oldObjectName
         << "\e[0m" << std::endl;
    ostr << "\e[32m+ " << indent(printDepth) << modification.kindName << " " << modification.newObjectName
         << "\e[0m" << std::endl;
    if ((nextObject) && (nextObject->modifiedObject == ObjectDefinition(modification.kindName, modification.newObjectName)))
        ++printDepth;
    else
        ostr << indent(printDepth) << "end" << std::endl;
    createdObj = false;
    return ostr.str();
}



std::string ModificationPrinter::operator()(const Db::SetAttributeModification &modification) const
{
    std::ostringstream ostr;
    if ((!prevObject) || (prevObject->modifiedObject != ObjectDefinition(modification.kindName, modification.objectName))) {
        ostr << indent(printDepth) << modification.kindName << " " << modification.objectName << std::endl;
        ++printDepth;
    }

    if (modification.oldAttributeData)
        ostr << "\e[31m- " << indent(printDepth) << modification.attributeName << " "
             << boost::apply_visitor(NonOptionalValuePrettyPrint(), *(modification.oldAttributeData))
             << "\e[0m" << std::endl;
    if (modification.attributeData)
        ostr << "\e[32m+ " << indent(printDepth) << modification.attributeName << " "
             << boost::apply_visitor(NonOptionalValuePrettyPrint(), *(modification.attributeData))
             << "\e[0m" << std::endl;

    if ((!nextObject) || (nextObject->modifiedObject != ObjectDefinition(modification.kindName, modification.objectName))) {
        --printDepth;
        if (createdObj)
            ostr << "\e[32m+ " << indent(printDepth) << "end\e[0m" << std::endl;
        else
            ostr << indent(printDepth) << "end" << std::endl;
        createdObj = false;
    }
    return ostr.str();
}



std::string ModificationPrinter::indent(unsigned int indentLevel) const
{
    std::ostringstream ss;
    for (unsigned int i = 0; i < (indentLevel * tabSize); ++i) {
        ss << " ";
    }
    return ss.str();
}



ModificationInfo ModificationObjectExtractor::operator()(const Db::CreateObjectModification &modification) const
{
    return ModificationInfo(ModificationInfo::OBJECT_MODIFICATION_TYPE_CREATE, ObjectDefinition(modification.kindName, modification.objectName));
}



ModificationInfo ModificationObjectExtractor::operator()(const Db::DeleteObjectModification &modification) const
{
    return ModificationInfo(ModificationInfo::OBJECT_MODIFICATION_TYPE_DELETE, ObjectDefinition(modification.kindName, modification.objectName));
}



ModificationInfo ModificationObjectExtractor::operator()(const Db::RenameObjectModification &modification) const
{
    return ModificationInfo(ModificationInfo::OBJECT_MODIFICATION_TYPE_RENAME, ObjectDefinition(modification.kindName, modification.newObjectName));
}



ModificationInfo ModificationObjectExtractor::operator()(const Db::SetAttributeModification &modification) const
{
    return ModificationInfo(ModificationInfo::OBJECT_MODIFICATION_TYPE_SETATTR, ObjectDefinition(modification.kindName, modification.objectName));
}



UserInterfaceIO::UserInterfaceIO()
{
}



UserInterfaceIO::UserInterfaceIO(Parser* parser, CliConfig* _config):
    tabSize(4), promptEnd("> "), config(_config), lineWidth(config->getVar<unsigned int>(CLI_LineWidth))
{
    completer = new CliCompleter(parser);
    reader = new ReadlineWrapper::Readline(config->getVar<std::string>(CLI_HistoryFilename),
                                           config->getVar<unsigned int>(CLI_HistoryLimit), completer);
}



UserInterfaceIO::~UserInterfaceIO()
{
    delete reader;
    delete completer;
}



void UserInterfaceIO::reportError(const std::string &errorMessage)
{
    std::vector<std::string> lines = wrap(errorMessage, 0);
    for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); ++it)
        std::cerr << *it << std::endl;
}



void UserInterfaceIO::printMessage(const std::string &message)
{
    std::vector<std::string> lines = wrap(message, 0);
    for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); ++it)
        std::cout << *it << std::endl;
}



void UserInterfaceIO::displayInPager(const std::string &message)
{
    Pager pager(message);
}



void UserInterfaceIO::editFile(const std::string &fileName)
{
    Editor editor(fileName);
}



bool UserInterfaceIO::confirmDeletion(const ObjectDefinition &object)
{
    std::ostringstream ss;
    ss << "Are you sure you want to delete object(s) " << object << "?";
    return askForConfirmationImpl(ss.str());
}



bool UserInterfaceIO::confirmDeletionNested(const std::vector<ObjectDefinition> &nestedObjects)
{
    std::ostringstream ss;
    ss << "Object has following nested object(s): ";
    for (std::vector<ObjectDefinition>::const_iterator it = nestedObjects.begin(); it != nestedObjects.end(); ++it) {
        if (it != nestedObjects.begin())
            ss << ", ";
        ss << *it;
    }
    ss << ". Delete anyway?";
    return askForConfirmationImpl(ss.str());
}



bool UserInterfaceIO::confirmDeletionConnected(const std::vector<ObjectDefinition> &connectedObjects)
{
    std::ostringstream ss;
    ss << "Do you wand to delete also connected object(s) ";
    for (std::vector<ObjectDefinition>::const_iterator it = connectedObjects.begin(); it != connectedObjects.end(); ++it) {
        if (it != connectedObjects.begin())
            ss << ", ";
        ss << *it;
    }
    ss << "?";
    return askForConfirmationImpl(ss.str());
}



bool UserInterfaceIO::confirmRenameConnected(const std::vector<ObjectDefinition> &connectedObjects)
{
    std::ostringstream ss;
    ss << "Do you wand to rename also connected object(s) ";
    for (std::vector<ObjectDefinition>::const_iterator it = connectedObjects.begin(); it != connectedObjects.end(); ++it) {
        if (it != connectedObjects.begin())
            ss << ", ";
        ss << *it;
    }
    ss << "?";
    return askForConfirmationImpl(ss.str());
}



bool UserInterfaceIO::confirmCreation(const ObjectDefinition &object)
{
    std::ostringstream ss;
    ss << "Object(s) " << object << " do(es) not exist. Create?";
    return askForConfirmationImpl(ss.str());
}



bool UserInterfaceIO::confirmCreationConnection(const ObjectDefinition &object)
{
    std::ostringstream ss;
    ss << "Object(s) " << object << " do(es) not exist. Create and connect?";
    return askForConfirmationImpl(ss.str());
}



bool UserInterfaceIO::confirmCreationConnection(const ObjectDefinition &object,
                                                const std::vector<ObjectDefinition> &connectedObjects)
{
    std::ostringstream ss;
    ss << "Object(s) " << object << " do(es) not exist. Create and connect to ";
    for (std::vector<ObjectDefinition>::const_iterator it = connectedObjects.begin(); it != connectedObjects.end(); ++it) {
        if (it != connectedObjects.begin())
            ss << ", ";
        ss << *it;
    }
    ss << "?";
    return askForConfirmationImpl(ss.str());
}



bool UserInterfaceIO::confirmRestoration(const ObjectDefinition &object)
{
    std::ostringstream ss;
    ss << "Object(s) " << object << " was/were deleted in current changeset. Restore?";
    return askForConfirmationImpl(ss.str());
}



bool UserInterfaceIO::askForConfirmation(const std::string &prompt)
{
    return askForConfirmationImpl(prompt);
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
    std::string::size_type maxWordWidth = 0;
    for (std::map<std::string, std::string>::const_iterator it = cliCommands.begin(); it != cliCommands.end(); ++it)
        maxWordWidth = std::max(maxWordWidth, it->first.length());
    for (std::map<std::string, std::string>::const_iterator it = parserKeywords.begin(); it != parserKeywords.end(); ++it)
        maxWordWidth = std::max(maxWordWidth, it->first.length());

    std::cout << "CLI commands:" << std::endl;
    for (std::map<std::string, std::string>::const_iterator it = cliCommands.begin(); it != cliCommands.end(); ++it) {
        std::cout << std::left << std::setw(maxWordWidth) << it->first << " - ";
        std::vector<std::string> wrappedDscr = wrap(it->second, (maxWordWidth + 3));
        for (std::vector<std::string>::iterator itd = wrappedDscr.begin(); itd != wrappedDscr.end(); ++itd) {
            if (itd != wrappedDscr.begin())
                std::cout << indent(maxWordWidth + 3, 1);
            std::cout << *itd << std::endl;
        }
    }
    std::cout << std::endl;
    std::cout << "Parser keywords:" << std::endl;
    for (std::map<std::string, std::string>::const_iterator it = parserKeywords.begin(); it != parserKeywords.end(); ++it) {
        std::cout << std::left << std::setw(maxWordWidth) << it->first << " - ";
        std::vector<std::string> wrappedDscr = wrap(it->second, (maxWordWidth + 3));
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
    std::vector<std::string> wrappedDscr = wrap(cmdDscr, 2);
    for (std::vector<std::string>::iterator itd = wrappedDscr.begin(); itd != wrappedDscr.end(); ++itd) {
        std::cout << indent(2, 1) << *itd << std::endl;
    }
}



void UserInterfaceIO::printHelpKeyword(const std::string &keywordName, const std::string &keywordDscr)
{
    std::cout << "Help for Parser keyword " << keywordName << ":" << std::endl;
    std::vector<std::string> wrappedDscr = wrap(keywordDscr, 2);
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
        std::string::size_type maxWordWidth = 0;
        for (std::vector<std::pair<std::string, std::string> >::const_iterator it = kindAttrs.begin();
             it != kindAttrs.end(); ++it)
             maxWordWidth = std::max(maxWordWidth, it->first.length());
        for (std::vector<std::pair<std::string, std::string> >::const_iterator it = kindAttrs.begin();
             it != kindAttrs.end(); ++it)
            std::cout << indent(4, 1) << std::left << std::setw(maxWordWidth) << it->first << " : " << it->second << std::endl;
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
        // Printing list of pending changesets
        unsigned int maxChWidth = 12;
        std::string::size_type maxUserWidth = 13;
        unsigned int maxNoWidth = 2;
        unsigned int log = digits(pendingChangesets.size() - 1);
        maxNoWidth = std::max(maxNoWidth, log);
        for (std::vector<Db::PendingChangeset>::const_iterator it = pendingChangesets.begin();
             it != pendingChangesets.end(); ++it) {
            log = (digits(it->revision.t) + 3);
            maxChWidth = std::max(maxChWidth, log);
            maxUserWidth = std::max(maxUserWidth, it->author.size());
        }
        std::cout << std::left << std::setw(maxNoWidth) << "No" << ": " << std::left
                  << std::setw(maxChWidth) << "Changeset ID" << " | " << std::left
                  << std::setw(maxUserWidth) << "Author" << " | " << "Time stamp                 " << " | "
                  << "Parent revision" << std::endl << indent(maxNoWidth + 2 + maxChWidth, 1) << " | "
                  << "Attach status" << " | " << "Connection info" << std::endl
                  << indent(maxNoWidth + 2 + maxChWidth, 1) << " | " << "Detach message" << std::endl;
        std::cout << "================================================================================" << std::endl;
        for (unsigned int i = 0; i < pendingChangesets.size(); ++i) {
            std::cout << std::left << std::setw(maxNoWidth) << i << ": " << pendingChangesets[i].revision
                  << indent(maxChWidth - (digits(pendingChangesets[i].revision.t) + 3), 1) << " | " << std::left
                  << std::setw(maxUserWidth) << pendingChangesets[i].author << " | " << pendingChangesets[i].timestamp
                  << " | " << pendingChangesets[i].parentRevision << std::endl << indent(maxNoWidth + 2 + maxChWidth, 1)
                  << " | " << std::left << std::setw(maxUserWidth) << pendingChangesets[i].attachStatus << " | "
                  << (!pendingChangesets[i].activeConnectionInfo ? std::string("No info") :
                      *(pendingChangesets[i].activeConnectionInfo)) << std::endl
                  << indent(maxNoWidth + 2 + maxChWidth, 1) << " | " << pendingChangesets[i].message << std::endl;
            std::cout << "--------------------------------------------------------------------------------" << std::endl;
        }
        std::cout << std::left << std::setw(maxNoWidth) << "n" << ": No changset" << std::endl << std::endl;
    }
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



std::pair<std::string, bool> UserInterfaceIO::readLine(const std::string &prompt)
{
    return reader->getLine(prompt + promptEnd);
}



void UserInterfaceIO::printAttributes(const std::vector<AttributeDefinition> &attributes, int indentLevel,
                                      std::ostream &out)
{
    for (std::vector<AttributeDefinition>::const_iterator it = attributes.begin(); it != attributes.end(); ++it) {
        printAttribute(*it, indentLevel, out);
    }
}



void UserInterfaceIO::printAttributesWithOrigin(
    const std::vector<std::pair<AttributeDefinition, Db::Identifier> > &attributes, int indentLevel, std::ostream &out)
{
    for (std::vector<std::pair<AttributeDefinition, Db::Identifier> >::const_iterator it = attributes.begin();
         it != attributes.end(); ++it) {
        printAttributeWithOrigin(it->first, it->second, indentLevel, out);
    }
}



void UserInterfaceIO::printObjects(const std::vector<ObjectDefinition> &objects, int indentLevel,
                                   bool fullName, std::ostream &out)
{
    for (std::vector<ObjectDefinition>::const_iterator it = objects.begin(); it != objects.end(); ++it) {
        printObject(*it, indentLevel, fullName, out);
    }
}



void UserInterfaceIO::printAttribute(const AttributeDefinition &attribute, int indentLevel, std::ostream &out)
{
    out << indent(indentLevel) << attribute << std::endl;
}



void UserInterfaceIO::printAttributeWithOrigin(const AttributeDefinition &attribute, const Db::Identifier &origin,
                                               int indentLevel, std::ostream &out)
{
    if (!origin.empty())
        out << indent(indentLevel) << attribute << " -> " << origin << std::endl;
    else
        out << indent(indentLevel) << attribute << std::endl;
}



void UserInterfaceIO::printObject(const ObjectDefinition &object, int indentLevel, bool fullName, std::ostream &out)
{
    if (fullName)
        out << indent(indentLevel) << object << std::endl;
    else
        out << indent(indentLevel)
            << ObjectDefinition(object.kind, pathToVector(object.name).back()) << std::endl;
}



void UserInterfaceIO::printEnd(int indentLevel, std::ostream &out)
{
    out << indent(indentLevel) << "end" << std::endl;
}



void UserInterfaceIO::printRevisions(const std::vector<Db::RevisionMetadata> &revisions)
{
    std::cout << "Revisions:" << std::endl << std::endl;
    if (revisions.empty()) {
        std::cout << "No revisions." << std::endl;
    } else {
        unsigned int maxRevWidth = 8;
        std::string::size_type maxUserWidth = 6;
        for (std::vector<Db::RevisionMetadata>::const_iterator it = revisions.begin(); it != revisions.end(); ++it) {
            unsigned int log = (digits(it->revision.r) + 1);
            maxRevWidth = std::max(maxRevWidth, log);
            maxUserWidth = std::max(maxUserWidth, it->author.size());
        }
        std::cout << std::left << std::setw(maxRevWidth) << "Revision" << " | " << std::left
                  << std::setw(maxUserWidth) << "Author" << " | " << "Time stamp                 " << std::endl
                  << indent(maxRevWidth, 1) << " | " << "Commit message" << std::endl;
        std::cout << "================================================================================" << std::endl;
        for (std::vector<Db::RevisionMetadata>::const_iterator it = revisions.begin(); it != revisions.end(); ++it) {
            if (it != revisions.begin())
                std::cout << "--------------------------------------------------------------------------------" << std::endl;
            std::cout << it->revision << indent(maxRevWidth - (digits(it->revision.r) + 1), 1) << " | "
                      << std::left << std::setw(maxUserWidth) << it->author << " | " << it->timestamp << std::endl
                      << indent(maxRevWidth, 1) << " | " << it->commitMessage << std::endl;
        }
    }
}



void UserInterfaceIO::printDiff(const std::vector<Db::ObjectModificationResult> &modifications)
{
    if (modifications.empty()) {
        std::cout << "No difference." << std::endl;
    } else {
        std::vector<Db::ObjectModificationResult> sortedModifications = modifications;
        std::sort(sortedModifications.begin(), sortedModifications.end(), modificationDiffComparatorLess);
        ModificationObjectExtractor modificationObjectExtractor;
        boost::optional<ModificationInfo> prevObj;
        boost::optional<ModificationInfo> nextObj;
        bool created = false;
        unsigned int depth = 0;
        for (std::vector<Db::ObjectModificationResult>::const_iterator it = sortedModifications.begin();
             it != sortedModifications.end(); ++it) {
            if (it != sortedModifications.begin())
                prevObj = boost::apply_visitor(modificationObjectExtractor, *(it - 1));
            if ((it + 1) != sortedModifications.end())
                nextObj = boost::apply_visitor(modificationObjectExtractor, *(it + 1));
            else
                nextObj = boost::optional<ModificationInfo>();
            std::cout << boost::apply_visitor(ModificationPrinter(prevObj, nextObj, created, depth, tabSize), *it);
        }
    }
}



void UserInterfaceIO::addCommandCompletion(const std::string &completion)
{
    completer->addCommandCompletion(completion);
}



bool UserInterfaceIO::askForConfirmationImpl(const std::string &prompt)
{
    std::cout << prompt << " ";
    std::string answer;
    getline(std::cin, answer);
    boost::algorithm::to_lower(answer);
    return answer == "yes" || answer == "y";
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



unsigned int UserInterfaceIO::digits(unsigned int n)
{
    if ((n == 0) || (n == 1))
        return 1;
    return static_cast<unsigned int>(std::floor(std::log10(n)) + 1);
}



std::vector<std::string> UserInterfaceIO::wrap(const std::string &text, unsigned int full)
{
    std::vector<std::string> lines;

    if (lineWidth == 0) {
        lines.push_back(text);
        return lines;
    }

    boost::char_separator<char> separators(" \t");
    boost::char_separator<char> lineSeparators("\n");
    boost::tokenizer<boost::char_separator<char> > lineTokenizer(text, lineSeparators);
    
    std::string word;
    std::string line;
    
    for (boost::tokenizer<boost::char_separator<char> >::const_iterator itl = lineTokenizer.begin();
         itl != lineTokenizer.end(); ++itl) {
        if (itl->empty()) {
            lines.push_back("");
            continue;
        }
        boost::tokenizer<boost::char_separator<char> > tokenizer(*itl, separators);
        for (boost::tokenizer<boost::char_separator<char> >::const_iterator it = tokenizer.begin();
             it != tokenizer.end(); ++it) {
            word = *it;
            boost::algorithm::trim(word);
            if ((line.length() + word.length() + 1) > (lineWidth - full)) {
                lines.push_back(line);
                line.clear();
            } else if (line.empty() && (word.length() >= (lineWidth - full))) {
                lines.push_back(word);
                continue;
            }
            if (!line.empty())
                line += " ";
            line += word;
        }

        if (!line.empty()) {
            lines.push_back(line);
            line.clear();
        }
    }

    return lines;
}



}
}
