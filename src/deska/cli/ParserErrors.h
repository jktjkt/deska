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


/** @short Type of parse error. */
typedef enum {
    /** @short Error in a kinds's name */
    PARSE_ERROR_TYPE_KIND,
    /** @short Error in nesting */
    PARSE_ERROR_TYPE_NESTING,
    /** @short Error in an attribute's name */
    PARSE_ERROR_TYPE_ATTRIBUTE,
    /** @short Error in an attribute's value or kind's identifier */
    PARSE_ERROR_TYPE_VALUE_TYPE,
} ParseErrorType;



/** @short Handle errors during parsing a kind's name. */
template <typename Iterator>
class KindErrorHandler
{
public:
    template <typename, typename, typename, typename, typename, typename, typename>
        struct result { typedef void type; };

    /** @short Function invoked when some error occures during parsing of kind name.
    *
    *   Generates appropriate parse error and pushes it to errors stack.
    *
    *   @param start Begin of the input being parsed when the error occures
    *   @param end End of the input being parsed when the error occures
    *   @param errorPos Position where the error occures
    *   @param what Expected tokens
    *   @param kinds Symbols table with possible kind names
    *   @param kindName Name of kind which attributes or nested kinds are currently being parsed
    *   @param parser Pointer to main parser for purposes of storing generated error
    */
    void operator()(Iterator start, Iterator end, Iterator errorPos, const spirit::info &what,
        const qi::symbols<char, qi::rule<Iterator, Db::Identifier(), ascii::space_type> > &kinds,
        const Db::Identifier &kindName, ParserImpl<Iterator> *parser) const;
};



/** @short Checks whether bad name of kind or attribute is a bad nesting. */
template <typename Iterator>
class NestingErrorHandler
{
public:
    template <typename, typename, typename, typename, typename, typename, typename>
        struct result { typedef void type; };

    /** @short Function invoked when some error occures during parsing of kind name.
    *
    *   Checks whether parsed token in some kind name and generates appropriate parse error and pushes it
    *   to errors stack.
    *
    *   @param start Begin of the input being parsed when the error occures
    *   @param end End of the input being parsed when the error occures
    *   @param errorPos Position where the error occures
    *   @param what Expected tokens
    *   @param failingToken Potential kind name
    *   @param kindName Name of kind which attributes or nested kinds are currently being parsed
    *   @param parser Pointer to main parser for purposes of storing generated error and obtaining kinds list
    */
    void operator()(Iterator start, Iterator end, Iterator errorPos, const spirit::info &what,
        const std::string &failingToken, const Db::Identifier &kindName, ParserImpl<Iterator> *parser) const;
};



/** @short Handle errors while parsing a name of an attribute. */
template <typename Iterator>
class AttributeErrorHandler
{
public:
    template <typename, typename, typename, typename, typename, typename, typename>
        struct result { typedef void type; };

    /** @short Function invoked when some error occures during parsing of attribute name.
    *
    *   Generates appropriate parse error and pushes it to errors stack.
    *
    *   @param start Begin of the input being parsed when the error occures
    *   @param end End of the input being parsed when the error occures
    *   @param errorPos Position where the error occures
    *   @param what Expected tokens
    *   @param attributes Symbols table with possible attributes names
    *   @param kindName Name of kind which attributes or nested kinds are currently being parsed
    *   @param parser Pointer to main parser for purposes of storing generated error
    */
    void operator()(Iterator start, Iterator end, Iterator errorPos, const spirit::info &what,
        const qi::symbols<char, qi::rule<Iterator, Db::Value(), ascii::space_type> > &attributes,
        const Db::Identifier &kindName, ParserImpl<Iterator> *parser) const;
};



/** @short Handle errors while parsing a kind's identifier. */
template <typename Iterator>
class IdentifierErrorHandler
{
public:
    template <typename, typename, typename, typename, typename, typename>
        struct result { typedef void type; };

