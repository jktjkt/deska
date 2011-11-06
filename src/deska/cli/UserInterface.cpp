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

#include "CliCommands.h"
#include "CliCommands_Log.h"
#include "CliCommands_Rebase.h"
#include "DbInteraction.h"
#include "Exceptions.h"
#include "Parser.h"
#include "UserInterface.h"
#include "UserInterfaceIOBase.h"
#include "CliConfig.h"
#include "deska/db/JsonApi.h"



namespace Deska
{
namespace Cli
{



UserInterface::UserInterface(DbInteraction *dbInteraction, Parser *parser, UserInterfaceIOBase *_io, CliConfig* _config):
    m_dbInteraction(dbInteraction), m_parser(parser), io(_io), currentChangeset(),
    forceNonInteractive(_config->getVar<bool>(CLI_NonInteractive))
{
    // Register all commands
    typedef std::tr1::shared_ptr<Command> Ptr;
    commandsMap["start"] = Ptr(new Start(this));
    commandsMap["resume"] = Ptr(new Resume(this));
    commandsMap["commit"] = Ptr(new Commit(this));
    commandsMap["detach"] = Ptr(new Detach(this));
    commandsMap["abort"] = Ptr(new Abort(this));
    commandsMap["rebase"] = Ptr(new Rebase(this));
    commandsMap["status"] = Ptr(new Status(this));
    commandsMap["log"] = Ptr(new Log(this));
    commandsMap["diff"] = Ptr(new Diff(this));
    commandsMap["configdiff"] = Ptr(new Configdiff(this));
    commandsMap["exit"] = Ptr(new Exit(this));
    commandsMap["quit"] = commandsMap["exit"];
    commandsMap["context"] = Ptr(new Context(this));
    commandsMap["dump"] = Ptr(new Dump(this));
    commandsMap["restore"] = Ptr(new Restore(this));
    // Help has to be constructed last because of completions generating
    commandsMap["help"] = Ptr(new Help(this));

    // Register all commands completions
    for (CommandMap::iterator it = commandsMap.begin(); it != commandsMap.end(); ++it) {
        std::vector<std::string> cmdCompletions = it->second->completionPatterns();
        for (std::vector<std::string>::iterator itc = cmdCompletions.begin(); itc != cmdCompletions.end(); ++itc)
            io->addCommandCompletion(*itc);
    }
    nonInteractiveMode = false;
}



UserInterface::~UserInterface()
{
}



bool UserInterface::applyCreateObject(const ContextStack &context,
                                      const Db::Identifier &kind, const Db::Identifier &object,
                                      ContextStackItem &newItem)
{
    try {
        newItem = m_dbInteraction->createObject(context);
        return true;
    } catch (Deska::Db::ReCreateObjectError &e) {
        if (io->confirmRestoration(ObjectDefinition(kind,object))) {
            m_dbInteraction->restoreDeletedObject(context);
            newItem = ContextStackItem(kind, object);
            return true;
        } else {
            return false;
        }
    }
}



bool UserInterface::applyCategoryEntered(const ContextStack &context,
                                         const Db::Identifier &kind, const Db::Identifier &object,
                                         ContextStackItem &newItem)
{
    if (m_dbInteraction->objectExists(context)) {
        newItem = ContextStackItem(kind, object);
        return true;
    }

    // Object does not exist -> try to create it
    try {
        newItem = m_dbInteraction->createObject(context);
        return true;
    } catch (Deska::Db::ReCreateObjectError &e) {
        if (nonInteractiveMode || forceNonInteractive || io->confirmRestoration(ObjectDefinition(kind,object))) {
            m_dbInteraction->restoreDeletedObject(context);
            newItem = ContextStackItem(kind, object);
            return true;
        } else {
            return false;
        }
    }
}



bool UserInterface::applySetAttribute(const ContextStack &context, const Db::Identifier &kind,
                                      const Db::Identifier &attribute, const Db::Value &value)
{
    ContextStack adjustedContext = context;
    if (context.back().kind != kind) {
        adjustedContext.back().kind = kind;
        if (!m_dbInteraction->objectExists(adjustedContext))
            m_dbInteraction->createObject(adjustedContext);
    }
    m_dbInteraction->setAttribute(adjustedContext, AttributeDefinition(attribute, value));
    return true;
}



bool UserInterface::applySetAttributeInsert(const ContextStack &context, const Db::Identifier &kind,
                                            const Db::Identifier &attribute, const Db::Identifier &value)
{
    ContextStack adjustedContext = context;
    if (context.back().kind != kind) {
        adjustedContext.back().kind = kind;
        if (!m_dbInteraction->objectExists(adjustedContext))
            m_dbInteraction->createObject(adjustedContext);
    }
    m_dbInteraction->setAttributeInsert(adjustedContext, attribute, value);
    return true;
}



bool UserInterface::applySetAttributeRemove(const ContextStack &context, const Db::Identifier &kind,
                                            const Db::Identifier &attribute, const Db::Identifier &value)
{
    ContextStack adjustedContext = context;
    if (context.back().kind != kind) {
        adjustedContext.back().kind = kind;
        if (!m_dbInteraction->objectExists(adjustedContext))
            return false;
    }
    m_dbInteraction->setAttributeRemove(adjustedContext, attribute, value);
    return true;
}



bool UserInterface::applyRemoveAttribute(const ContextStack &context, const Db::Identifier &kind, const Db::Identifier &attribute)
{
    ContextStack adjustedContext = context;
    if (context.back().kind != kind) {
        adjustedContext.back().kind = kind;
        if (!m_dbInteraction->objectExists(adjustedContext))
            return false;
    }
    m_dbInteraction->removeAttribute(adjustedContext, attribute);
    return true;
}



bool UserInterface::applyObjectsFilter(const ContextStack &context, const Db::Identifier &kind, 
                                       const boost::optional<Db::Filter> &filter)
{
    if (m_dbInteraction->expandContextStack(context).empty()) {
        io->printMessage("Entered filter does not match any object.");
        return false;
    } else {
        return true;
    }
}



bool UserInterface::applyFunctionShow(const ContextStack &context)
{
    if (context.empty()) {
        // Print top level objects if we are not in any context
        BOOST_FOREACH(const Deska::Db::Identifier &kindName, m_dbInteraction->topLevelKinds()) {
             io->printObjects(m_dbInteraction->kindInstances(kindName), 0, true);
        }
    } else {
        // If we are in some context, print all attributes and kind names
        try {
            showObjectRecursive(ObjectDefinition(context.back().kind, contextStackToPath(context)), 0);
        } catch (std::runtime_error &e) {
            std::vector<ObjectDefinition> objects = m_dbInteraction->expandContextStack(context);
            for (std::vector<ObjectDefinition>::iterator it = objects.begin(); it != objects.end(); ++it) {
                io->printObject(*it, 0, true);
                showObjectRecursive(*it, 1);
            }
        }
    }
    return true;
}



bool UserInterface::applyFunctionDelete(const ContextStack &context)
{
    m_dbInteraction->deleteObject(context);
    return true;
}



bool UserInterface::applyFunctionRename(const ContextStack &context, const Db::Identifier &newName)
{
    m_dbInteraction->renameObject(context, newName);
    return true;
}



bool UserInterface::confirmCreateObject(const ContextStack &context,
                                        const Db::Identifier &kind, const Db::Identifier &object)
{
    if (!currentChangeset) {
        io->reportError("Error: You have to be connected to a changeset to create an object. Use commands \"start\" or \"resume\". Use \"help\" for more info.");
        return false;
    }

    if (m_dbInteraction->objectExists(context)) {
        std::ostringstream ostr;
        ostr << "Object " << ObjectDefinition(kind,object) << " already exists!";
        io->reportError(ostr.str());
        return false;
    }

    return true;
}



bool UserInterface::confirmCategoryEntered(const ContextStack &context,
                                           const Db::Identifier &kind, const Db::Identifier &object, bool &autoCreate)
{
    // We're entering into some context, so we should check whether the object in question exists, and if it does not,
    // ask the user whether to create it.
    if (m_dbInteraction->objectExists(context))
        return true;

    if (!currentChangeset) {
        io->reportError("Error: You have to be connected to a changeset to create an object. Use commands \"start\" or \"resume\". Use \"help\" for more info.");
        return false;
    }

    if (nonInteractiveMode || forceNonInteractive)
        return true;

    // Object does not exist -> ask the user here
    try {
        std::vector<ObjectDefinition> mergedObjects = m_dbInteraction->mergedObjects(context);
        if (mergedObjects.empty())
            autoCreate = io->confirmCreation(ObjectDefinition(kind,object));
        else
            autoCreate = io->confirmCreationConnection(ObjectDefinition(kind, object), mergedObjects);
    } catch (std::logic_error &e) {
        autoCreate = io->confirmCreationConnection(ObjectDefinition(kind, object));
    }
    
    return autoCreate;
}



bool UserInterface::confirmSetAttribute(const ContextStack &context, const Db::Identifier &kind,
                                        const Db::Identifier &attribute, const Db::Value &value)
{
    if (!currentChangeset) {
        io->reportError("Error: You have to be connected to a changeset to set an attribue. Use commands \"start\" or \"resume\". Use \"help\" for more info.");
        return false;
    }

    if (context.back().kind == kind)
        return true;
    ContextStack adjustedContext = context;
    adjustedContext.back().kind = kind;
    if (!nonInteractiveMode && !forceNonInteractive && !m_dbInteraction->objectExists(adjustedContext)) {
        try {
            std::vector<ObjectDefinition> mergedObjects = m_dbInteraction->mergedObjects(adjustedContext);
            if (mergedObjects.empty())
                mergedObjects.push_back(ObjectDefinition(context.back().kind, contextStackToPath(context)));
            return io->confirmCreationConnection(ObjectDefinition(kind, context.back().name), mergedObjects);
        } catch (std::logic_error &e) {
            return io->confirmCreationConnection(ObjectDefinition(kind, context.back().name));
        }
    }
    return true;
}



bool UserInterface::confirmSetAttributeInsert(const ContextStack &context, const Db::Identifier &kind,
                                              const Db::Identifier &attribute, const Db::Identifier &value)
{
    if (!currentChangeset) {
        io->reportError("Error: You have to be connected to a changeset to insert an identifier to a set. Use commands \"start\" or \"resume\". Use \"help\" for more info.");
        return false;
    }

    if (context.back().kind == kind)
        return true;
    ContextStack adjustedContext = context;
    adjustedContext.back().kind = kind;
    if (!nonInteractiveMode && !forceNonInteractive && !m_dbInteraction->objectExists(adjustedContext)) {
        try {
            std::vector<ObjectDefinition> mergedObjects = m_dbInteraction->mergedObjects(adjustedContext);
            if (mergedObjects.empty())
                mergedObjects.push_back(ObjectDefinition(context.back().kind, contextStackToPath(context)));
            return io->confirmCreationConnection(ObjectDefinition(kind, context.back().name), mergedObjects);
        } catch (std::logic_error &e) {
            return io->confirmCreationConnection(ObjectDefinition(kind, context.back().name));
        }
    }
    return true;
}



bool UserInterface::confirmSetAttributeRemove(const ContextStack &context, const Db::Identifier &kind,
                                              const Db::Identifier &attribute, const Db::Identifier &value)
{
    if (!currentChangeset) {
        io->reportError("Error: You have to be connected to a changeset to remove an identifier from a set. Use commands \"start\" or \"resume\". Use \"help\" for more info.");
        return false;
    }

    if (context.back().kind == kind)
        return true;
    ContextStack adjustedContext = context;
    adjustedContext.back().kind = kind;
    if (!m_dbInteraction->objectExists(adjustedContext)) {
        io->reportError("Object " + contextStackToString(adjustedContext) + " does not exist, so you can not remove identifiers from its sets.");
        return false;
    }
    return true;
}



bool UserInterface::confirmRemoveAttribute(const ContextStack &context, const Db::Identifier &kind, const Db::Identifier &attribute)
{
    if (!currentChangeset) {
        io->reportError("Error: You have to be connected to a changeset to remove an attribute. Use commands \"start\" or \"resume\". Use \"help\" for more info.");
        return false;
    }

    if (context.back().kind == kind)
        return true;
    ContextStack adjustedContext = context;
    adjustedContext.back().kind = kind;
    if (!m_dbInteraction->objectExists(adjustedContext)) {
        io->reportError("Object " + contextStackToString(adjustedContext) + " does not exist, so you can not remove its attributes.");
        return false;
    }
    return true;
}



bool UserInterface::confirmObjectsFilter(const ContextStack &context, const Db::Identifier &kind,
                                         const boost::optional<Db::Filter> &filter)
{
    return true;
}



bool UserInterface::confirmFunctionShow(const ContextStack &context)
{
    return true;
}



bool UserInterface::confirmFunctionDelete(const ContextStack &context)
{
    if (!currentChangeset) {
        io->reportError("Error: You have to be connected to a changeset to delete an object. Use commands \"start\" or \"resume\". Use \"help\" for more info.");
        return false;
    }

    if (nonInteractiveMode || forceNonInteractive)
        return true;
    // FIXME: Some better messages
    if (context.back().filter)
        return io->confirmDeletion(ObjectDefinition(context.back().kind, "from filter"));
    else
        return io->confirmDeletion(ObjectDefinition(context.back().kind, context.back().name));
}



bool UserInterface::confirmFunctionRename(const ContextStack &context, const Db::Identifier &newName)
{
    if (!currentChangeset) {
        io->reportError("Error: You have to be connected to a changeset to rename an object. Use commands \"start\" or \"resume\". Use \"help\" for more info.");
        return false;
    }

    return true;
}



void UserInterface::reportParseError(const ParserException &error)
{
    parsingFailed = true;
    if (error.offset() == 0) {
        io->reportError("Error while parsing CLI command or " + error.dump());
    } else {
        io->reportError(error.dump());
    }
}



void UserInterface::run()
{
    io->printMessage("Deska CLI started. For usage info try typing \"help\".");
    std::pair<std::string, bool> line;
    exitLoop = false;
    while (!exitLoop) {
        parsingFailed = false;
        line = io->readLine(contextStackToString(m_parser->currentContextStack()));
        if (line.second)
            (*(commandsMap["quit"]))("");

        // Split line to command and arguments
        std::string::iterator commandEnd = line.first.begin();
        while ((commandEnd != line.first.end()) && (*commandEnd != ' '))
            ++commandEnd;
        std::string parsedCommand(line.first.begin(), commandEnd);
        std::string parsedArguments((commandEnd == line.first.end() ? commandEnd : (commandEnd +1)), line.first.end());
        
        try {
            if (commandsMap.find(parsedCommand) == commandsMap.end()) {
                // Command not found -> use CLI parser
                m_parser->parseLine(line.first);
            } else {
                // Command found -> run it
                (*(commandsMap[parsedCommand]))(parsedArguments);
            }
        } catch (Db::RemoteDbError &e) {
            std::ostringstream ostr;
            ostr << "Unexpected server error:\n" << e.whatWithBacktrace() << std::endl;
            io->reportError(ostr.str());
        }
    }
}



void UserInterface::showObjectRecursive(const ObjectDefinition &object, unsigned int depth)
{
    bool printEnd = false;
    std::vector<std::pair<AttributeDefinition, Db::Identifier> > attributes =
        m_dbInteraction->allAttributesResolvedWithOrigin(object);
    printEnd = printEnd || !attributes.empty();
    io->printAttributesWithOrigin(attributes, depth);
    std::vector<ObjectDefinition> nestedObjs = m_dbInteraction->allNestedObjects(object);
    printEnd = printEnd || !nestedObjs.empty();
    for (std::vector<ObjectDefinition>::iterator it = nestedObjs.begin(); it != nestedObjs.end(); ++it) {
        io->printObject(*it, depth, false);
        showObjectRecursive(*it, depth + 1);
    }
    if (printEnd && (depth > 0))
        io->printEnd(depth - 1);
}



}
}
