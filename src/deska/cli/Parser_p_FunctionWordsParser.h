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

#ifndef DESKA_CLI_PARSERPRIVATE_FUNCTIONWORDSPARSER_H
#define DESKA_CLI_PARSERPRIVATE_FUNCTIONWORDSPARSER_H

#include "Parser_p.h"

namespace Deska
{
namespace Cli
{

/** @short Parser for parsing function words, that can be typed at the beginning of any line.
*
*   Parser works as alternatives parser with all words inside and invokes appropriate actions using semantic functions.
*/
template <typename Iterator>
class FunctionWordsParser: public qi::grammar<Iterator, ascii::space_type>
{

public:

    /** @short Constructor initializes the grammar with all rules.
    *
    *   @param parent Pointer to main parser for calling its functions as semantic actions.
    */
    FunctionWordsParser(ParserImpl<Iterator> *parent);

private:

    /** @short Function used as semantic action for parsed "delete" function word. */
    void actionDelete();

    /** @short Function used as semantic action for parsed "show" function word. */
    void actionShow();

    /** @short Function used as semantic action for parsed "rename" function word. */
    void actionRename();

    qi::rule<Iterator, ascii::space_type > start;

    /** Pointer to main parser for calling its functions as semantic actions. */
    ParserImpl<Iterator> *m_parent;
};

}
}

#endif  // DESKA_PARSERPRIVATE_H