    /** @short Function invoked when non-existant object name was parsed.
    *
    *   Generates appropriate parse error and pushes it to errors stack.
    *
    *   @param start Begin of the input being parsed when the error occures
    *   @param end End of the input being parsed when the error occures
    *   @param errorPos Position where the error occures
    *   @param what Expected tokens
    *   FIXME: Impement this class
    */
    void operator()(Iterator start, Iterator end, Iterator errorPos, const spirit::info &what,
        const Db::Identifier &kindName, const std::vector<Db::Identifier> &objectNames, ParserImpl<Iterator> *parser) const;
};



/** @short Handle errors while parsing an attribute's value. */
template <typename Iterator>
class ValueErrorHandler
{
public:
    template <typename, typename, typename, typename, typename, typename>
        struct result { typedef void type; };

    /** @short An error has occured while parsing an attribute's value.
    *
    *   Function invoked when bad value type of the attribute was parsed.
    *
    *   @param start Begin of the input being parsed when the error occures
    *   @param end End of the input being parsed when the error occures
    *   @param errorPos Position where the error occures
    *   @param what Expected tokens
    *   @param attributeName Name of attribute which value is currently being parsed
    *   @param parser Pointer to main parser for purposes of storing generated error
    */
    void operator()(Iterator start, Iterator end, Iterator errorPos, const spirit::info &what,
        const Db::Identifier &attributeName, ParserImpl<Iterator> *parser) const;
};



/** @short Extract keywords from boost::spirit::info into a vector of strings
*
*   This class is used as a visitor of boost::spirit::info to extract keywords from it to a std::vector passed
*   to the constructor.
*/
class InfoExtractor
{
public:
    InfoExtractor(std::vector<Db::Identifier> *keywordsList, std::vector<Db::Identifier> *typesList);

    void element(spirit::utf8_string const& tag, spirit::utf8_string const& value, int) const;

private:
    std::vector<Db::Identifier> *kList;
    std::vector<Db::Identifier> *tList;
};



/** @short Representation of input parsing errors */
template <typename Iterator>
class ParseError
{
public:
    ParseError(Iterator start, Iterator end, Iterator errorPos, const spirit::info &what, const Db::Identifier &attributeName);
    ParseError(Iterator start, Iterator end, Iterator errorPos, const spirit::info &what,
        const qi::symbols<char, qi::rule<Iterator, Db::Identifier(), ascii::space_type> > &kinds, const Db::Identifier &kindName);
    /** @short Create error using NestingErrorHandler when some error occures in kind name or attribute name parsing
    *          and parsed name corresponds to another defined kind name, that can not be nested in current kind.
    *
    *   @param start Begin of the input being parsed when the error occures
    *   @param end End of the input being parsed when the error occures
    *   @param errorPos Position where the error occures
    *   @param what Expected tokens
    *   @param failingToken Potential kind name
    *   @param kindName Name of kind which attributes or nested kinds are currently being parsed
    */
    ParseError(Iterator start, Iterator end, Iterator errorPos, const spirit::info &what,
        const std::string &failingToken, const Db::Identifier &kindName);
        const qi::symbols<char, qi::rule<Iterator, Db::Value(), ascii::space_type> > &attributes, const Db::Identifier &kindName);

    ParseErrorType errorType() const;
    Iterator errorPosition(const std::string &line) const;
    std::vector<Db::Identifier> expectedTypes() const;
    std::vector<Db::Identifier> expectedKeywords() const;

    std::string toString() const;
    // FIXME: Maybe rewrite in some other, better way.
    std::string toCombinedString(const ParseError<Iterator> &kindError) const;

    bool valid() const;

private:
    void extractKindName(const Db::Identifier &name, const qi::rule<Iterator, Db::Identifier(), ascii::space_type> &rule);
    void extractAttributeName(const Db::Identifier &name, const qi::rule<Iterator, Db::Value(), ascii::space_type> &rule);

    ParseErrorType m_errorType;
    std::vector<Db::Identifier> m_expectedTypes;
    std::vector<Db::Identifier> m_expectedKeywords;
    Iterator m_start;
    Iterator m_end;
    Iterator m_errorPos;
    std::string m_context;
};


}
}


#endif // DESKA_PARSER_ERRORS_H
