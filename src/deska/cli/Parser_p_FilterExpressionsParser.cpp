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
#include "Parser_p_FilterExpressionsParser.h"

namespace Deska
{
namespace Cli
{


template <typename Iterator>
FilterExpressionsParser<Iterator>::FilterExpressionsParser(const Db::Identifier &kindName, ParserImpl<Iterator> *parent):
    FilterExpressionsParser<Iterator>::base_type(start), m_name(kindName), m_parent(parent)
{
    using qi::_val;
    using qi::_1;
    using qi::_2;
    using qi::_3;
    using qi::_4;
    using qi::_a;
    using qi::_b;
    using qi::eps;
    using qi::raw;
    using qi::eoi;
    using qi::on_error;
    using qi::fail;

    // If the boost::spirit::qi::grammar API was sane, the following line would read setName(kindName).
    // The API is not sane, and therefore we have the following crap here.
    this->name(kindName);

    phoenix::function<RangeToString<Iterator> > rangeToString = RangeToString<Iterator>();

    // Fill symbols table with conversions from string to Db::ComparisonOperator
    operators.add("=", Db::FILTER_COLUMN_EQ);
    operators.add("==", Db::FILTER_COLUMN_EQ);
    operators.add("!=", Db::FILTER_COLUMN_NE);
    operators.add("<>", Db::FILTER_COLUMN_NE);
    operators.add(">", Db::FILTER_COLUMN_GT);
    operators.add(">=", Db::FILTER_COLUMN_GE);
    operators.add("<", Db::FILTER_COLUMN_LT);
    operators.add("<=", Db::FILTER_COLUMN_LE);

    setsOperators.add("contains", Db::FILTER_COLUMN_CONTAINS);
    setsOperators.add("not_contains", Db::FILTER_COLUMN_NOT_CONTAINS);

    // When parsing some input using Nabialek trick, the rule, that is using the symbols table will not be entered when
    // the keyword is not found in the table. The eps is there to ensure, that the start rule will be entered every
    // time and so the error handler for bad keywords could be bound to it. The eoi rule is there to avoid the grammar
    // require more input on the end of the line, which is side effect of eps usage in this way.
    start %= (eps(!_a) > dispatch >> -eoi[_a = true]);

    dispatch %= dispatchAll | dispatchSets;

    // Attribute name recognized -> try to parse filter value. The raw function is here to get the name of the
    // attribute being parsed.
    dispatchAll = (raw[attributes[_a = _1]][rangeToString(_1, phoenix::ref(currentAttributeName))] >> operators[_b = _1]
        > lazy(_a)[_val = phoenix::bind(&FilterExpressionsParser<Iterator>::constructFilter, this,
            _b, phoenix::ref(currentAttributeName), phoenix::construct<Db::Value>(_1))]);
    dispatchSets = (raw[sets[_a = _1]][rangeToString(_1, phoenix::ref(currentAttributeName))] >> setsOperators[_b = _1]
        > lazy(_a)[_val = phoenix::bind(&FilterExpressionsParser<Iterator>::constructFilter, this,
            _b, phoenix::ref(currentAttributeName), phoenix::construct<Db::Value>(_1))]);

    phoenix::function<AttributeErrorHandler<Iterator> > attributeErrorHandler = AttributeErrorHandler<Iterator>();
    phoenix::function<ValueErrorHandler<Iterator> > valueErrorHandler = ValueErrorHandler<Iterator>();
    on_error<fail>(start, attributeErrorHandler(_1, _2, _3, _4,
                                                phoenix::ref(attributes), phoenix::ref(m_name), m_parent));
    on_error<fail>(dispatch, valueErrorHandler(_1, _2, _3, _4, phoenix::ref(currentAttributeName), m_parent));
}



template <typename Iterator>
void FilterExpressionsParser<Iterator>::addAtrributeToFilter(const Db::Identifier &kindName, const Db::Identifier &attributeName,
                                                   qi::rule<Iterator, Db::Value(), ascii::space_type> attributeParser)
{
    attributes.add(attributeName, attributeParser);
    attrKind[attributeName] = kindName;
}



template <typename Iterator>
void FilterExpressionsParser<Iterator>::addIdentifiersSetToFilter(const Db::Identifier &kindName, const Db::Identifier &setName,
                                         qi::rule<Iterator, Db::Identifier(), ascii::space_type> identifierParser)
{
    sets.add(setName, identifierParser);
    attrKind[setName] = kindName;
}



template <typename Iterator>
Db::Filter FilterExpressionsParser<Iterator>::constructFilter(Db::ComparisonOperator op, const Db::Identifier &attribute,
                                                              const Db::Value &value)
{
    return Db::AttributeExpression(op, attrKind[attribute], attribute, value);
}



/////////////////////////Template instances for linker//////////////////////////

template FilterExpressionsParser<iterator_type>::FilterExpressionsParser(const Db::Identifier &kindName, ParserImpl<iterator_type> *parent);

template void FilterExpressionsParser<iterator_type>::addAtrributeToFilter(const Db::Identifier &kindName, const Db::Identifier &attributeName, qi::rule<iterator_type, Db::Value(), ascii::space_type> attributeParser);

template void FilterExpressionsParser<iterator_type>::addIdentifiersSetToFilter(const Db::Identifier &kindName, const Db::Identifier &setName, qi::rule<iterator_type, Db::Identifier(), ascii::space_type> identifierParser);

template Db::Filter FilterExpressionsParser<iterator_type>::constructFilter(Db::ComparisonOperator op, const Db::Identifier &attribute, const Db::Value &value);

}
}
