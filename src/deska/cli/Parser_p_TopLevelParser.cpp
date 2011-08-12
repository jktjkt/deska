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
#include "Parser_p_KindsOnlyParser.h"
#include "Parser_p_KindsFiltersParser.h"
#include "Parser_p_TopLevelParser.h"

namespace Deska
{
namespace Cli
{


template <typename Iterator>
TopLevelParser<Iterator>::TopLevelParser(KindsOnlyParser<Iterator> *topLevelKinds,
                                         KindsFiltersParser<Iterator> *topLevelKindsFilters,
                                         ParserImpl<Iterator> *parent):
    TopLevelParser<Iterator>::base_type(start), m_parent(parent)
{
    start = ((*topLevelKinds)
            | (*topLevelKindsFilters))[phoenix::bind(&TopLevelParser::parsedSingleKind, this)];
}



template <typename Iterator>
void TopLevelParser<Iterator>::parsedSingleKind()
{
    m_parent->parsedSingleKind();
}



/////////////////////////Template instances for linker//////////////////////////

template TopLevelParser<iterator_type>::TopLevelParser(KindsOnlyParser<iterator_type> *topLevelKinds, KindsFiltersParser<iterator_type> *topLevelKindsFilters, ParserImpl<iterator_type> *parent);

template void TopLevelParser<iterator_type>::parsedSingleKind();

}
}
