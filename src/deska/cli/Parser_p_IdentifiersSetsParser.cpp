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
#include "Parser_p_IdentifiersSetsParser.h"

namespace Deska
{
namespace Cli
{


template <typename Iterator>
IdentifiersSetsParser<Iterator>::IdentifiersSetsParser(const Db::Identifier &kindName, ParserImpl<Iterator> *parent):
    IdentifiersSetsParser<Iterator>::base_type(start), m_name(kindName), m_parent(parent)
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

    // If the boost::spirit::qi::grammar API was sane, the following line would read setName(kindName).
    // The API is not sane, and therefore we have the following crap here.
    this->name(kindName);

    phoenix::function<RangeToString<Iterator> > rangeToString = RangeToString<Iterator>();

    // When parsing some input using Nabialek trick, the rule, that is using the symbols table will not be entered when
    // the keyword is not found in the table. The eps is there to ensure, that the start rule will be entered every
    // time and so the error handler for bad keywords could be bound to it. The eoi rule is there to avoid the grammar
    // require more input on the end of the line, which is side effect of eps usage in this way.
    start = (qi::lit("add") > dispatchAdd)
          | (qi::lit("remove") > dispatchRemove);

    // Set name recognized -> try to parse identifier. The raw function is here to get the name of the
    // set which maniulations are being parsed.
    dispatchAdd = ((raw[sets[_a = _1]][rangeToString(_1, phoenix::ref(currentSetName))]
        > lazy(_a)[phoenix::bind(&IdentifiersSetsParser::parsedAdd, this,
            phoenix::ref(currentSetName), _1)]));
    dispatchRemove = ((raw[sets[_a = _1]][rangeToString(_1, phoenix::ref(currentSetName))]
        > lazy(_a)[phoenix::bind(&IdentifiersSetsParser::parsedRemove, this,
            phoenix::ref(currentSetName), _1)]));

    phoenix::function<IdentifiersSetsErrorHandler<Iterator> > identifiersSetsErrorHandler =
        IdentifiersSetsErrorHandler<Iterator>();
    phoenix::function<ValueErrorHandler<Iterator> > valueErrorHandler = ValueErrorHandler<Iterator>();
    on_error<fail>(start, identifiersSetsErrorHandler(_1, _2, _3, _4,
                                                      phoenix::ref(sets), phoenix::ref(m_name), m_parent));
    on_error<fail>(dispatchAdd, valueErrorHandler(_1, _2, _3, _4, phoenix::ref(currentSetName),
        phoenix::ref(m_name), m_parent));
    on_error<fail>(dispatchRemove, valueErrorHandler(_1, _2, _3, _4, phoenix::ref(currentSetName),
        phoenix::ref(m_name), m_parent));
}



template <typename Iterator>
void IdentifiersSetsParser<Iterator>::addIdentifiersSet(const Db::Identifier &kindName, const Db::Identifier &setName,
                                        qi::rule<Iterator, Db::Identifier(), ascii::space_type> identifierParser)
{
    sets.add(setName, identifierParser);
    attrKind[setName] = kindName;
}



template <typename Iterator>
void IdentifiersSetsParser<Iterator>::parsedAdd(const Db::Identifier &parameter, Db::Identifier &value)
{
    m_parent->attributeSetInsert(attrKind[parameter], parameter, value);
}



template <typename Iterator>
void IdentifiersSetsParser<Iterator>::parsedRemove(const Db::Identifier &parameter, Db::Identifier &value)
{
    m_parent->attributeSetRemove(attrKind[parameter], parameter, value);
}



/////////////////////////Template instances for linker//////////////////////////

template IdentifiersSetsParser<iterator_type>::IdentifiersSetsParser(const Db::Identifier &kindName, ParserImpl<iterator_type> *parent);

template void IdentifiersSetsParser<iterator_type>::addIdentifiersSet(const Db::Identifier &kindName, const Db::Identifier &setName, qi::rule<iterator_type, Db::Identifier(), ascii::space_type> identifierParser);

template void IdentifiersSetsParser<iterator_type>::parsedAdd(const Db::Identifier &parameter, Db::Identifier &value);

template void IdentifiersSetsParser<iterator_type>::parsedRemove(const Db::Identifier &parameter, Db::Identifier &value);

}
}
