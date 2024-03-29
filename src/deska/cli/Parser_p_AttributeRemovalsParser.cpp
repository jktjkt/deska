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
#include "Parser_p_AttributeRemovalsParser.h"
#include "Parser_p.h"

namespace Deska
{
namespace Cli
{


template <typename Iterator>
AttributeRemovalsParser<Iterator>::AttributeRemovalsParser(const Db::Identifier &kindName, ParserImpl<Iterator> *parent):
    AttributeRemovalsParser<Iterator>::base_type(start), m_name(kindName), m_parent(parent)
{
    using qi::_1;
    using qi::_2;
    using qi::_3;
    using qi::_4;
    using qi::_a;
    using qi::eps;
    using qi::raw;
    using qi::on_error;
    using qi::fail;

    // If the boost::spirit::qi::grammar API was sane, the following line would read setName(kindName).
    // The API is not sane, and therefore we have the following crap here.
    this->name(kindName);

    namespace phoenix = boost::phoenix;
    phoenix::function<RangeToString<Iterator> > rangeToString = RangeToString<Iterator>();

    start = (keyword["no"] > dispatch);

    dispatch = raw[keyword[attributes[_a = _1]]][rangeToString(_1, phoenix::ref(currentAttributeName))] > lazy(_a)
        [phoenix::bind(&AttributeRemovalsParser::parsedAttributeRemoval, this,
            phoenix::ref(currentAttributeName))];

    phoenix::function<AttributeRemovalErrorHandler<Iterator> > attributeRemovalErrorHandler =
        AttributeRemovalErrorHandler<Iterator>();

    on_error<fail>(start, attributeRemovalErrorHandler(_1, _2, _3, _4,
                                                       phoenix::ref(attributes), phoenix::ref(m_name), m_parent));
}



template <typename Iterator>
void AttributeRemovalsParser<Iterator>::addAtrribute(const Db::Identifier &kindName, const Db::Identifier &attributeName)
{
    attributes.add(attributeName, qi::eps);
    attrKind[attributeName] = kindName;
}



template <typename Iterator>
void AttributeRemovalsParser<Iterator>::parsedAttributeRemoval(const Db::Identifier &attribute)
{
    m_parent->attributeRemove(attrKind[attribute], attribute);
}



/////////////////////////Template instances for linker//////////////////////////

template AttributeRemovalsParser<iterator_type>::AttributeRemovalsParser(const Db::Identifier &kindName, ParserImpl<iterator_type> *parent);

template void AttributeRemovalsParser<iterator_type>::addAtrribute(const Db::Identifier &kindName, const Db::Identifier &attributeName);

template void AttributeRemovalsParser<iterator_type>::parsedAttributeRemoval(const Db::Identifier &parameter);


}
}
