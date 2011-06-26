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

#ifndef DESKA_CLI_PARSERPRIVATE_TOPLEVELPARSER_H
#define DESKA_CLI_PARSERPRIVATE_TOPLEVELPARSER_H

#include "Parser_p.h"

namespace Deska
{
namespace Cli
{

/** @short Parser for set of top-level kind definitions or filters for all kinds.
*
*   Combines all needed grammars into one parser for parsing the top-level kinds or filters for all kinds.
*   For parsing of kind definitions is used grammar KindsOnlyParser, for parsing kind filters is used grammar
*   KindsFiltersParser.
*
*   @see KindsOnlyParser
*   @see KindsFiltersParser
*/
template <typename Iterator>
class TopLevelParser: public qi::grammar<Iterator, ascii::space_type>
{

public:

    /** @short Constructor initializes the grammar with all rules.
    *
    *   @param topLevelKinds Grammar used for parsing top level kinds definitions.
    *   @param topLevelKindsFilters Grammar used for parsing filters for all kinds.
    *   @param parent Pointer to main parser for calling its functions as semantic actions.
    */
    TopLevelParser(KindsOnlyParser<Iterator> *topLevelKinds, KindsFiltersParser<Iterator> *topLevelKindsFilters,
                   ParserImpl<Iterator> *parent);

private:

    /** @short Function used as semantic action when there is only single kind on the line
    *          and so parser should nest into this kind.
    *
    *   Calls appropriate method in main parser.
    */
    void parsedSingleKind();

    qi::rule<Iterator, ascii::space_type > start;

    /** Pointer to main parser for calling its functions as semantic actions. */
    ParserImpl<Iterator> *m_parent;
};

}
}

#endif  // DESKA_CLI_PARSERPRIVATE_TOPLEVELPARSER_H
