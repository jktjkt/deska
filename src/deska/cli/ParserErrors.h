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
    /** @short Error in a kinds's name when creating new object */
    PARSE_ERROR_TYPE_KINDS_CONSTRUCT,
    /** @short Error in a kinds's name */
    PARSE_ERROR_TYPE_KIND,
    /** @short Error in a kinds's name when jumping in context */
    PARSE_ERROR_TYPE_KIND_NESTING,
    /** @short Error in a kinds's name in a filter */
    PARSE_ERROR_TYPE_KIND_FILTER,
    /** @short Error in a kinds's name in a special filter */
    PARSE_ERROR_TYPE_KIND_SPECIAL_FILTER,
    /** @short Error in nesting */
    PARSE_ERROR_TYPE_NESTING,
    /** @short Error in an attribute's name */
    PARSE_ERROR_TYPE_ATTRIBUTE,
    /** @short Error in an identifiers set's name */
    PARSE_ERROR_TYPE_IDENTIFIERS_SET,
    /** @short Error in an attribute's name  when removing attributes value */
    PARSE_ERROR_TYPE_ATTRIBUTE_REMOVAL,
    /** @short Error in an attribute's value */
    PARSE_ERROR_TYPE_VALUE_TYPE,
    /** @short Error in a kind's identifier */
    PARSE_ERROR_TYPE_OBJECT_NAME,
    /** @short Error when object definition expected, but not found */
    PARSE_ERROR_TYPE_OBJECT_DEFINITION_NOT_FOUND,
    /** @short Error when object being used in some function does not exist */
    PARSE_ERROR_TYPE_OBJECT_NOT_FOUND,
    /** @short Error when identifier was expected, but not found */
    PARSE_ERROR_TYPE_IDENTIFIER_NOT_FOUND
} ParseErrorType;

/** @short Converts ParseErrorType to a string representation
*   
*   @param errorType ParseErrorType to convert
*   @return String representation of the error
*/
std::string parseErrorTypeToString(const ParseErrorType errorType);


/** @short Handles errors during parsing a kind's name during new object construction parsing. */
template <typename Iterator>
class KindConstructErrorHandler
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
    *   @param kindName Name of kind which nested kinds are currently being parsed
    *   @param parser Pointer to main parser for purposes of storing generated error
    *   @see ParseError
    */
    void operator()(Iterator start, Iterator end, Iterator errorPos, const spirit::info &what,
                    const qi::symbols<char, qi::rule<Iterator, ascii::space_type> > &kinds,
                    const Db::Identifier &kindName, ParserImpl<Iterator> *parser) const;
};



/** @short Handles errors during parsing a kind's name during standard parsing. */
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
    *   @see ParseError
    */
    void operator()(Iterator start, Iterator end, Iterator errorPos, const spirit::info &what,
                    const qi::symbols<char, qi::rule<Iterator, Db::Identifier(), ascii::space_type> > &kinds,
                    const Db::Identifier &kindName, ParserImpl<Iterator> *parser) const;
};



/** @short Handles errors during parsing a kind's name in a filter. */
template <typename Iterator>
class KindFiltersErrorHandler
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
    *   @see ParseError
    */
    void operator()(Iterator start, Iterator end, Iterator errorPos, const spirit::info &what,
                    const qi::symbols<char, qi::rule<Iterator, Db::Filter(), ascii::space_type> > &kinds,
                    const Db::Identifier &kindName, ParserImpl<Iterator> *parser) const;
};



/** @short Handles errors during parsing a kind's name in a special filter like "all" or "last". */
template <typename Iterator>
class KindSpecialFiltersErrorHandler
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
    *   @see ParseError
    */
    void operator()(Iterator start, Iterator end, Iterator errorPos, const spirit::info &what,
                    const qi::symbols<char, qi::rule<Iterator, ascii::space_type> > &kinds,
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
    *   @see ParseError
    */
    void operator()(Iterator start, Iterator end, Iterator errorPos, const spirit::info &what,
                    const std::string &failingToken,
                    const Db::Identifier &kindName, ParserImpl<Iterator> *parser) const;
};



/** @short Handles errors while parsing a name of an attribute. */
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
    *   @see ParseError
    */
    void operator()(Iterator start, Iterator end, Iterator errorPos, const spirit::info &what,
                    const qi::symbols<char, qi::rule<Iterator, Db::Value(), ascii::space_type> > &attributes,
                    const Db::Identifier &kindName, ParserImpl<Iterator> *parser) const;
};



