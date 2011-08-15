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

#ifndef DESKA_CLI_PARSERPRIVATE_KINDSCONSTRUCTPARSER_H
#define DESKA_CLI_PARSERPRIVATE_KINDSCONSTRUCTPARSER_H

#include <boost/spirit/include/qi.hpp>
#include "deska/db/Objects.h"

namespace Deska
{
namespace Cli
{

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
template <typename Iterator> class ParserImpl;

/** @short Parser for set of object constructors for each kind.
*
*   This grammar parses only one pair <"new" kind_name>.
*   For parsing set of thees pairs, use some boost::spirit operator like kleene star.
*
*   The grammar is based on a symbols table with lazy lookup function. This method could be found
*   under name "Nabialek trick". Each parsed pair is sent to the main parser (parent) using semantic
*   action parsedObjectCreation().
*

// TODO: Error handling

*   This parser is connected to one error handler AttributeRemovalErrorHandler for reporting an error
*   while parsing an attribute name.
*
*   @see AttributeRemovalErrorHandler
*/
template <typename Iterator>
class KindsConstructParser: public qi::grammar<Iterator, ascii::space_type>
{

public:

    /** @short Constructor only initializes the grammar with empty symbols table.
    *
    *   @param kindName Name of top-level object type, to which the attributes belong.
    *   @param parent Pointer to main parser for calling its functions as semantic actions.
    */
    KindsConstructParser(const Db::Identifier &kindName, ParserImpl<Iterator> *parent);

    /** @short Function used for filling of symbols table of the parser.
    *
    *   @param kindName Name of the kind.
    */
    void addKind(const Db::Identifier &kindName);

private:

    /** @short Function used as semantic action for each parsed object creation.
    *
    *   Calls appropriate method in main parser.
    *
    *   @param kind Name of the kind.
    */
    void parsedObjectCreation(const Db::Identifier &kind);

    /** Kind name definitions for purposes of Nabialek trick. */
    qi::symbols<char, qi::rule<Iterator, ascii::space_type> > kinds;

    /** Rule for parsing "new" keyword. */
    qi::rule<Iterator, ascii::space_type> start;
    /** Rule for parsing kind names. */
    qi::rule<Iterator, ascii::space_type, qi::locals<qi::rule<Iterator, ascii::space_type> > > dispatch;

    /** Name of kind which creation is being currently parsed. This variable is used for error handling. */
    Db::Identifier currentKindName;
    /** Name of the top-level object, whose nested kinds are parsed by this grammar.
    *   This variable is used for error handling.
    */
    Db::Identifier m_name;

    /** Pointer to main parser for calling its functions as semantic actions. */
    ParserImpl<Iterator> *m_parent;
};

}
}

#endif  // DESKA_CLI_PARSERPRIVATE_KINDSCONSTRUCTPARSER_H
