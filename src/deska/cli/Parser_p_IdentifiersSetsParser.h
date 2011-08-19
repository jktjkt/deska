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

#ifndef DESKA_CLI_PARSERPRIVATE_ATTRIBUTESSETSPARSER_H
#define DESKA_CLI_PARSERPRIVATE_ATTRIBUTESSETSPARSER_H

#include "Parser_p.h"

namespace Deska
{
namespace Cli
{

/** @short Parser for inserting and removing identifiers into attritubes of type TYPE_IDENTIFIER_SET.
*
*   This grammar parses only one pair from set of <set_name identifier> definitions.
*   For parsing set of thees pairs, use some boost::spirit operator like kleene star.
*
*   The grammar is based on a symbols table with lazy lookup function. This method could be found
*   under name "Nabialek trick". Each parsed pair is sent to the main parser (parent) using semantic
*   action parsedAdd() for adding item into the set or parsedRemove() for item removal.
*
*   This parser is connected to two error handlers. IdentifiersSetsErrorHandler for reporting an error while parsing
*   a set name and ValueErrorHandler for reporting an error while parsing a value on an attribute.
*
*   @see IdentifiersSetsErrorHandler
*   @see ValueErrorHandler
*/
template <typename Iterator>
class IdentifiersSetsParser: public qi::grammar<Iterator, ascii::space_type>
{

public:

    /** @short Constructor only initializes the grammar with empty symbols table.
    *
    *   @param kindName Name of top-level object type, to which the attributes belong.
    *   @param parent Pointer to main parser for calling its functions as semantic actions.
    */
    IdentifiersSetsParser(const Db::Identifier &kindName, ParserImpl<Iterator> *parent);

    /** @short Function used for filling of symbols table of the parser.
    *
    *   @param attributeName Name of the set.
    *   @param identifierParser Identifier parser obtained from PredefinedRules class.
    *   @see PredefinedRules
    */
    void addIdentifiersSet(const Db::Identifier &setName,
                           qi::rule<Iterator, Db::Identifier(), ascii::space_type> identifierParser);

private:

    /** @short Function used as semantic action for each parsed insert into the set.
    *
    *   Calls appropriate method in main parser.
    *
    *   @param parameter Name of the set.
    *   @param value Parsed identifier to add to the set.
    */
    void parsedAdd(const Db::Identifier &parameter, Db::Identifier &value);

    /** @short Function used as semantic action for each parsed remove from the set.
    *
    *   Calls appropriate method in main parser.
    *
    *   @param parameter Name of the set.
    *   @param value Parsed identifier to remove from the set.
    */
    void parsedRemove(const Db::Identifier &parameter, Db::Identifier &value);

    /** Attribute name - identifier pairs definitions for purposes of Nabialek trick. */
    qi::symbols<char, qi::rule<Iterator, Db::Identifier(), ascii::space_type> > sets;

    /** Rule for parsing sets names. */
    qi::rule<Iterator, ascii::space_type> start;
    /** Rule for parsing identifiers. */
    qi::rule<Iterator, ascii::space_type, qi::locals<qi::rule<Iterator, Db::Identifier(), ascii::space_type> > > dispatchAdd;
    qi::rule<Iterator, ascii::space_type, qi::locals<qi::rule<Iterator, Db::Identifier(), ascii::space_type> > > dispatchRemove;

    /** Name of set which items adition or removal is being currently parsed.
    *   This variable is used for error handling.
    */
    Db::Identifier currentSetName;
    /** Name of the top-level object, whose set manipulations are parsed by this grammar.
    *   This variable is used for error handling.
    */
    Db::Identifier m_name;

    /** Pointer to main parser for calling its functions as semantic actions. */
    ParserImpl<Iterator> *m_parent;
};

}
}

#endif  // DESKA_CLI_PARSERPRIVATE_ATTRIBUTESSETSPARSER_H
