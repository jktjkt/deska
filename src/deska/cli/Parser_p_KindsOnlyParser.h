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

#ifndef DESKA_CLI_PARSERPRIVATE_KINDSONLYPARSER_H
#define DESKA_CLI_PARSERPRIVATE_KINDSONLYPARSER_H

#include "Parser_p.h"
#include "ParserErrors.h"
#include "Exceptions.h"

namespace Deska
{
namespace Cli
{

/** @short Parser for kinds definitions.
*
*   This grammar parses only one pair from set of <kind_name object_name> definitions.
*
*   The grammar is based on a symbols table with lazy lookup function. This method could be found under name
*   "Nabialek trick". Each parsed pair is sent to the main parser (parent) using semantic action parsedKind().
*
*   This parser is connected to two error handlers. KindErrorHandler for reporting an error while parsing a kind name
*   and ValueErrorHandler for reporting an error while parsing an object name.
*
*   @see KindErrorHandler
*   @see ValueErrorHandler
*/
template <typename Iterator>
class KindsOnlyParser: public qi::grammar<Iterator, ascii::space_type, qi::locals<bool> >
{

public:

    /** @short Constructor only initializes the grammar with empty symbols table.
    *
    *   @param kindName Name of top-level object type, to which the parser belongs in case of parser for nested kinds.
    *   @param parent Pointer to main parser for calling its functions as semantic actions.
    */
    KindsOnlyParser(const Db::Identifier &kindName, ParserImpl<Iterator> *parent);

    /** @short Function used for filling of symbols table of the parser.
    *
    *   @param kindName Name of the kind.
    *   @param identifierParser Identifier parser obtained from PredefinedRules class.
    */
    void addKind(const Db::Identifier &kindName,
                 qi::rule<Iterator, Db::Identifier(), ascii::space_type> identifierParser);

private:

    /** @short Function used as semantic action for parsed kind.
    *
    *   Calls appropriate method in main parser and updates context stack.
    *
    *   @param kindName Name of the kind.
    *   @param objectName Parsed name of the object.
    */
    void parsedKind(const Db::Identifier &kindName, const Db::Identifier &objectName);

    /** Kind name - identifier type pairs definitions for purposes of Nabialek trick. */
    qi::symbols<char, qi::rule<Iterator, Db::Identifier(), ascii::space_type> > kinds;

    /** Rule for parsing kind names. */
    qi::rule<Iterator, ascii::space_type, qi::locals<bool> > start;
    /** Rule for parsing object names. */
    qi::rule<Iterator, ascii::space_type,
             qi::locals<qi::rule<Iterator, Db::Identifier(), ascii::space_type> > > dispatch;

    /** Name of kind which identifier is being currently parsed. This variable is used for error handling. */
    Db::Identifier currentKindName;
    /** Name of the top-level object, whose nested kind definitions are parsed by this grammar.
    *   This variable is used for error handling.
    */
    Db::Identifier m_name;

    /** Pointer to main parser for calling its functions as semantic actions. */
    ParserImpl<Iterator> *m_parent;
};

}
}

#endif  // DESKA_CLI_PARSERPRIVATE_KINDSONLYPARSER_H
