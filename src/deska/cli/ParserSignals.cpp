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


ParserSignalCategoryEntered::ParserSignalCategoryEntered(const ContextStack &context,
                                                         const Db::Identifier &kind, const Db::Identifier &object):
    signalsContext(context), kindName(kind), objectName(object)
{
}



bool ParserSignalCategoryEntered::apply(SignalsHandler *signalsHandler) const
{
    if (signalsHandler->userInterface->applyCategoryEntered(signalsContext, kindName, objectName)) {
        signalsHandler->contextStack.push_back(Db::ObjectDefinition(kindName, objectName));
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
        // Careful here -- we have to work with *our* instance of the contextStack, not the signalsHandler's one
        signalsHandler->autoCreate = signalsHandler->userInterface->confirmCategoryEntered(signalsContext, kindName, objectName);
    }
    return signalsHandler->autoCreate;
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



ParserSignalSetAttribute::ParserSignalSetAttribute(const ContextStack &context, 
                                                   const Db::Identifier &attribute, const Db::Value &value):
    signalsContext(context), attributeName(attribute), setValue(value)
{
}



bool ParserSignalSetAttribute::apply(SignalsHandler *signalsHandler) const
{
    return signalsHandler->userInterface->applySetAttribute(signalsContext, attributeName, setValue);
}



bool ParserSignalSetAttribute::confirm(SignalsHandler *signalsHandler) const
{
    return signalsHandler->userInterface->confirmSetAttribute(signalsContext, attributeName, setValue);
}



ParserSignalRemoveAttribute::ParserSignalRemoveAttribute(const ContextStack &context, 
                                                         const Db::Identifier &attribute):
    signalsContext(context), attributeName(attribute)
{
}



bool ParserSignalRemoveAttribute::apply(SignalsHandler *signalsHandler) const
{
    return signalsHandler->userInterface->applyRemoveAttribute(signalsContext, attributeName);
}



bool ParserSignalRemoveAttribute::confirm(SignalsHandler *signalsHandler) const
{
    return signalsHandler->userInterface->confirmRemoveAttribute(signalsContext, attributeName);
}



ParserSignalFunctionShow::ParserSignalFunctionShow(const ContextStack &context):
    signalsContext(context)
{
}



bool ParserSignalFunctionShow::apply(SignalsHandler *signalsHandler) const
{
    return signalsHandler->userInterface->applyFunctionShow(signalsContext);
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
    return signalsHandler->userInterface->applyFunctionDelete(signalsContext);
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
    return signalsHandler->userInterface->applyFunctionRename(signalsContext, name);
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
    m_parser(parser), userInterface(_userInterface), autoCreate(false)
{   
    using boost::phoenix::arg_names::_1;
    using boost::phoenix::arg_names::_2;
    m_parser->categoryEntered.connect(boost::phoenix::bind(&SignalsHandler::slotCategoryEntered, this, _1, _2));
    m_parser->categoryLeft.connect(boost::phoenix::bind(&SignalsHandler::slotCategoryLeft, this));
    m_parser->attributeSet.connect(boost::phoenix::bind(&SignalsHandler::slotSetAttribute, this, _1, _2));
    m_parser->attributeRemove.connect(boost::phoenix::bind(&SignalsHandler::slotRemoveAttribute, this, _1));
    m_parser->functionShow.connect(boost::phoenix::bind(&SignalsHandler::slotFunctionShow, this));
    m_parser->functionDelete.connect(boost::phoenix::bind(&SignalsHandler::slotFunctionDelete, this));
    m_parser->functionRename.connect(boost::phoenix::bind(&SignalsHandler::slotFunctionRename, this, _1));
    m_parser->parseError.connect(boost::phoenix::bind(&SignalsHandler::slotParserError, this, _1));
    m_parser->parsingFinished.connect(boost::phoenix::bind(&SignalsHandler::slotParsingFinished, this));
}



void SignalsHandler::slotCategoryEntered(const Db::Identifier &kind, const Db::Identifier &name)
{
    signalsStack.push_back(ParserSignalCategoryEntered(m_parser->currentContextStack(), kind, name));
}



void SignalsHandler::slotCategoryLeft()
{
    signalsStack.push_back(ParserSignalCategoryLeft());
}



void SignalsHandler::slotSetAttribute(const Db::Identifier &attribute, const Db::Value &value)
{
    signalsStack.push_back(ParserSignalSetAttribute(m_parser->currentContextStack(), attribute, value));
}



void SignalsHandler::slotRemoveAttribute(const Db::Identifier &attribute)
{
    signalsStack.push_back(ParserSignalRemoveAttribute(m_parser->currentContextStack(), attribute));
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
    autoCreate = false;
    signalsStack.clear();
    userInterface->reportParseError(error);
}



void SignalsHandler::slotParsingFinished()
{
    autoCreate = false;
    bool allConfirmed = true;
    ApplyParserSignal applyParserSignal(this);
    ConfirmParserSignal confirmParserSignal(this);

    for (std::vector<ParserSignal>::iterator it = signalsStack.begin(); it != signalsStack.end(); ++it) {
        if (!(boost::apply_visitor(confirmParserSignal, *it))) {
            allConfirmed = false;
            break;
        }
    }

    if (allConfirmed) {
        for (std::vector<ParserSignal>::iterator it = signalsStack.begin(); it != signalsStack.end(); ++it) {
            if (!(boost::apply_visitor(applyParserSignal, *it)))
                break;
        }
    }

    signalsStack.clear();

    // Set context stack of parser. In case we did not confirm creation of an object, parser is nested, but should not.
    m_parser->setContextStack(contextStack);
}


}
}
