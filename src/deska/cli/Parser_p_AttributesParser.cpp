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

#include <boost/assert.hpp>
#include "Parser_p_AttributeRemovalsParser.h"
#include "Parser_p_AttributesSettingParser.h"
#include "Parser_p_AttributesParser.h"
#include "deska/db/Api.h"

namespace Deska
{
namespace Cli
{


template <typename Iterator>
AttributesParser<Iterator>::AttributesParser(const Db::Identifier &kindName,
                                             AttributesSettingParser<Iterator> *attributesSettingParser,
                                             AttributeRemovalsParser<Iterator> *attributeRemovalsParser,
                                             ParserImpl<Iterator> *parent):
    AttributesParser<Iterator>::base_type(start), m_parent(parent)
{
    // If the boost::spirit::qi::grammar API was sane, the following line would read setName(kindName).
    // The API is not sane, and therefore we have the following crap here.
    this->name(kindName);

    start = (+((*attributesSettingParser) | (*attributeRemovalsParser)));
}



/////////////////////////Template instances for linker//////////////////////////

template AttributesParser<iterator_type>::AttributesParser(const std::string &kindName, AttributesSettingParser<iterator_type> *attributesSettingParser, AttributeRemovalsParser<iterator_type> *attributeRemovalsParser, ParserImpl<iterator_type> *parent);

}
}
