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

#ifndef DESKA_CLI_PARSERPRIVATE_FILTERSPARSER_H
#define DESKA_CLI_PARSERPRIVATE_FILTERSPARSER_H

#include "Parser_p.h"

namespace Deska
{
namespace Cli
{

template <typename Iterator> class FilterExpressionsParser;

/** @short Parser for filter of specific kind.
*
*   The grammar is based on a symbols table with lazy lookup function. This method could be found
*   under name "Nabialek trick".
*
*   This parser is connected to one error handler. KindFiltersErrorHandler for reporting an error while parsing
*   name of nested kind.
*
*   @see KindFiltersErrorHandler
*/
template <typename Iterator>
class FiltersParser: public qi::grammar<Iterator, Db::Filter(), ascii::space_type>
{

public:

    /** @short Constructor only initializes the grammar with empty symbols table.
    *
    *   @param kindName Name of top-level object type, to which the attributes belong.
    *   @param ownAttrsExpressionsParser Parser for parsing expressions containing own attributes, not nested ones.
    *   @param parent Pointer to main parser for error reporting purposes.
    */
    FiltersParser(const Db::Identifier &kindName, FilterExpressionsParser<Iterator> *ownAttrsExpressionsParser,
                  ParserImpl<Iterator> *parent);

    /** @short Function used for filling of symbols table of the parser.
    *
    *   @param nestedKindName Name of nested kind.
    *   @param expressionsParser Filter expressions parser for the nested kind.
    */
    void addNestedKindExpressionsParser(const Db::Identifier &nestedKindName,
                                        FilterExpressionsParser<Iterator> *expressionsParser);

private:

    /** Nested kind name - expressions parser pairs for purposes of Nabialek trick. */
    qi::symbols<char, qi::rule<Iterator, Db::Filter(), ascii::space_type> > nestedAttributes;

    /** Main rule. */
    qi::rule<Iterator, Db::Filter(), ascii::space_type> start;

    qi::rule<Iterator, Db::Filter(), ascii::space_type> andFilter;
    qi::rule<Iterator, Db::Filter(), ascii::space_type> orFilter;

    qi::rule<Iterator, Db::Filter(), ascii::space_type> attrExpr;

    /** Rule for parsing nested kind names in a filter. */
    qi::rule<Iterator, Db::Filter(), ascii::space_type, qi::locals<bool> > nestedAttrExpr;
    /** Rule for parsing expressions for attributes from nested kinds names. */
    qi::rule<Iterator, Db::Filter(), ascii::space_type,
             qi::locals<qi::rule<Iterator, Db::Filter(), ascii::space_type> > > dispatch;

    /** Name of the kind, whose filter is parsed by this grammar. */
    Db::Identifier m_name;
    /** Name of kind which filter expression is being currently parsed. */
    Db::Identifier nestedKindName;
    /** Pointer to main parser for error reporting. */
    ParserImpl<Iterator> *m_parent;
};

}
}

#endif  // DESKA_CLI_PARSERPRIVATE_FILTERSPARSER_H
