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

#ifndef DESKA_PARSERPRIVATE_H
#define DESKA_PARSERPRIVATE_H

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>

#include "Parser.h"


namespace Deska
{
namespace CLI
{

namespace spirit = boost::spirit;
namespace phoenix = boost::phoenix;
namespace ascii = boost::spirit::ascii;
namespace qi = boost::spirit::qi;



/** @short Class used for conversion from boost::iterator_range<class> to std::string */
template <typename Iterator>
class RangeToString
{
public:
    template <typename, typename>
        struct result { typedef void type; };

    void operator()( const boost::iterator_range<Iterator> &rng, std::string &str ) const;
};



/** @short Class for reporting parsing errors of input */
template <typename Iterator>
class ObjectErrorHandler
{
public:
    template <typename, typename, typename, typename, typename>
        struct result { typedef void type; };

    /** @short Function executed when some error while parsing a top-level object type occures.
    *          Prints information about the error
    *
    *   @param start Begin of the input being parsed when the error occures
    *   @param end End of the input being parsed when the error occures
    *   @param errorPos Position where the error occures
    *   @param what Expected tokens
    */
    void operator()(
        Iterator start,
        Iterator end,
        Iterator errorPos,
        const spirit::info &what,
        qi::symbols<char, qi::rule<Iterator, std::string(), ascii::space_type> > kinds ) const;

    void printKindName(
        const std::string &name,
        const qi::rule<Iterator, std::string(), ascii::space_type> &rule );
};



/** @short Class for reporting parsing errors of input */
template <typename Iterator>
class KeyErrorHandler
{
public:
    template <typename, typename, typename, typename, typename>
        struct result { typedef void type; };

    /** @short Function executed when some error while parsing a name of an attribute occures.
    *          Prints information about the error
    *
    *   @param start Begin of the input being parsed when the error occures
    *   @param end End of the input being parsed when the error occures
    *   @param errorPos Position where the error occures
    *   @param what Expected tokens
    */
    void operator()(
        Iterator start,
        Iterator end,
        Iterator errorPos,
        const spirit::info &what,
        qi::symbols<char, qi::rule<Iterator, Value(), ascii::space_type> > attributes ) const;

    void printAttributeName(
        const std::string &name,
        const qi::rule<Iterator, Value(), ascii::space_type> &rule );
};



/** @short Class for reporting parsing errors of input */
template <typename Iterator>
class ValueErrorHandler
{
public:
    template <typename, typename, typename, typename>
        struct result { typedef void type; };

    /** @short Function executed when some error while parsing a value of an attribute occures.
    *          Prints information about the error
    *
    *   @param start Begin of the input being parsed when the error occures
    *   @param end End of the input being parsed when the error occures
    *   @param errorPos Position where the error occures
    *   @param what Expected tokens
    */
    void operator()( Iterator start, Iterator end, Iterator errorPos, const spirit::info &what ) const;
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
    *   @param attrType Type of the attribute in question, @see Type
    *   @return Rule that parses specific type of attribute
    */
    const qi::rule<Iterator, Value(), ascii::space_type>& getRule( const Type attrType );

    /** @short Function for getting rule used to parse identifier of top-level objects */
    const qi::rule<Iterator, std::string(), ascii::space_type>& getObjectIdentifier();

private:

    std::map<Type, qi::rule<Iterator, Value(), ascii::space_type> > rulesMap;
    qi::rule<Iterator, std::string(), ascii::space_type> objectIdentifier;

};



/** @short Parser for set of attributes of specific top-level grammar */
template <typename Iterator>
class AttributesParser: public qi::grammar<Iterator, ascii::space_type, qi::locals<bool> >
{

public:

    /** @short Constructor only initializes the grammar with empty symbols table
    *
    *   @param kindName Name of top-level object type, to which the attributes belong
    */
    AttributesParser( const std::string &kindName, ParserImpl<Iterator> *parent );

    /** @short Function used for filling of symbols table of the parser
    *
    *   @param attributeName Name of the attribute
    *   @param attributeParser  Attribute parser obtained from PredefinedRules class
    *   @see PredefinedRules
    */
    void addAtrribute(
        const std::string &attributeName,
        qi::rule<Iterator, Value(), ascii::space_type> attributeParser );

private:

    /** @short Function used as semantic action for each parsed attribute
    *
    *   @param parameter Name of the attribute
    *   @param value Parsed value of the attribute
    */
    void parsedAttribute( const std::string &parameter, Value &value );


    qi::symbols<
        char,
        qi::rule<
            Iterator,
            Value(),
            ascii::space_type> > attributes;

    qi::rule<
        Iterator,
        ascii::space_type,
        qi::locals<bool> > start;

    qi::rule<
        Iterator,
        ascii::space_type,
        qi::locals<
            qi::rule<Iterator, Value(), ascii::space_type>,
            std::string> > dispatch;

    ParserImpl<Iterator> *m_parent;
};



/** @short Parser for set of attributes of specific top-level grammar */
template <typename Iterator>
class TopLevelParser: public qi::grammar<Iterator, ascii::space_type, qi::locals<bool> >
{

public:

    /** @short Constructor only initializes the grammar with empty symbols table */
    TopLevelParser( ParserImpl<Iterator> *parent );

    /** @short Function used for filling of symbols table of the parser
    *
    *   @param kindName Name of the kind
    */
    void addKind( const std::string &kindName, qi::rule<Iterator, std::string(), ascii::space_type> identifierParser );

private:

    /** @short Function used as semantic action for parsed kind
    *
    *   @param kindName Name of the kind
    *   @param objectName Parsed name of the object
    */
    void parsedKind( const std::string &kindName, const std::string &objectName );

    qi::symbols<
        char,
        qi::rule<
            Iterator,
            std::string(),
            ascii::space_type> > kinds;

    qi::rule<
        Iterator,
        ascii::space_type,
        qi::locals<bool> > start;

    qi::rule<
        Iterator,
        ascii::space_type,
        qi::locals<
            qi::rule<Iterator, std::string(), ascii::space_type>,
            std::string> > dispatch;

    ParserImpl<Iterator> *m_parent;
};



template <typename Iterator>
class ParserImpl: boost::noncopyable
{
public:
    ParserImpl( Parser *parent );
    virtual ~ParserImpl();

    void parseLine( const std::string &line );
    bool isNestedInContext() const;
    std::vector<AttributeDefinition> currentContextStack() const;

    void categoryEntered( const Identifier &kind, const Identifier &name );
    void categoryLeft();
    void attributeSet( const Identifier &name, const Value &value );


private:
    Parser *m_parser;

    bool leaveCategory;

    /** @short Fills symbols table of specific attribute parser with all attributes of given kind */
    void addKindAttributes(
        std::string &kindName,
        AttributesParser<Iterator>* attributeParser );

    bool matchesEnd( const std::string &word );

    std::map<std::string, AttributesParser<Iterator>* > attributesParsers;
    TopLevelParser<Iterator> *topLevelParser;
    PredefinedRules<Iterator> *predefinedRules;

    std::vector<AttributeDefinition> contextStack;
};

}

}



#endif  // DESKA_PARSERPRIVATE_H