/** @short Handles errors while parsing a name of an attribute in a filter. */
template <typename Iterator>
class AttributeFilterErrorHandler
{
public:
    template <typename, typename, typename, typename, typename, typename, typename, typename>
        struct result { typedef void type; };

    /** @short Function invoked when some error occures during parsing of attribute name in context of a filter.
    *
    *   Generates appropriate parse error and pushes it to errors stack.
    *
    *   @param start Begin of the input being parsed when the error occures
    *   @param end End of the input being parsed when the error occures
    *   @param errorPos Position where the error occures
    *   @param what Expected tokens
    *   @param attributes Symbols table with possible attributes names
    *   @param sets Symbols table with possible sets names
    *   @param kindName Name of kind which attributes or nested kinds are currently being parsed
    *   @param parser Pointer to main parser for purposes of storing generated error
    *   @see ParseError
    */
    void operator()(Iterator start, Iterator end, Iterator errorPos, const spirit::info &what,
                    const qi::symbols<char, qi::rule<Iterator, Db::Value(), ascii::space_type> > &attributes,
                    const qi::symbols<char, qi::rule<Iterator, Db::Identifier(), ascii::space_type> > &sets,
                    const Db::Identifier &kindName, ParserImpl<Iterator> *parser) const;
};



/** @short Handles errors while parsing a name of an identifiers set. */
template <typename Iterator>
class IdentifiersSetsErrorHandler
{
public:
    template <typename, typename, typename, typename, typename, typename, typename>
        struct result { typedef void type; };

    /** @short Function invoked when some error occures during parsing of identifiers set name.
    *
    *   Generates appropriate parse error and pushes it to errors stack.
    *
    *   @param start Begin of the input being parsed when the error occures
    *   @param end End of the input being parsed when the error occures
    *   @param errorPos Position where the error occures
    *   @param what Expected tokens
    *   @param sets Symbols table with possible sets names
    *   @param kindName Name of kind which sets, attributes or nested kinds are currently being parsed
    *   @param parser Pointer to main parser for purposes of storing generated error
    *   @see ParseError
    */
    void operator()(Iterator start, Iterator end, Iterator errorPos, const spirit::info &what,
                    const qi::symbols<char, qi::rule<Iterator, Db::Identifier(), ascii::space_type> > &sets,
                    const Db::Identifier &kindName, ParserImpl<Iterator> *parser) const;
};



/** @short Handles errors while parsing a name of an attribute while deleting attribute value. */
template <typename Iterator>
class AttributeRemovalErrorHandler
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
    *   @see ParseError
    */
    void operator()(Iterator start, Iterator end, Iterator errorPos, const spirit::info &what,
                    const qi::symbols<char, qi::rule<Iterator, ascii::space_type> > &attributes,
                    const Db::Identifier &kindName, ParserImpl<Iterator> *parser) const;
};



/** @short Handles errors while parsing a kind's identifier. */
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
    *   @see ParseError
    *   FIXME: Impement this class
    */
    void operator()(Iterator start, Iterator end, Iterator errorPos, const spirit::info &what,
                    const Db::Identifier &kindName, const std::vector<Db::Identifier> &objectNames,
                    ParserImpl<Iterator> *parser) const;
};



/** @short Handles errors while parsing an attribute's value. */
template <typename Iterator>
class ValueErrorHandler
{
public:
    template <typename, typename, typename, typename, typename, typename, typename>
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
    *   @param kindName Name of kind which attribute value is currently being parsed
    *   @param parser Pointer to main parser for purposes of storing generated error
    *   @see ParseError
    */
    void operator()(Iterator start, Iterator end, Iterator errorPos, const spirit::info &what,
                    const Db::Identifier &attributeName, const Db::Identifier &kindName,
                    ParserImpl<Iterator> *parser) const;
};



/** @short Handles errors while parsing a kind's name. */
template <typename Iterator>
class ObjectNameErrorHandler
{
public:
    template <typename, typename, typename, typename, typename, typename>
        struct result { typedef void type; };

    /** @short An error has occured while parsing a kind's name.
    *
    *   Function invoked when bad value type of the attribute was parsed.
    *
    *   @param start Begin of the input being parsed when the error occures
    *   @param end End of the input being parsed when the error occures
    *   @param errorPos Position where the error occures
    *   @param what Expected tokens
    *   @param kindName Name of kind which name is currently being parsed
    *   @param parser Pointer to main parser for purposes of storing generated error
    *   @see ParseError
    */
    void operator()(Iterator start, Iterator end, Iterator errorPos, const spirit::info &what,
                    const Db::Identifier &kindName, ParserImpl<Iterator> *parser) const;
};



