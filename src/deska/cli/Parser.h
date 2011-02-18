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
#include <map>

#include <boost/noncopyable.hpp>
#include <boost/signals2.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>

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


namespace spirit = boost::spirit;
namespace phoenix = boost::phoenix;
namespace ascii = boost::spirit::ascii;
namespace qi = boost::spirit::qi;


/** @short A type returned by parser when parsing attributes */
typedef boost::variant<int, std::string, double> Variant;


/** @short Class for reporting parsing errors of input */
template <typename Iterator>
class ErrorHandler
{
public:
    template <typename, typename, typename, typename>
        struct result { typedef void type; };

    /** @short Function executed when some error occures. Prints information about the error
    *
    *   @param start Begin of the input being parsed when the error occures
    *   @param end End of the input being parsed when the error occures
    *   @param errorPos Position where the error occures
    *   @param what Expected tokens
    */
    void operator()( Iterator start, Iterator end, Iterator errorPos, const spirit::info& what ) const;
};



/** @short Predefined rules for parsing single parameters */
template <typename Iterator>
class PredefinedRules
{

public:

    /** @short Fills internal map with predefined rules, that can be used to parse attributes of top-level objects */
    PredefinedRules();

    /** @short Function for getting single rules, that can be used in attributes grammar
    *
    *   @param typeName Supported rules are: integer, quoted_string, double, identifier
    *   @return Rule that parses specific type of attribute
    */
    qi::rule<Iterator, Variant(), ascii::space_type> getRule( const std::string &typeName );

private:

    std::map<std::string, qi::rule<Iterator, Variant(), ascii::space_type> > rulesMap;

};



/** @short Parser for set of attributes of specific top-level grammar */
template <typename Iterator>
class AttributesParser: public qi::grammar<Iterator, ascii::space_type, qi::locals<qi::rule<Iterator, Variant(), ascii::space_type> > >
{

public:

    /** @short Constructor only initializes the grammar with empty symbols table
    *
    *   @param kindName Name of top-level object type, to which the attributes belong
    */
    AttributesParser( const std::string &kindName );

    /** @short Function used for filling of symbols table of the parser
    *
    *   @param attributeName Name of the attribute
    *   @param attributeParser  Attribute parser obtained from PredefinedRules class
    *   @see PredefinedRules
    */
    void addAtrribute(
        const std::string &attributeName,
        qi::rule<Iterator, Variant(), ascii::space_type> attributeParser );
    
    std::string getKindName() const;

private:

    /** @short Function used as semantic action for each parsed attribute
    *
    *   @param parameter Name of the attribute
    *   @param value Parsed value of the attribute
    */
    void parsedAttribute( const char* parameter, Variant value );

    qi::symbols<
        char,
        qi::rule<
            Iterator,
            Variant(),
            ascii::space_type> > attributes;

    qi::rule<
        Iterator,
        ascii::space_type,
        qi::locals<qi::rule<
            Iterator,
            Variant(),
            ascii::space_type> > > start;

    std::string name;

};



/** @short Parser for set of attributes of specific top-level grammar */
template <typename Iterator>
class TopLevelParser: public qi::grammar<Iterator, ascii::space_type, qi::locals<qi::rule<Iterator, std::string(), ascii::space_type> > >
{

public:

    /** @short Constructor only initializes the grammar with empty symbols table */
    TopLevelParser();

    /** @short Function used for filling of symbols table of the parser
    *
    *   @param kindName Name of the kind
    */
    void addKind( const std::string &kindName );

private:

    /** @short Function used as semantic action for parsed kind
    *
    *   @param kindName Name of the kind
    *   @param objectName Parsed name of the object
    */
    void parsedKind( const char* kindName, const std::string &objectName );

    qi::symbols<
        char,
        qi::rule<
            Iterator,
            std::string(),
            ascii::space_type> > kinds;

    qi::rule<
        Iterator,
        ascii::space_type,
        qi::locals<qi::rule<
            Iterator,
            std::string(),
            ascii::space_type> > > start;
};



template <typename Iterator>
class Parser: boost::noncopyable
{
public:
    /** @short Initialize the Parser with DB scheme information retrieved via the Deska API */
    Parser( Api* dbApi );

    virtual ~Parser();

    /** @short Parse a full line of user's input
    *
    *   As a result of this parsing, events could get triggered and the state may change.
    */
    void parseLine( const std::string &line );

    /** @short The input indicates that the following signals will be related to a particular object
    *
    *   This signal is emitted whenever the parsed text indicates that we should enter a "context", like when it
    *   reads a line like "host hpv2".  The first argument is the name of the object kind ("hardware" in this case)
    *   and the second one is the object's identifier ("hpv2").
    */
    boost::signals2::signal<void ( const Identifier &kind, const Identifier &name )> categoryEntered;

    /** @short Leaving a context
    *
    *   The Parser hit a line indicating that the current block hsould be left. This could be a result of an explicit
    *   "end" line, or a side effect of a standalone, self-contained line.
    */
    boost::signals2::signal<void ()> categoryLeft;

    /** @short Set an object's attribute
    *
    *   This signal is triggered whenever an attribute definition is encountered. The first argument is the name
    *   of the attribute and the second one the attribute value.
    */
    boost::signals2::signal<void ( const Identifier &name, const Value &value )> attributeSet;

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
    std::vector<std::pair<Identifier, Identifier> > currentContextStack() const;


private:

    /** @short Fills symbols table of specific attribute parser with all attributes of given kind */
    void addKindAttributes(
        std::string &kindName,
        AttributesParser<Iterator>* attributeParser );

    Api *m_dbApi;

    std::map<std::string, AttributesParser<Iterator>* > attributesParsers;

    TopLevelParser<Iterator>* topLevelParser;

};

}

}



#endif  // DESKA_PARSER_H
