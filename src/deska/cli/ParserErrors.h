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

#ifndef DESKA_PARSER_ERRORS_H
#define DESKA_PARSER_ERRORS_H

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>

#include "Parser.h"

namespace Deska
{
namespace Cli
{

namespace spirit = boost::spirit;
namespace phoenix = boost::phoenix;
namespace ascii = boost::spirit::ascii;
namespace qi = boost::spirit::qi;

typedef enum {
    PARSE_ERROR_TYPE_KIND,
    PARSE_ERROR_TYPE_ATTRIBUTE,
    PARSE_ERROR_TYPE_VALUE_TYPE,
} ParseErrorType;



/** @short Handle errors during parsing a top-level objects */
template <typename Iterator>
class ObjectErrorHandler
{
public:
    template <typename, typename, typename, typename, typename, typename, typename>
        struct result { typedef void type; };

    /** @short An error has occured during parsing a top-level object
    *
    * Prints information about the error.
    *
    *   @param start Begin of the input being parsed when the error occures
    *   @param end End of the input being parsed when the error occures
    *   @param errorPos Position where the error occures
    *   @param what Expected tokens
    */
    void operator()(Iterator start, Iterator end, Iterator errorPos, const spirit::info &what,
        const qi::symbols<char, qi::rule<Iterator, std::string(), ascii::space_type> > kinds,
        const std::string &kindName, ParserImpl<Iterator> *parser ) const;
};



/** @short Handle errors while parsing a name of an attribute */
template <typename Iterator>
class KeyErrorHandler
{
public:
    template <typename, typename, typename, typename, typename, typename, typename>
        struct result { typedef void type; };

    /** @short An error has occured while parsing a name of an attribute
    *
    * Prints information about the error.
    *
    *   @param start Begin of the input being parsed when the error occures
    *   @param end End of the input being parsed when the error occures
    *   @param errorPos Position where the error occures
    *   @param what Expected tokens
    */
    void operator()(Iterator start, Iterator end, Iterator errorPos, const spirit::info &what,
        const qi::symbols<char, qi::rule<Iterator, Db::Value(), ascii::space_type> > attributes,
        const std::string &kindName, ParserImpl<Iterator> *parser ) const;
};



/** @short Handle errors while parsing an attribute's value */
template <typename Iterator>
class ValueErrorHandler
{
public:
    template <typename, typename, typename, typename, typename, typename>
        struct result { typedef void type; };

    /** @short An error has occured while parsing an attribute's value
    *
    * Prints information about the error.
    *
    *   @param start Begin of the input being parsed when the error occures
    *   @param end End of the input being parsed when the error occures
    *   @param errorPos Position where the error occures
    *   @param what Expected tokens
    */
        void operator()( Iterator start, Iterator end, Iterator errorPos, const spirit::info &what,
            const std::string &attributeName, ParserImpl<Iterator> *parser ) const;
};



/** @short Extract keywords from boost::spirit::info into a vector of strings

This class is used as a visitor of boost::spirit::info to extract keywords from it to a std::vector passed to the constructor.
*/
class InfoExtractor
{
public:
    InfoExtractor( std::vector<std::string> *keywordsList, std::vector<std::string> *typesList );

    void element( spirit::utf8_string const& tag, spirit::utf8_string const& value, int ) const;

private:
    std::vector<std::string> *kList;
    std::vector<std::string> *tList;
};



/** @short Representation of input parsing errors */
template <typename Iterator>
class ParseError
{
public:
    ParseError( Iterator start, Iterator end, Iterator errorPos, const spirit::info &what, const std::string &attributeName );
    ParseError( Iterator start, Iterator end, Iterator errorPos, const spirit::info &what,
        const qi::symbols<char, qi::rule<Iterator, std::string(), ascii::space_type> > &kinds, const std::string &kindName );
    ParseError( Iterator start, Iterator end, Iterator errorPos, const spirit::info &what,
        const qi::symbols<char, qi::rule<Iterator, Db::Value(), ascii::space_type> > &attributes, const std::string &kindName );

    std::string toString();

    bool valid();

private:
    void extractKindName( const std::string &name, const qi::rule<Iterator, std::string(), ascii::space_type> &rule );
    void extractAttributeName( const std::string &name, const qi::rule<Iterator, Db::Value(), ascii::space_type> &rule );

    ParseErrorType errorType;
    std::vector<std::string> expectedTypes;
    std::vector<std::string> expectedKeywords;
    Iterator m_start;
    Iterator m_end;
    Iterator m_errorPos;
    std::string context;
};


}
}


#endif // DESKA_PARSER_ERRORS_H
