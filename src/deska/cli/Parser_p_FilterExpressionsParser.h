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

#ifndef DESKA_CLI_PARSERPRIVATE_FILTEREXPRESSIONSSPARSER_H
#define DESKA_CLI_PARSERPRIVATE_FILTEREXPRESSIONSSPARSER_H

#include "Parser_p.h"

namespace Deska
{
namespace Cli
{

/** @short Parser for expressions comparing attributes and values in a filter for specific kind.
*
*   The grammar is based on a symbols table with lazy lookup function. This method could be found
*   under name "Nabialek trick".
*
*   This parser is connected to two error handlers. AttributeErrorHandler for reporting an error while parsing
*   an attribute name and ValueErrorHandler for reporting an error while parsing a value on an attribute.
*
*   @see AttributeErrorHandler
*   @see ValueErrorHandler
*/
template <typename Iterator>
class FilterExpressionsParser: public qi::grammar<Iterator, Db::Filter(), ascii::space_type, qi::locals<bool> >
{

public:

    /** @short Constructor only initializes the grammar with empty symbols table.
    *
    *   @param kindName Name of top-level object type, to which the attributes belong.
    *   @param parent Pointer to main parser for error reporting purposes.
    */
    FilterExpressionsParser(const Db::Identifier &kindName, ParserImpl<Iterator> *parent);

    /** @short Function used for filling of symbols table of the parser.
    *
    *   @param attributeName Name of the attribute.
    *   @param attributeParser Attribute parser obtained from PredefinedRules class.
    *   @see PredefinedRules
    */
    void addAtrributeToFilter(const Db::Identifier &attributeName,
                              qi::rule<Iterator, Db::Value(), ascii::space_type> attributeParser);

private:

    /** Symbols table with comparison operators. */
    qi::symbols<char, Db::ComparisonOperator> operators;

    /** Attribute name - attribute value type pairs definitions for purposes of Nabialek trick. */
    qi::symbols<char, qi::rule<Iterator, Db::Value(), ascii::space_type> > attributes;

    /** Rule for parsing attribute names. */
    qi::rule<Iterator, Db::Filter(), ascii::space_type, qi::locals<bool> > start;
    /** Rule for parsing attribute values. */
    qi::rule<Iterator, Db::Filter(), ascii::space_type,
             qi::locals<qi::rule<Iterator, Db::Value(), ascii::space_type>, Db::ComparisonOperator> > dispatch;

    /** Name of attribute which value is being currently parsed. This variable is used for error handling. */
    Db::Identifier currentAttributeName;
    /** Name of the kind, whose filter is parsed by this grammar. */
    Db::Identifier m_name;
    /** Pointer to main parser for error reporting. */
    ParserImpl<Iterator> *m_parent;
};

}
}

#endif  // DESKA_CLI_PARSERPRIVATE_FILTEREXPRESSIONSSPARSER_H
