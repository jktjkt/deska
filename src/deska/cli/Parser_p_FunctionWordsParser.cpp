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

#include <boost/assert.hpp>
#include "Parser_p_FunctionWordsParser.h"

namespace Deska
{
namespace Cli
{



template <typename Iterator>
FunctionWordsParser<Iterator>::FunctionWordsParser(ParserImpl<Iterator> *parent):
    FunctionWordsParser<Iterator>::base_type(start), m_parent(parent)
{
    start = ((keyword["create"][phoenix::bind(&FunctionWordsParser::actionCreate, this)])
           | (keyword["delete"][phoenix::bind(&FunctionWordsParser::actionDelete, this)])
           | (keyword["show"][phoenix::bind(&FunctionWordsParser::actionShow, this)])
           | (keyword["rename"][phoenix::bind(&FunctionWordsParser::actionRename, this)]));
}



template <typename Iterator>
void FunctionWordsParser<Iterator>::actionCreate()
{
    m_parent->setParsingMode(PARSING_MODE_CREATE);
}



template <typename Iterator>
void FunctionWordsParser<Iterator>::actionDelete()
{
    m_parent->setParsingMode(PARSING_MODE_DELETE);
}



template <typename Iterator>
void FunctionWordsParser<Iterator>::actionShow()
{
    m_parent->setParsingMode(PARSING_MODE_SHOW);
}



template <typename Iterator>
void FunctionWordsParser<Iterator>::actionRename()
{
    m_parent->setParsingMode(PARSING_MODE_RENAME);
}



/////////////////////////Template instances for linker//////////////////////////

template FunctionWordsParser<iterator_type>::FunctionWordsParser(ParserImpl<iterator_type> *parent);

template void FunctionWordsParser<iterator_type>::actionCreate();

template void FunctionWordsParser<iterator_type>::actionDelete();

template void FunctionWordsParser<iterator_type>::actionShow();

template void FunctionWordsParser<iterator_type>::actionRename();

}
}
