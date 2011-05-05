/*
* Copyright (C) 2011 Jan Kundrát <kundratj@fzu.cz>
* Copyright (C) 2011 Tomáż Hubík <hubik.tomas@gmail.com>
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


ParserSignalCategoryEntered::ParserSignalCategoryEntered(const std::vector<Db::ObjectDefinition> &context,
                                                         const Db::Identifier &kind, const Db::Identifier &object):
    contextStack(context), kindName(kind), objectName(object)
{
}



void ParserSignalCategoryEntered::apply(Db::Api *api)
{
}



ParserSignalSetAttribute::ParserSignalSetAttribute(const std::vector<Db::ObjectDefinition> &context, 
                                                   const Db::Identifier &attribute, const Db::Value &value):
    contextStack(context), attributeName(attribute), setValue(value)
{
}



void ParserSignalSetAttribute::apply(Db::Api *api)
{
}



ParserSignalFunctionShow::ParserSignalFunctionShow(const std::vector<Db::ObjectDefinition> &context):
    contextStack(context)
{
}



void ParserSignalFunctionShow::apply(Db::Api *api)
{
}



ParserSignalFunctionDelete::ParserSignalFunctionDelete(const std::vector<Db::ObjectDefinition> &context):
    contextStack(context)
{
}



void ParserSignalFunctionDelete::apply(Db::Api *api)
{
}



SignalsHandler::SignalsHandler(Parser *parser)
{
    m_parser = parser;
}



ApplyParserSignal::ApplyParserSignal(Db::Api *api): m_api(api)
{
}



template <typename T>
void ApplyParserSignal::operator()(const T &parserSignal) const
{
    parserSignal.apply(m_api);
}



void SignalsHandler::slotCategoryEntered(const Db::Identifier &kind, const Db::Identifier &name)
{
    signalsStack.push_back(ParserSignalCategoryEntered(m_parser->currentContextStack(), kind, name));
}



void SignalsHandler::slotCategoryLeft()
{
    // TODO ?
}



void SignalsHandler::slotSetAttribute(const Db::Identifier &attribute, const Db::Value &value)
{
    signalsStack.push_back(ParserSignalSetAttribute(m_parser->currentContextStack(), attribute, value));
}



void SignalsHandler::slotFunctionShow()
{
    signalsStack.push_back(ParserSignalFunctionShow(m_parser->currentContextStack()));
}



void SignalsHandler::slotFunctionDelete()
{
    signalsStack.push_back(ParserSignalFunctionDelete(m_parser->currentContextStack()));
}



void SignalsHandler::slotParserError(const ParserException &error)
{
    // TODO
}



void SignalsHandler::slotParsingFinished()
{
    // TODO
}



}
}