/** @short Representation of input parsing errors */
template <typename Iterator>
class ParseError
{

public:
    
    /** @short Creates error using KindErrorHandler when some error occures in kind name parsing.
    *
    *   @param start Begin of the input being parsed when the error occures
    *   @param end End of the input being parsed when the error occures
    *   @param errorPos Position where the error occures
    *   @param what Expected tokens
    *   @param kinds Symbols table with possible kind names
    *   @param kindName Name of kind which attributes or nested kinds are currently being parsed
    *   @param parseErrorType Type of the error identifying, where the error occures
    *   @see KindErrorHandler
    */
    ParseError(Iterator start, Iterator end, Iterator errorPos, const spirit::info &what,
               const qi::symbols<char, qi::rule<Iterator, Db::Identifier(), ascii::space_type> > &kinds,
               const Db::Identifier &kindName, ParseErrorType parseErrorType);

    /** @short Creates error using KindFilterErrorHandler when some error occures in kind name in filter parsing.
    *
    *   @param start Begin of the input being parsed when the error occures
    *   @param end End of the input being parsed when the error occures
    *   @param errorPos Position where the error occures
    *   @param what Expected tokens
    *   @param kinds Symbols table with possible kind names
    *   @param kindName Name of kind which attributes or nested kinds are currently being parsed
    *   @param parseErrorType Type of the error identifying, where the error occures
    *   @see KindFiltersErrorHandler
    */
    ParseError(Iterator start, Iterator end, Iterator errorPos, const spirit::info &what,
               const qi::symbols<char, qi::rule<Iterator, Db::Filter(), ascii::space_type> > &kinds,
               const Db::Identifier &kindName, ParseErrorType parseErrorType);

    /** @short Creates error using NestingErrorHandler when some error occures in kind name or attribute name parsing
    *          and parsed name corresponds to another defined kind name, that can not be nested in current kind.
    *
    *   @param start Begin of the input being parsed when the error occures
    *   @param end End of the input being parsed when the error occures
    *   @param errorPos Position where the error occures
    *   @param what Expected tokens
    *   @param failingToken Potential kind name
    *   @param kindName Name of kind which attributes or nested kinds are currently being parsed
    *   @param parseErrorType Type of the error identifying, where the error occures
    */
    ParseError(Iterator start, Iterator end, Iterator errorPos, const spirit::info &what,
               const std::string &failingToken, const Db::Identifier &kindName, ParseErrorType parseErrorType);

    /** @short Creates error using AttributeErrorHandler when some error occures in attribute name parsing.
    *   @param start Begin of the input being parsed when the error occures
    *   @param end End of the input being parsed when the error occures
    *   @param errorPos Position where the error occures
    *   @param what Expected tokens
    *   @param attributes Symbols table with possible attributes names
    *   @param kindName Name of kind which attributes or nested kinds are currently being parsed
    *   @param parseErrorType Type of the error identifying, where the error occures
    */
    ParseError(Iterator start, Iterator end, Iterator errorPos, const spirit::info &what,
               const qi::symbols<char, qi::rule<Iterator, Db::Value(), ascii::space_type> > &attributes,
               const Db::Identifier &kindName, ParseErrorType parseErrorType);

    /** @short Creates error using AttributeErrorHandler when some error occures in attribute name parsing in filter.
    *   @param start Begin of the input being parsed when the error occures
    *   @param end End of the input being parsed when the error occures
    *   @param errorPos Position where the error occures
    *   @param what Expected tokens
    *   @param attributes Symbols table with possible attributes names
    *   @param sets Symbols table with possible sets names
    *   @param kindName Name of kind which attributes or nested kinds are currently being parsed
    *   @param parseErrorType Type of the error identifying, where the error occures
    */
    ParseError(Iterator start, Iterator end, Iterator errorPos, const spirit::info &what,
               const qi::symbols<char, qi::rule<Iterator, Db::Value(), ascii::space_type> > &attributes,
               const qi::symbols<char, qi::rule<Iterator, Db::Identifier(), ascii::space_type> > &sets,
               const Db::Identifier &kindName, ParseErrorType parseErrorType);

    /** @short Creates error using AttributeErrorHandler when some error occures in attribute name parsing.
    *   @param start Begin of the input being parsed when the error occures
    *   @param end End of the input being parsed when the error occures
    *   @param errorPos Position where the error occures
    *   @param what Expected tokens
    *   @param attributes Symbols table with possible attributes names
    *   @param kindName Name of kind which attributes or nested kinds are currently being parsed
    *   @param parseErrorType Type of the error identifying, where the error occures
    */
    ParseError(Iterator start, Iterator end, Iterator errorPos, const spirit::info &what,
               const qi::symbols<char, qi::rule<Iterator, ascii::space_type> > &attributes,
               const Db::Identifier &kindName, ParseErrorType parseErrorType);

