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

#ifndef DESKA_CLI_PARSERPRIVATE_ATTRIBUTESREMOVAL_H
#define DESKA_CLI_PARSERPRIVATE_ATTRIBUTESREMOVAL_H

#include "Parser_p.h"

namespace Deska
{
namespace Cli
{

/** @short Parser for set of attribute removals of specific top-level grammar.
*
*   This grammar parses only one pair <"no" attribute_name>.
*   For parsing set of thees pairs, use some boost::spirit operator like kleene star.
*/
template <typename Iterator>
class AttributeRemovalsParser: public qi::grammar<Iterator, ascii::space_type>
{

public:

    /** @short Constructor only initializes the grammar with empty symbols table.
    *
    *   @param kindName Name of top-level object type, to which the attributes belong.
    *   @param parent Pointer to main parser for calling its functions as semantic actions.
    */
    AttributeRemovalsParser(const Db::Identifier &kindName, ParserImpl<Iterator> *parent);

    /** @short Function used for filling of symbols table of the parser.
    *
    *   @param attributeName Name of the attribute.
    */
    void addAtrribute(const Db::Identifier &attributeName);

private:

    /** @short Function used as semantic action for each parsed attribute removal.
    *
    *   Calls appropriate method in main parser.
    *
    *   @param attribute Name of the attribute.
    */
    void parsedAttributeRemoval(const Db::Identifier &attribute);

    /** Attribute name - attribute value type pairs definitions for purposes of Nabialek trick. */
    qi::symbols<char, qi::rule<Iterator, ascii::space_type> > attributes;

    /** Rule for parsing "no" keyword. */
    qi::rule<Iterator, ascii::space_type> start;
    /** Rule for parsing attribute names. */
    qi::rule<Iterator, ascii::space_type, qi::locals<qi::rule<Iterator, ascii::space_type> > > dispatch;

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

#endif  // DESKA_CLI_PARSERPRIVATE_ATTRIBUTESREMOVAL_H
