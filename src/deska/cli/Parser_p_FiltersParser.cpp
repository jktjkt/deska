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
#include "Parser_p_FiltersParser.h"
#include "Parser_p_FilterExpressionsParser.h"

namespace Deska
{
namespace Cli
{


template <typename Iterator>
FiltersParser<Iterator>::FiltersParser(const Db::Identifier &kindName,
                                       FilterExpressionsParser<Iterator> *ownAttrsExpressionsParser,
                                       ParserImpl<Iterator> *parent):
    FiltersParser<Iterator>::base_type(start), m_name(kindName), m_parent(parent)
{
    using qi::_val;
    using qi::_1;
    using qi::_2;
    using qi::_3;
    using qi::_4;
    using qi::_a;
    using qi::eps;
    using qi::raw;
    using qi::eoi;
    using qi::on_error;
    using qi::fail;

    // If the boost::spirit::qi::grammar API was sane, the following line would read setName(kindName).
    // The API is not sane, and therefore we have the following crap here.
    this->name(kindName);

    phoenix::function<RangeToString<Iterator> > rangeToString = RangeToString<Iterator>();

    start %= ((qi::lit("(") >> andFilter >> qi::lit(")"))
            | (qi::lit("(") >> orFilter >> qi::lit(")"))
            | (qi::lit("(") >> attrExpr >> qi::lit(")")));

    andFilter = (start % qi::lit("&"))[_val = phoenix::construct<Db::AndFilter>(_1)];
    orFilter = (start % qi::lit("|"))[_val = phoenix::construct<Db::OrFilter>(_1)];

    attrExpr %= (*ownAttrsExpressionsParser) | nestedAttrExpr;

    // When parsing some input using Nabialek trick, the rule, that is using the symbols table will not be entered when
    // the keyword is not found in the table. The eps is there to ensure, that the start rule will be entered every
    // time and so the error handler for bad keywords could be bound to it. The eoi rule is there to avoid the grammar
    // require more input on the end of the line, which is side effect of eps usage in this way.
    nestedAttrExpr %= (eps(!_a) > dispatch >> -eoi[_a = true]);

    // Nested kind name recognized -> try to parse expression for the kind. The raw function is here to get the name
    // of the kind for which the expression is being parsed.
    dispatch = (raw[nestedAttributes[_a = _1]][rangeToString(_1, phoenix::ref(nestedKindName))] >
               qi::lit(".") > lazy(_a))[_val = _2];

    phoenix::function<KindFiltersErrorHandler<Iterator> > kindErrorHandler = KindFiltersErrorHandler<Iterator>();
    on_error<fail>(nestedAttrExpr, kindErrorHandler(_1, _2, _3, _4,
                                                    phoenix::ref(nestedAttributes), phoenix::ref(m_name), m_parent));
}



template <typename Iterator>
void FiltersParser<Iterator>::addNestedKindExpressionsParser(const Db::Identifier &nestedKindName,
                                                             FilterExpressionsParser<Iterator> *expressionsParser)
{
    // This creates rule reference to our grammar so we can store it in symbols table.
    qi::rule<Iterator, Db::Filter(), ascii::space_type> filterRuleRef = (*expressionsParser);
    nestedAttributes.add(nestedKindName, filterRuleRef);
}



/////////////////////////Template instances for linker//////////////////////////

template FiltersParser<iterator_type>::FiltersParser(const Db::Identifier &kindName, FilterExpressionsParser<iterator_type> *ownAttrsExpressionsParser, ParserImpl<iterator_type> *parent);

template void FiltersParser<iterator_type>::addNestedKindExpressionsParser(const Db::Identifier &nestedKindName, FilterExpressionsParser<iterator_type> *expressionsParser);

}
}
