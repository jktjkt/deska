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

#ifndef DESKA_PARSER_H
#define DESKA_PARSER_H

#include <string>
#include <boost/noncopyable.hpp>
#include <boost/signal.hpp>
#include "deska/db/Api.h"

namespace Deska {
namespace CLI {

/** @short Process the CLI input and generate events based on the parsed data

This class is fed by individual lines of user input (or the configuration file). It will parse
these data and emit signals based on their meaning, similar to how a SAX XML parser processes an
XML file.

Example: suppose we read the following input; the comments indicate the signals which the Parser emits:

    # isNestedInContext() return false
    host hpv2
        # categoryEntered("host", "hpv2") gets called
        # isNestedInContext() would return true
        # currentContextStack() would return (("host", "hpv2"))
        color red
        # attributeSet("color", "red")
    end
    # categoryLeft() gets signalled, isNestedInContext() is false and the currentContextStack() is empty

    host hpv2
        # categoryEntered("host", "hpv2") gets called
        # isNestedInContext() would return true
        # currentContextStack() would return (("host", "hpv2"))
        depth 10
        # attributeSet("depth", "10")
        interface eth0
            # categoryEntered("interface", "eth0") gets called
            # isNestedInContext() would return true
            # currentContextStack() would return (("host", "hpv2"), ("interface", "eth0"))
            ip 1.2.3.4
            # attributeSet("ip", "1.2.3.4")
        end
        # categoryLeft()
        # isNestedInContext() would return true
        # currentContextStack() would return (("host", "hpv2"))
    end
    # categoryLeft() gets signalled, isNestedInContext() is false and the currentContextStack() is empty


The following example demonstrates statements "inlined" to just a single line; we can see that the terminate
"end" is not required in these cases. The time goes from left to right. The whole line, ie. ("host hpv2 color blue\n")
is passed to the parser as a single unit, without any further structure.

    host hpv2 color blue
             ^          ^-- (2) Here it emits setAttribute("color", "blue") immediately followed by categoryLeft()
             |              the Parser returns to the initial state
             +-- (1) Here it emits the categoryEntered("host", "hpv2")

And another example, showing that it's possible to set multiple attributes at once:

    host hpv2 color blue depth 1
             ^          ^       ^+-- (3) setAttribute("depth", 1) immediately followed by categoryLeft()
             |          |
             |          +-- (2) setAttribute("color", "blue")
             |
             +-- (1) categoryEntered("host", "hpv2")

*/
class Parser: boost::noncopyable
{
public:
    /** @short Initialize the Parser with DB scheme information retrieved via the Deska API */
    Parser( Api* dbApi);
    virtual ~Parser();

    /** @short Parse a full line of user's input

As a result of this parsing, events could get triggered and the state may change.
 */
    void parseLine(const std::string &line);

    /** @short The input indicates that the following signals will be related to a particular object

This signal is emitted whenever the parsed text indicates that we should enter a "context", like when it
reads a line like "host hpv2".  The first argument is the name of the object kind ("hardware" in this case)
and the second one is the object's identifier ("hpv2").
*/
    boost::signal<void (const Identifier &kind, const Identifier &name)> categoryEntered;

    /** @short Leaving a context

The Parser hit a line indicating that the current block hsould be left. This could be a result of an explicit
"end" line, or a side effect of a standalone, self-contained line.
*/
    boost::signal<void ()> categoryLeft;

    /** @short Set an object's attribute

This signal is triggered whenever an attribute definition is encountered. The first argument is the name
of the attribute and the second one the attribute value.
 */
    boost::signal<void (const Identifier &name, const Value &value)> attributeSet;

    /** @short True if the parser is currently nested in some block

The return value is false iff the currentContextStack() would return an empty vector.
*/
    bool isNestedInContext() const;

    /** @short Return current nesting of the contexts

The return value is a vector of items where each item indicates one level of context nesting. The first member
of the pair represents the object kind and the second one contains the object's identifier.
*/
    std::vector<std::pair<Identifier,Identifier> > currentContextStack() const;


private:
    Api *m_dbApi;
};

}

}



#endif  //DESKA_PARSER_H
