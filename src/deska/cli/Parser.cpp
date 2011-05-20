/* Copyright (C) 2011 Jan Kundrát <kundratj@fzu.cz>
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

#include <boost/assert.hpp>
#include "Parser.h"
#include "Parser_p.h"


namespace Deska
{
namespace Cli
{


Parser::Parser(Db::Api *dbApi)
{
    m_dbApi = dbApi;
    BOOST_ASSERT(m_dbApi);
    d_ptr = new ParserImpl<iterator_type>(this);
}

Parser::~Parser()
{
    delete d_ptr;
}

void Parser::parseLine(const std::string &line)
{
    d_ptr->parseLine(line);
}

std::vector<std::string> Parser::tabCompletitionPossibilities(const std::string &line)
{
    return d_ptr->tabCompletitionPossibilities(line);
}

bool Parser::isNestedInContext() const
{
    return d_ptr->isNestedInContext();
}

Db::ContextStack Parser::currentContextStack() const
{
    return d_ptr->currentContextStack();
}

void Parser::setContextStack(const Db::ContextStack &stack)
{
    d_ptr->setContextStack(stack);
}

void Parser::clearContextStack()
{
    d_ptr->clearContextStack();
}

}
}
