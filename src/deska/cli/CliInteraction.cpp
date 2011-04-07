/* Copyright (C) 2011 Jan Kundrát <kundratj@fzu.cz>
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

#include <iostream>
#include <sstream>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include "CliInteraction.h"
#include "Parser.h"
#include "Exceptions.h"
#include "deska/db/Api.h"

namespace Deska
{
namespace Cli
{

std::ostream &operator<<(std::ostream &stream, const std::vector<ContextStackItem> &stack)
{
    for (std::vector<ContextStackItem>::const_iterator it = stack.begin(); it != stack.end(); ++it) {
        if (it != stack.begin())
            stream << " -> ";
        stream << it->kind << " " << it->name;
    }
    return stream;
}


CliInteraction::CliInteraction(Db::Api *api, Parser *parser):
    m_api(api), m_parser(parser), m_ignoreParserActions(false)
{
    using boost::phoenix::bind;
    using boost::phoenix::arg_names::_1;
    using boost::phoenix::arg_names::_2;
    m_parser->parseError.connect(bind(&CliInteraction::slotParserError, this, _1));
    // got to fully quallify bind on next lines :(
    m_parser->attributeSet.connect(boost::phoenix::bind(&CliInteraction::slotSetAttribute, this, _1, _2));
    m_parser->categoryEntered.connect(boost::phoenix::bind(&CliInteraction::slotCategoryEntered, this, _1, _2));
}

void CliInteraction::run()
{
    std::string line;
    std::cout << "> ";
    std::vector<ContextStackItem> context;
    while ( getline( std::cin, line ) ) {
        if ( line.size() == 1 && ( line[ 0 ] == 'q' || line[ 0 ] == 'Q' ) )
            break;

        m_parser->parseLine(line);

        if (m_ignoreParserActions) {
            m_parser->clearContextStack();
        }

        context = m_parser->currentContextStack();
        std::cout << context << "> ";
    }
}

void CliInteraction::slotParserError(const ParserException &e)
{
    std::cout << "Parse error: " << e.dump() << std::endl;
}

void CliInteraction::slotSetAttribute(const Db::Identifier &name, const Db::Value &value)
{
    if (m_ignoreParserActions)
        return;

    std::cout << "Setting attribute " << m_parser->currentContextStack() << " -> " << name << " to '" << value << "'" << std::endl;

}

void CliInteraction::slotCategoryEntered(const Db::Identifier &kind, const Db::Identifier &name)
{
    // We're entering into some context, so we should check whether the object in question exists, and if it does not,
    // ask the user whether to create it
    std::vector<Deska::Db::Identifier> kindInstances = m_api->kindInstances(kind);
    if (std::find(kindInstances.begin(), kindInstances.end(), name) == kindInstances.end()) {
        // Object does not exist -> ask the user here
        std::ostringstream ss;
        ss << kind << " " << name << " does not exist. Create?";
        if (askForConfirmation(ss.str())) {
            std::cout << "Creating " << kind << " " << name << "." << std::endl;
            m_api->createObject(kind, name);
        } else {
            std::cout << "Ignoring the rest of this line." << std::endl;
            m_ignoreParserActions = true;
        }
    }
}

bool CliInteraction::askForConfirmation(const std::string &prompt)
{
    std::cout << prompt << " ";
    std::string answer;
    std::cin >> answer;
    std::cout << std::endl;
    boost::algorithm::to_lower(answer);
    return answer == "yes" || answer == "y";
}

}
}
