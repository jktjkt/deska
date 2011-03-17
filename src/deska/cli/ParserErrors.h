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
namespace CLI
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


/** @short Class used for conversion from boost::iterator_range<class> to std::string */
template <typename Iterator>
class RangeToString
{
public:
    template <typename, typename>
        struct result { typedef void type; };

    void operator()( const boost::iterator_range<Iterator> &range, std::string &str ) const;
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
    void operator()(Iterator start, Iterator end, Iterator errorPos, const spirit::info &what,
        qi::symbols<char, qi::rule<Iterator, std::string(), ascii::space_type> > kinds ) const;

    static void printKindName( const std::string &name, const qi::rule<Iterator, std::string(), ascii::space_type> &rule );
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
    void operator()(Iterator start, Iterator end, Iterator errorPos, const spirit::info &what,
        qi::symbols<char, qi::rule<Iterator, Value(), ascii::space_type> > attributes ) const;

    static void printAttributeName( const std::string &name, const qi::rule<Iterator, Value(), ascii::space_type> &rule );
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



/** @short Class used in visitor for boost::spirit::info to extract
*          keywords from it to std::vector passed to the constructor
*/
class InfoExtractor
{
public:
    InfoExtractor( std::vector<std::string> *keywordsList );

    void element( spirit::utf8_string const& tag, spirit::utf8_string const& value, int ) const;

private:
    std::vector<std::string> *list;
};



/** @short Class representing parsing errors of input */
template <typename Iterator>
class ParseError
{
public:
    ParseError( Iterator start, Iterator end, Iterator errorPos, const spirit::info &what );
    ParseError( Iterator start, Iterator end, Iterator errorPos, const spirit::info &what,
        qi::symbols<char, qi::rule<Iterator, std::string(), ascii::space_type> > &kinds );
    ParseError( Iterator start, Iterator end, Iterator errorPos, const spirit::info &what,
        qi::symbols<char, qi::rule<Iterator, Value(), ascii::space_type> > &attributes );

private:
    void extractKindName( const std::string &name, const qi::rule<Iterator, std::string(), ascii::space_type> &rule );
    void extractAttributeName( const std::string &name, const qi::rule<Iterator, Value(), ascii::space_type> &rule );

    ParseErrorType errorType;
    std::vector<std::string> expectedKeywords;
    Iterator m_start;
    Iterator m_end;
    Iterator m_errorPos;
};


}
}


#endif // DESKA_PARSER_ERRORS_H