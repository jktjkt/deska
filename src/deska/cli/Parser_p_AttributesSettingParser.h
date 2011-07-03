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

#ifndef DESKA_CLI_PARSERPRIVATE_ATTRIBUTESSETTINGPARSER_H
#define DESKA_CLI_PARSERPRIVATE_ATTRIBUTESSETTINGPARSER_H

#include "Parser_p.h"

namespace Deska
{
namespace Cli
{

/** @short Parser for set of attributes of specific top-level grammar.
*
*   This grammar parses only one pair from set of <attribute_name attribute_value> definitions.
*   For parsing set of thees pairs, use some boost::spirit operator like kleene star.
*
*   The grammar is based on a symbols table with lazy lookup function. This method could be found
*   under name "Nabialek trick". Each parsed pair is sent to the main parser (parent) using semantic
*   action parsedAttribute().
*
*   This parser is connected to two error handlers. AttributeErrorHandler for reporting an error while parsing
*   an attribute name and ValueErrorHandler for reporting an error while parsing a value on an attribute.
*
*   @see AttributeErrorHandler
*   @see ValueErrorHandler
*/
template <typename Iterator>
class AttributesSettingParser: public qi::grammar<Iterator, ascii::space_type, qi::locals<bool> >
{

public:

    /** @short Constructor only initializes the grammar with empty symbols table.
    *
    *   @param kindName Name of top-level object type, to which the attributes belong.
    *   @param parent Pointer to main parser for calling its functions as semantic actions.
    */
    AttributesSettingParser(const Db::Identifier &kindName, ParserImpl<Iterator> *parent);

    /** @short Function used for filling of symbols table of the parser.
    *
    *   @param attributeName Name of the attribute.
    *   @param attributeParser Attribute parser obtained from PredefinedRules class.
    *   @see PredefinedRules
    */
    void addAtrribute(const Db::Identifier &attributeName,
                      qi::rule<Iterator, Db::Value(), ascii::space_type> attributeParser);

private:

    /** @short Function used as semantic action for each parsed attribute.
    *
    *   Calls appropriate method in main parser.
    *
    *   @param parameter Name of the attribute.
    *   @param value Parsed value of the attribute.
    *   @see Db::Value
    */
    void parsedAttribute(const Db::Identifier &parameter, Db::Value &value);

    /** Attribute name - attribute value type pairs definitions for purposes of Nabialek trick. */
    qi::symbols<char, qi::rule<Iterator, Db::Value(), ascii::space_type> > attributes;

    /** Rule for parsing attribute names. */
    qi::rule<Iterator, ascii::space_type, qi::locals<bool> > start;
    /** Rule for parsing attribute values. */
    qi::rule<Iterator, ascii::space_type, qi::locals<qi::rule<Iterator, Db::Value(), ascii::space_type> > > dispatch;

    /** Name of attribute which value is being currently parsed. This variable is used for error handling. */
    Db::Identifier currentAttributeName;
    /** Name of the top-level object, whose attributes are parsed by this grammar.
    *   This variable is used for error handling.
    */
    Db::Identifier m_name;

    /** Pointer to main parser for calling its functions as semantic actions. */
    ParserImpl<Iterator> *m_parent;
};

}
}

#endif  // DESKA_CLI_PARSERPRIVATE_ATTRIBUTESSETTINGPARSER_H
