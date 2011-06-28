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

#include <iosfwd>
#include <boost/noncopyable.hpp>
#include <boost/signals2.hpp>

#include "deska/db/Objects.h"

namespace Deska {
namespace Db {
class Api;
}

namespace Cli {

class ParserException;

/** @short INTERNAL; Iterator for parser input */
typedef std::string::const_iterator iterator_type;
template<typename Iterator> class ParserImpl;

/** @short Modes of parsing.
*   
*   Mode determines, what kind of input parser expects.
*/
typedef enum {
    /** @short Standard mode expects kinds including their attributes. */
    PARSING_MODE_STANDARD,
    /** @short Delete mode expects kinds with nested kinds with no attributes. */
    PARSING_MODE_DELETE,
    /** @short Show mode expects kinds with nested kinds with no attributes. */
    PARSING_MODE_SHOW,
    /** @short Rename mode expects kinds with nested kinds with no attributes and new object name at the end. */
    PARSING_MODE_RENAME
} ParsingMode;

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
    Parser(Db::Api* dbApi);

    virtual ~Parser();

    /** @short Obtains some help for usage of parser keywords.
    *   
    *   @return Map of keywords, where key is the keyword name and value is description of its usage.
    */
    std::map<std::string, std::string> parserKeywordsUsage();

    /** @short Obtains list of embedded kinds for given kind name.
    *   
    *   @param kindName Kind name for which the embedded kinds will be obtained
    *   @return Vector of embedded kinds
    */
    std::vector<Db::Identifier> parserKindsEmbeds(const Db::Identifier &kindName);

    /** @short Obtains list of attributes for given kind name.
    *   
    *   @param kindName Kind name for which the attributes will be obtained
    *   @return Vector of pairs, where first item is attribute name and second is attribute value type name
    */
    std::vector<std::pair<Db::Identifier, std::string> > parserKindsAttributes(const Db::Identifier &kindName);

    /** @short Parse a full line of user's input
    *
    *   As a result of this parsing, events could get triggered and the state may change.
    */
    void parseLine(const std::string &line);

    /** @short Get list of strings for tab completion of current line
    *
    *   @return Vector of strings, that are possible continuations of current line.
    */
    std::vector<std::string> tabCompletionPossibilities(const std::string &line);

    /** @short The input indicates that the following signals will be related to a particular object
    *
    *   This signal is emitted whenever the parsed text indicates that we should enter a "context", like when it
    *   reads a line like "host hpv2".  The first argument is the name of the object kind ("hardware" in this case)
    *   and the second one is the object's identifier ("hpv2").
    */
    boost::signals2::signal<void (const Db::Identifier &kind, const Db::Identifier &name)> categoryEntered;

    /** @short Leaving a context
    *
    *   The Parser hit a line indicating that the current block should be left. This could be a result of an explicit
    *   "end" line, or a side effect of a standalone, self-contained line.
    */
    boost::signals2::signal<void ()> categoryLeft;

    /** @short Set an object's attribute
    *
    *   This signal is triggered whenever an attribute definition is encountered. The first argument is the name
    *   of the attribute and the second one the attribute value.
    */
    boost::signals2::signal<void (const Db::Identifier &name, const Db::Value &value)> attributeSet;

    /** @short Remove an object's attribute
    *
    *   This signal is triggered whenever an attribute removal is encountered. The argument is the name
    *   of the attribute
    */
    boost::signals2::signal<void (const Db::Identifier &name)> attributeRemove;

    /** @short An error during parsing
    *
    *   The current user input has triggered an error during parsing.
    */
    boost::signals2::signal<void (const ParserException &err)> parseError;

    /** @short Show attributes and nested kinds of an object in the current context
    *
    *   The signal is triggered when keyword "show" was found on the beginning of the line
    */
    boost::signals2::signal<void ()> functionShow;

    /** @short Delete object in current context
    *
    *   The signal is triggered when keyword "delete" was found on the beginning of the line
    */
    boost::signals2::signal<void ()> functionDelete;

    /** @short Rename object in current context
    *
    *   The signal is triggered when keyword "rename" was found on the beginning of the line
    */
    boost::signals2::signal<void (const Db::Identifier &newName)> functionRename;

    /** @short Parsing of current line finished
    *
    *   The parser triggeres this signal when parsing of current line is finished successfully.
    */
    boost::signals2::signal<void ()> parsingFinished;

    /** @short True if the parser is currently nested in some block
    *
    *   The return value is false iff the currentContextStack() would return an empty vector.
    */
    bool isNestedInContext() const;

    /** @short Return current nesting of the contexts
    *
    *   The return value is a vector of items where each item indicates one level of context nesting. The first member
    *   of the pair represents the object kind and the second one contains the object's identifier.
    */
    Db::ContextStack currentContextStack() const;

    /** @short Replaces current context stack with new one.
    *
    *   @param stack Vector of object definitions representing new context stack
    */
    void setContextStack(const Db::ContextStack &stack);

    /** @short Moves context to top level */
    void clearContextStack();


private:
    friend class ParserImpl<iterator_type>;
    ParserImpl<iterator_type> *d_ptr;
    Db::Api *m_dbApi;
};

}

}



#endif  // DESKA_PARSER_H