    /** @short Creates error using ValueErrorHandler when some error occures in kind's identifier parsing.
    *   
    *   @param start Begin of the input being parsed when the error occures
    *   @param end End of the input being parsed when the error occures
    *   @param errorPos Position where the error occures
    *   @param what Expected tokens
    *   @param kindName Name of kind which name is currently being parsed
    *   @param parseErrorType Type of the error identifying, where the error occures
    */
    ParseError(Iterator start, Iterator end, Iterator errorPos, const spirit::info &what,
               const Db::Identifier &kindName, ParseErrorType parseErrorType);

    /** @short Creates error using ValueErrorHandler when some error occures in attribute's value
    *   
    *   @param start Begin of the input being parsed when the error occures
    *   @param end End of the input being parsed when the error occures
    *   @param errorPos Position where the error occures
    *   @param what Expected tokens
    *   @param attributeName Name of attribute which value is currently being parsed
    *   @param parseErrorType Type of the error identifying, where the error occures
    *   @param kindName Name of kind which attribute value is currently being parsed
    */
    ParseError(Iterator start, Iterator end, Iterator errorPos, const spirit::info &what,
               const Db::Identifier &attributeName, ParseErrorType parseErrorType, const Db::Identifier &kindName);

    /** @short Creates error when object definition expected, but not found.
    *   
    *   @param start Begin of the input being parsed when the error occures
    *   @param end End of the input being parsed when the error occures
    *   @param errorPos Position where the error occures
    *   @param kindName Name of kind which nested kinds are currently being parsed
    *   @param expectedKinds Expected kind names
    *   @param parseErrorType Type of the error identifying, where the error occures
    */
    ParseError(Iterator start, Iterator end, Iterator errorPos, const Db::Identifier &kindName,
               const std::vector<Db::Identifier> &expectedKinds, ParseErrorType parseErrorType);

    /** @short Creates error when object being used in some function does not exist.
    *   
    *   @param start Begin of the input being parsed when the error occures
    *   @param end End of the input being parsed when the error occures
    *   @param errorPos Position where the error occures
    *   @param kindName Name of parsed kind
    *   @param objectName Name of parsed object
    *   @param parseErrorType Type of the error identifying, where the error occures
    */
    ParseError(Iterator start, Iterator end, Iterator errorPos, const Db::Identifier &kindName,
               const Db::Identifier &objectName, ParseErrorType parseErrorType);

    /** @short Creates error when identifier was expected, but not found.
    *   
    *   @param start Begin of the input being parsed when the error occures
    *   @param end End of the input being parsed when the error occures
    *   @param errorPos Position where the error occures
    *   @param parseErrorType Type of the error identifying, where the error occures
    */
    ParseError(Iterator start, Iterator end, Iterator errorPos, ParseErrorType parseErrorType);

    /** @short Creates error when bad object name was entered when jumping in context.
    *   
    *   @param kindName Name of parsed kind
    *   @param objectName Name of parsed object
    *   @param parseErrorType Type of the error identifying, where the error occures
    */
    ParseError(const Db::Identifier &kindName, const Db::Identifier &objectName, ParseErrorType parseErrorType);


    /** @short Function for obtaining type of the error.
    *   
    *   @return Error type
    *   @see ParseErrorType
    */
    ParseErrorType errorType() const;
    /** @short Function for obtaining position in the line, where the error occured.
    *   
    *   @return Iterator to the error position
    */
    Iterator errorPosition() const;
    /** @short Function for obtaining expected value types from the error.
    *
    *   @return Expected vale types as vector of their names
    */
    std::vector<std::string> expectedTypes() const;
    /** @short Function for obtaining expected keywords from the error.
    *
    *   @return Expected keywords as vector of identifiers
    */
    std::vector<Db::Identifier> expectedKeywords() const;

