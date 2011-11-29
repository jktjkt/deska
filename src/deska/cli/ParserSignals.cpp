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

#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/phoenix_core.hpp>

#include "Exceptions.h"
#include "Parser.h"
#include "ParserSignals.h"
#include "UserInterface.h"


namespace Deska
{
namespace Cli
{


ParserSignalCreateObject::ParserSignalCreateObject(const ContextStack &context,
                                                   const Db::Identifier &kind, const Db::Identifier &object):
    signalsContext(context), kindName(kind), objectName(object)
{
}



bool ParserSignalCreateObject::apply(SignalsHandler *signalsHandler) const
{
    ContextStackItem newItem;
    ContextStack handlersContext = signalsHandler->contextStack;
    handlersContext.push_back(ContextStackItem(kindName, objectName));
    if (signalsHandler->userInterface->applyCreateObject(handlersContext, kindName, objectName, newItem)) {
        signalsHandler->contextStack.push_back(newItem);
        return true;
    } else {
        return false;
    }
}



bool ParserSignalCreateObject::confirm(SignalsHandler *signalsHandler) const
{
    return signalsHandler->userInterface->confirmCreateObject(signalsContext, kindName, objectName);
}



ParserSignalCategoryEntered::ParserSignalCategoryEntered(const ContextStack &context,
                                                         const Db::Identifier &kind, const Db::Identifier &object):
    signalsContext(context), kindName(kind), objectName(object)
{
}



bool ParserSignalCategoryEntered::apply(SignalsHandler *signalsHandler) const
{
    ContextStackItem newItem;
    ContextStack handlersContext = signalsHandler->contextStack;
    handlersContext.push_back(ContextStackItem(kindName, objectName));
    if (signalsHandler->userInterface->applyCategoryEntered(handlersContext, kindName, objectName, newItem)) {
        signalsHandler->contextStack.push_back(newItem);
        return true;
    } else {
        return false;
    }
}



bool ParserSignalCategoryEntered::confirm(SignalsHandler *signalsHandler) const
{
    if (signalsHandler->autoCreate) {
        return true;
    } else {
        return signalsHandler->userInterface->confirmCategoryEntered(signalsContext, kindName, objectName, signalsHandler->autoCreate);
    }
}



ParserSignalCategoryLeft::ParserSignalCategoryLeft()
{
}



bool ParserSignalCategoryLeft::apply(SignalsHandler *signalsHandler) const
{
    signalsHandler->contextStack.pop_back();
    return true;
}



bool ParserSignalCategoryLeft::confirm(SignalsHandler *signalsHandler) const
{
    signalsHandler->autoCreate = false;
    return true;
}



ParserSignalSetAttribute::ParserSignalSetAttribute(const ContextStack &context, const Db::Identifier &kind, 
                                                   const Db::Identifier &attribute, const Db::Value &value):
    signalsContext(context), kindName(kind), attributeName(attribute), setValue(value)
{
}



bool ParserSignalSetAttribute::apply(SignalsHandler *signalsHandler) const
{
    return signalsHandler->userInterface->applySetAttribute(signalsHandler->contextStack, kindName, attributeName, setValue);
}



bool ParserSignalSetAttribute::confirm(SignalsHandler *signalsHandler) const
{
    return signalsHandler->userInterface->confirmSetAttribute(signalsContext, kindName, attributeName, setValue);
}



ParserSignalSetAttributeInsert::ParserSignalSetAttributeInsert(const ContextStack &context, const Db::Identifier &kind, 
                                                               const Db::Identifier &attribute,
                                                               const Db::Identifier &value):
    signalsContext(context), kindName(kind), setName(attribute), identifier(value)
{
}



bool ParserSignalSetAttributeInsert::apply(SignalsHandler *signalsHandler) const
{
    return signalsHandler->userInterface->applySetAttributeInsert(signalsHandler->contextStack, kindName, setName, identifier);
}



bool ParserSignalSetAttributeInsert::confirm(SignalsHandler *signalsHandler) const
{
    return signalsHandler->userInterface->confirmSetAttributeInsert(signalsContext, kindName, setName, identifier);
}


ParserSignalSetAttributeRemove::ParserSignalSetAttributeRemove(const ContextStack &context, const Db::Identifier &kind, 
                                                               const Db::Identifier &attribute,
                                                               const Db::Identifier &value):
    signalsContext(context), kindName(kind), setName(attribute), identifier(value)
{
}



bool ParserSignalSetAttributeRemove::apply(SignalsHandler *signalsHandler) const
{
    return signalsHandler->userInterface->applySetAttributeRemove(signalsHandler->contextStack, kindName, setName, identifier);
}



bool ParserSignalSetAttributeRemove::confirm(SignalsHandler *signalsHandler) const
{
    return signalsHandler->userInterface->confirmSetAttributeRemove(signalsContext, kindName, setName, identifier);
}



ParserSignalRemoveAttribute::ParserSignalRemoveAttribute(const ContextStack &context, const Db::Identifier &kind, 
                                                         const Db::Identifier &attribute):
    signalsContext(context), kindName(kind), attributeName(attribute)
{
}



bool ParserSignalRemoveAttribute::apply(SignalsHandler *signalsHandler) const
{
    return signalsHandler->userInterface->applyRemoveAttribute(signalsHandler->contextStack, kindName, attributeName);
}



bool ParserSignalRemoveAttribute::confirm(SignalsHandler *signalsHandler) const
{
    return signalsHandler->userInterface->confirmRemoveAttribute(signalsContext, kindName, attributeName);
}



ParserSignalObjectsFilter::ParserSignalObjectsFilter(const ContextStack &context, const Db::Identifier &kind,
                                                     const boost::optional<Db::Filter> &filter):
    signalsContext(context), kindName(kind), objectsFilter(filter)
{
}



bool ParserSignalObjectsFilter::apply(SignalsHandler *signalsHandler) const
{
    ContextStack handlersContext = signalsHandler->contextStack;
    handlersContext.push_back(ContextStackItem(kindName, objectsFilter));
    if (signalsHandler->userInterface->applyObjectsFilter(handlersContext, kindName, objectsFilter)) {
        signalsHandler->contextStack.push_back(ContextStackItem(kindName, objectsFilter));
        return true;
    } else {
        return false;
    }
}



bool ParserSignalObjectsFilter::confirm(SignalsHandler *signalsHandler) const
{
    return signalsHandler->userInterface->confirmObjectsFilter(signalsContext, kindName, objectsFilter);
}



ParserSignalFunctionShow::ParserSignalFunctionShow(const ContextStack &context):
    signalsContext(context)
{
}



bool ParserSignalFunctionShow::apply(SignalsHandler *signalsHandler) const
{
    signalsHandler->functionWord = true;
    return signalsHandler->userInterface->applyFunctionShow(signalsHandler->contextStack);
}



bool ParserSignalFunctionShow::confirm(SignalsHandler *signalsHandler) const
{
    return signalsHandler->userInterface->confirmFunctionShow(signalsContext);
}



ParserSignalFunctionDelete::ParserSignalFunctionDelete(const ContextStack &context):
    signalsContext(context)
{
}



bool ParserSignalFunctionDelete::apply(SignalsHandler *signalsHandler) const
{
    signalsHandler->functionWord = true;
    return signalsHandler->userInterface->applyFunctionDelete(signalsHandler->contextStack);
}



bool ParserSignalFunctionDelete::confirm(SignalsHandler *signalsHandler) const
{
    return signalsHandler->userInterface->confirmFunctionDelete(signalsContext);
}



ParserSignalFunctionRename::ParserSignalFunctionRename(const ContextStack &context, const Db::Identifier &newName):
    signalsContext(context), name(newName)
{
}



bool ParserSignalFunctionRename::apply(SignalsHandler *signalsHandler) const
{
    signalsHandler->functionWord = true;
    return signalsHandler->userInterface->applyFunctionRename(signalsHandler->contextStack, name);
}



bool ParserSignalFunctionRename::confirm(SignalsHandler *signalsHandler) const
{
    return signalsHandler->userInterface->confirmFunctionRename(signalsContext, name);
}



ApplyParserSignal::ApplyParserSignal(SignalsHandler *_signalsHandler): signalsHandler(_signalsHandler)
{
}



template <typename T>
bool ApplyParserSignal::operator()(const T &parserSignal) const
{
    return parserSignal.apply(signalsHandler);
}



ConfirmParserSignal::ConfirmParserSignal(SignalsHandler *_signalsHandler): signalsHandler(_signalsHandler)
{
}



template <typename T>
bool ConfirmParserSignal::operator()(const T &parserSignal) const
{
    return parserSignal.confirm(signalsHandler);
}



SignalsHandler::SignalsHandler(Parser *parser, UserInterface *_userInterface):
    m_parser(parser), userInterface(_userInterface), autoCreate(false), functionWord(false)
{   
    using boost::phoenix::arg_names::_1;
    using boost::phoenix::arg_names::_2;
    using boost::phoenix::arg_names::_3;
    m_parser->createObject.connect(boost::phoenix::bind(&SignalsHandler::slotCreateObject, this, _1, _2));
    m_parser->categoryEntered.connect(boost::phoenix::bind(&SignalsHandler::slotCategoryEntered, this, _1, _2));
    m_parser->categoryLeft.connect(boost::phoenix::bind(&SignalsHandler::slotCategoryLeft, this));
    m_parser->attributeSet.connect(boost::phoenix::bind(&SignalsHandler::slotSetAttribute, this, _1, _2, _3));
    m_parser->attributeSetInsert.connect(boost::phoenix::bind(&SignalsHandler::slotSetAttributeInsert, this, _1, _2, _3));
    m_parser->attributeSetRemove.connect(boost::phoenix::bind(&SignalsHandler::slotSetAttributeRemove, this, _1, _2, _3));
    m_parser->attributeRemove.connect(boost::phoenix::bind(&SignalsHandler::slotRemoveAttribute, this, _1, _2));
    m_parser->objectsFilter.connect(boost::phoenix::bind(&SignalsHandler::slotObjectsFilter, this, _1, _2));
    m_parser->functionShow.connect(boost::phoenix::bind(&SignalsHandler::slotFunctionShow, this));
    m_parser->functionDelete.connect(boost::phoenix::bind(&SignalsHandler::slotFunctionDelete, this));
    m_parser->functionRename.connect(boost::phoenix::bind(&SignalsHandler::slotFunctionRename, this, _1));
    m_parser->parseError.connect(boost::phoenix::bind(&SignalsHandler::slotParserError, this, _1));
    m_parser->parsingFinished.connect(boost::phoenix::bind(&SignalsHandler::slotParsingFinished, this));
    m_parser->parsingStarted.connect(boost::phoenix::bind(&SignalsHandler::slotParsingStarted, this));
}



void SignalsHandler::slotCreateObject(const Db::Identifier &kind, const Db::Identifier &name)
{
    signalsStack.push_back(ParserSignalCreateObject(m_parser->currentContextStack(), kind, name));
}



void SignalsHandler::slotCategoryEntered(const Db::Identifier &kind, const Db::Identifier &name)
{
    signalsStack.push_back(ParserSignalCategoryEntered(m_parser->currentContextStack(), kind, name));
}



void SignalsHandler::slotCategoryLeft()
{
    signalsStack.push_back(ParserSignalCategoryLeft());
}



void SignalsHandler::slotSetAttribute(const Db::Identifier &kind, const Db::Identifier &attribute, const Db::Value &value)
{
    signalsStack.push_back(ParserSignalSetAttribute(m_parser->currentContextStack(), kind, attribute, value));
}



void SignalsHandler::slotSetAttributeInsert(const Db::Identifier &kind, const Db::Identifier &attribute, const Db::Identifier &value)
{
    signalsStack.push_back(ParserSignalSetAttributeInsert(m_parser->currentContextStack(), kind, attribute, value));
}



void SignalsHandler::slotSetAttributeRemove(const Db::Identifier &kind, const Db::Identifier &attribute, const Db::Identifier &value)
{
    signalsStack.push_back(ParserSignalSetAttributeRemove(m_parser->currentContextStack(), kind, attribute, value));
}



void SignalsHandler::slotRemoveAttribute(const Db::Identifier &kind, const Db::Identifier &attribute)
{
    signalsStack.push_back(ParserSignalRemoveAttribute(m_parser->currentContextStack(), kind, attribute));
}



void SignalsHandler::slotObjectsFilter(const Db::Identifier &kind, const boost::optional<Db::Filter> &filter)
{
    signalsStack.push_back(ParserSignalObjectsFilter(m_parser->currentContextStack(), kind, filter));
}



void SignalsHandler::slotFunctionShow()
{
    signalsStack.push_back(ParserSignalFunctionShow(m_parser->currentContextStack()));
}



void SignalsHandler::slotFunctionDelete()
{
    signalsStack.push_back(ParserSignalFunctionDelete(m_parser->currentContextStack()));
}



void SignalsHandler::slotFunctionRename(const Db::Identifier &newName)
{
    signalsStack.push_back(ParserSignalFunctionRename(m_parser->currentContextStack(), newName));
}



void SignalsHandler::slotParserError(const ParserException &error)
{
    userInterface->reportParseError(error);
}



void SignalsHandler::slotParsingFinished()
{
    bool allConfirmed = true;
    ApplyParserSignal applyParserSignal(this);
    ConfirmParserSignal confirmParserSignal(this);

    for (std::vector<ParserSignal>::iterator it = signalsStack.begin(); it != signalsStack.end(); ++it) {
        if (!(boost::apply_visitor(confirmParserSignal, *it))) {
            allConfirmed = false;
            break;
        }
    }

    bool allPerformed = true;
    if (allConfirmed) {
        for (std::vector<ParserSignal>::iterator it = signalsStack.begin(); it != signalsStack.end(); ++it) {
            if (!(boost::apply_visitor(applyParserSignal, *it))) {
                allPerformed = false;
                break;
            }
        }
    }

    // Set context stack of parser. In case we did not confirm creation of an object, parser is nested, but should not.
    if (!allPerformed && functionWord)
        m_parser->setContextStack(contextStackBackup);
    else
        m_parser->setContextStack(contextStack);
}



void SignalsHandler::slotParsingStarted()
{
    autoCreate = false;
    functionWord = false;
    signalsStack.clear();
    contextStack = m_parser->currentContextStack();
    contextStackBackup = m_parser->currentContextStack();
}


}
}
