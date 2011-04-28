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

#include "ParserSignals.h"


namespace Deska
{
namespace Cli
{


ParserSignal::ParserSignal(SignalType type, const std::vector<ContextStackItem> &context,
                           const Db::Identifier &kind, const Db::Identifier &object):
    signalType(type), contextStack(context), kindName(kind), objectName(object)
{
}



ParserSignal::ParserSignal(SignalType type, const std::vector<ContextStackItem> &context):
    signalType(type), contextStack(context)
{
}



ParserSignal::ParserSignal(SignalType type, const std::vector<ContextStackItem> &context, 
                           const Db::Identifier &attribute, const Db::Value &value):
    signalType(type), contextStack(context), attributeName(attribute), setValue(value)
{
}



ParserSignal::ParserSignal(SignalType type, const std::vector<ContextStackItem> &context,
                           const std::string &error):
    signalType(type), contextStack(context), parseError(error)
{
}



SignalsHandler::SignalsHandler()
{
}



void SignalsHandler::slotCategoryEntered(const Db::Identifier &kind, const Db::Identifier &name)
{
    signalsStack.push_back(ParserSignal(SIGNAL_TYPE_CATEGORY_ENTERED, parser->currentContextStack(), kind, name));
}



void SignalsHandler::slotCategoryLeft()
{
    signalsStack.push_back(ParserSignal(SIGNAL_TYPE_CATEGORY_LEFT, parser->currentContextStack()));
}



void SignalsHandler::slotSetAttribute(const Db::Identifier &attribute, const Db::Value &value)
{
    signalsStack.push_back(ParserSignal(SIGNAL_TYPE_SET_ATTRIBUTE, parser->currentContextStack(), attribute, value));
}



void SignalsHandler::slotFunctionShow()
{
    signalsStack.push_back(ParserSignal(SIGNAL_TYPE_FUNCTION_SHOW, parser->currentContextStack()));
}



void SignalsHandler::slotFunctionDelete()
{
    signalsStack.push_back(ParserSignal(SIGNAL_TYPE_FUNCTION_DELETE, parser->currentContextStack()));
}



void SignalsHandler::slotParserError(const ParserException &error)
{
    signalsStack.push_back(ParserSignal(SIGNAL_TYPE_PARSE_ERROR, parser->currentContextStack(), error.dump()));
}



}
}