    /** @short Gets context of the error.
    *
    *   Error types and context content:
    *   PARSE_ERROR_TYPE_KIND - kind name or no context when in top-level
    *   PARSE_ERROR_TYPE_KIND_FILTER - kind name or no context when in top-level
    *   PARSE_ERROR_TYPE_NESTING - kind name
    *   PARSE_ERROR_TYPE_ATTRIBUTE - kind name
    *   PARSE_ERROR_TYPE_ATTRIBUTE_REMOVAL - kind name
    *   PARSE_ERROR_TYPE_VALUE_TYPE - attribute name
    *   PARSE_ERROR_TYPE_OBJECT_DEFINITION_NOT_FOUND - kind name
    *   PARSE_ERROR_TYPE_OBJECT_NOT_FOUND - pair kind name and object name
    *   PARSE_ERROR_TYPE_IDENTIFIER_NOT_FOUND - no context
    *   
    *   @return Context of the error.
    */
    std::string context() const;

    /** @short Gets kind name in case of parsing an attribute value.
    *
    *   @return Name of kind which attribute value caused an error
    */
    std::string contextKind() const;

    /** @short Converts error to std::string
    *
    *   @return Description of the error
    */
    std::string toString() const;

    /** @short Combines parse error of type PARSE_ERROR_TYPE_ATTRIBUTE with error of type PARSE_ERROR_TYPE_KIND
    *          into one string for purposes of error reporting to main parser.
    *
    *   Function have to be called on grammar of type PARSE_ERROR_TYPE_ATTRIBUTE and attribute have to be of type
    *   PARSE_ERROR_TYPE_KIND.
    *
    *   @param kindError Parse error of type PARSE_ERROR_TYPE_KIND
    */
    // FIXME: Maybe rewrite in some other, better way.
    std::string toCombinedString(const ParseError<Iterator> &kindError) const;

    /** @short Tests if error is a real error, or only consequence of usage of eps rule in the parser
    *
    *   @return True if the error is real error
    */
    bool valid() const;

private:
    /** @short Function for extracting kind names from symbols table used in kind grammar.
    *
    *   This function should be used for extracting kind names from symbols table using for_each function.
    *
    *   @param name Key from the symbols table
    *   @param rule Value from the symbols table
    *   @see KindsOnlyParser
    */
    void extractKindName(const Db::Identifier &name,
                         const qi::rule<Iterator, Db::Identifier(), ascii::space_type> &rule);
    /** @short Function for extracting kind names from symbols table used in filters grammar.
    *
    *   This function should be used for extracting kind names from symbols table using for_each function.
    *
    *   @param name Key from the symbols table
    *   @param grammar Value from the symbols table
    *   @see KindsFiltersParser
    */
    void extractKindFilterName(const Db::Identifier &name,
                               const qi::rule<Iterator, Db::Filter(), ascii::space_type> &rule);
    /** @short Function for extracting attribute names from symbols table used in attributes grammars.
    *
    *   This function should be used for extracting attributes names from symbols table using for_each function.
    *
    *   @param name Key from the symbols table
    *   @param rule Value from the symbols table
    *   @see AttributesSettingParser
    */
    void extractAttributeName(const Db::Identifier &name,
                              const qi::rule<Iterator, Db::Value(), ascii::space_type> &rule);
    /** @short Function for extracting set names from symbols table used in filter grammars.
    *
    *   This function should be used for extracting set names from symbols table using for_each function.
    *
    *   @param name Key from the symbols table
    *   @param rule Value from the symbols table
    *   @see FilterExpressionsParser
    */
    void extractSetName(const Db::Identifier &name,
                              const qi::rule<Iterator, Db::Identifier(), ascii::space_type> &rule);
    /** @short Function for extracting attribute names from symbols table used in attributeRemovals grammars.
    *
    *   This function should be used for extracting attributes names from symbols table using for_each function.
    *
    *   @param name Key from the symbols table
    *   @param rule Value from the symbols table
    *   @see AttributeRemovalsParser
    */
    void extractRemovedAttributeName(const Db::Identifier &name, const qi::rule<Iterator, ascii::space_type> &rule);

    /** Error type */
    ParseErrorType m_errorType;

    /** List with expected keywords */
    std::vector<std::string> m_expectedTypes;
    /** List with expected value types */
    std::vector<Db::Identifier> m_expectedKeywords;

    /** Begin of the input being parsed when the error occures */
    Iterator m_start;
    /** End of the input being parsed when the error occures */
    Iterator m_end;
    /** errorPos Position where the error occures */
    Iterator m_errorPos;

    /** Current context of the parser. Attribute name when parsing attribute's value, or kind name,
    *   when parsing it's attributes or nested kinds or whole object name, when parsing arguments
    *   for some function.
    */
    std::string m_context;

    /** kindName in case of parsing an attribute value */
    std::string m_contextKind;
};


}
}


#endif // DESKA_PARSER_ERRORS_H
