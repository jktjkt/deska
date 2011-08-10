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

#ifndef DESKA_CLI_PARSERPRIVATE_ATTRIBUTESPARSER_H
#define DESKA_CLI_PARSERPRIVATE_ATTRIBUTESPARSER_H

#include "Parser_p.h"

namespace Deska
{
namespace Cli
{

/** @short Parser for set of attributes specific top-level grammar.
*
*   Combines all needed grammars into one parser for parsing the attributes setting and removing of one kind.
*   For attribute setting is used grammar AttributesSettingParser and for attribute removing grammar AttributeRemovalsParser.
*
*   @see AttributesSettingParser
*   @see AttributeRemovalsParser
*/
template <typename Iterator>
class AttributesParser: public qi::grammar<Iterator, ascii::space_type>
{

public:

    /** @short Constructor initializes the grammar with all rules.
    *
    *   @param kindName Name of top-level object type, to which the parser belongs.
    *   @param attributesSettingParser Grammar used for parsing of attributes of the kind.
    *   @param attributesRemovalParser Grammar used for parsing of attributes removals of the kind.
    *   @param identifiersSetsParser Grammar used for parsing of identifiers sets manipulations.
    *   @param parent Pointer to main parser for calling its functions as semantic actions.
    */
    AttributesParser(const Db::Identifier &kindName, AttributesSettingParser<Iterator> *attributesSettingParser,
                     AttributeRemovalsParser<Iterator> *attributeRemovalsParser,
                     IdentifiersSetsParser<Iterator> *identifiersSetsParser, ParserImpl<Iterator> *parent);

private:

    qi::rule<Iterator, ascii::space_type> start;

    /** Pointer to main parser for calling its functions as semantic actions. */
    ParserImpl<Iterator> *m_parent;
};

}
}

#endif  // DESKA_CLI_PARSERPRIVATE_ATTRIBUTESPARSER_H
