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
#include "Parser_p_KindsFiltersParser.h"
#include "Parser_p_FiltersParser.h"
#include "deska/db/Api.h"

namespace Deska
{
namespace Cli
{



template <typename Iterator>
KindsFiltersParser<Iterator>::KindsFiltersParser(const Db::Identifier &kindName, ParserImpl<Iterator> *parent):
    KindsFiltersParser<Iterator>::base_type(start), m_name(kindName), m_parent(parent)
{
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
    //using qi::rethrow;

    // If the boost::spirit::qi::grammar API was sane, the following line would read setName(kindName).
    // The API is not sane, and therefore we have the following crap here.
    this->name(kindName);

    phoenix::function<RangeToString<Iterator> > rangeToString = RangeToString<Iterator>();

    // When parsing some input using Nabialek trick, the rule, that is using the symbols table will not be entered when
    // the keyword is not found in the table. The eps is there to ensure, that the start rule will be entered every
    // time and so the error handler for bad keywords could be bound to it. The eoi rule is there to avoid the grammar
    // require more input on the end of the line, which is side effect of eps usage in this way.
    start = (eps(!_a) > dispatch >> -eoi[_a = true]);

    // Attribute name recognized -> try to parse attribute value. The raw function is here to get the name of the
    // attribute being parsed.
    dispatch = (raw[filters[_a = _1]][rangeToString(_1, phoenix::ref(currentKindName))] > qi::lit("where")
        > lazy(_a)[phoenix::bind(&KindsFiltersParser::parsedFilter, this, phoenix::ref(currentKindName), _1)]);

    phoenix::function<KindFiltersErrorHandler<Iterator> > kindFiltersErrorHandler = KindFiltersErrorHandler<Iterator>();
    //phoenix::function<NestingErrorHandler<Iterator> > nestingErrorHandler = NestingErrorHandler<Iterator>();
    on_error<fail>(start, kindFiltersErrorHandler(_1, _2, _3, _4, phoenix::ref(filters), phoenix::ref(m_name), m_parent));
    // In case of enabling error handler for nesting, on_error<fail> for kindErrorHandler have to be changed
    // to on_error<rethrow>.
    //on_error<fail>(start, nestingErrorHandler(_1, _2, _3, _4, phoenix::ref(currentKindName),
    //                                          phoenix::ref(m_name), m_parent));
}



template <typename Iterator>
void KindsFiltersParser<Iterator>::addKindFilter(const Db::Identifier &kindName,
                                                 FiltersParser<Iterator> *filtersParser)
{
    // This creates rule reference to our grammar so we can store it in symbols table.
    qi::rule<Iterator, Db::Filter(), ascii::space_type> filterRuleRef = (*filtersParser);
    filters.add(kindName, filterRuleRef);
}



template <typename Iterator>
void KindsFiltersParser<Iterator>::parsedFilter(const Db::Identifier &kindName, const Db::Filter &filter)
{
    m_parent->objectsFilter(filter);
}



/////////////////////////Template instances for linker//////////////////////////

template KindsFiltersParser<iterator_type>::KindsFiltersParser(const Db::Identifier &kindName, ParserImpl<iterator_type> *parent);

template void KindsFiltersParser<iterator_type>::addKindFilter(const Db::Identifier &kindName, FiltersParser<iterator_type> *filtersParser);

template void KindsFiltersParser<iterator_type>::parsedFilter(const Db::Identifier &kindName, const Db::Filter &filter);

}
}
