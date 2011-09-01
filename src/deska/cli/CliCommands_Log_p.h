/*
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

#ifndef DESKA_CLI_CLICOMMANDS_LOG_P_H
#define DESKA_CLI_CLICOMMANDS_LOG_P_H

#include <string>
#include <vector>

#include "CliCommands.h"
#include "PredefinedRules.h"
#include "ParserIterator.h"
#include "deska/db/Revisions.h"
#include "deska/db/Filter.h"


namespace Deska {
namespace Cli {


class UserInterface;
class Log;



/** @short Handles errors during parsing a kind name or metadata name in log filters. */
template <typename Iterator>
class LogAttributeErrorHandler
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
    *   @param metadatas Symbols table with possible metadata names
    *   @param parent Pointer to Log command for error reporting purposes
    */
    void operator()(Iterator start, Iterator end, Iterator errorPos, const boost::spirit::info &what,
                    const qi::symbols<char, qi::rule<Iterator, Db::Identifier(), ascii::space_type> > &kinds,
                    const qi::symbols<char, qi::rule<Iterator, Db::MetadataValue(), ascii::space_type> > &metadatas,
                    Log *parent) const;
};



/** @short Handles errors while parsing a metadata value in log filters. */
template <typename Iterator>
class LogValueErrorHandler
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
    *   @param metadataName Name of metadata which value is currently being parsed
    *   @param parent Pointer to Log command for error reporting purposes
    */
    void operator()(Iterator start, Iterator end, Iterator errorPos, const boost::spirit::info &what,
                    const Db::Identifier &metadataName, Log *parent) const;
};



/** @short Handles errors while parsing an object name in log filters. */
template <typename Iterator>
class LogIdentifierErrorHandler
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
    *   @param kindName Name of kind which object name is currently being parsed
    *   @param parent Pointer to Log command for error reporting purposes
    */
    void operator()(Iterator start, Iterator end, Iterator errorPos, const boost::spirit::info &what,
                    const Db::Identifier &kindName, Log *parent) const;
};



/** @short Type of parse error when parsing filters for revisions. */
typedef enum {
    /** @short Error in a kind name or metadata name */
    LOG_FILTER_PARSE_ERROR_TYPE_ATTRIBUTE,
    /** @short Error in a metadata value */
    LOG_FILTER_PARSE_ERROR_TYPE_VALUE_TYPE,
    /** @short Error in an object name */
    LOG_FILTER_PARSE_ERROR_TYPE_IDENTIFIER,
} LogFilterParseErrorType;



/** @short Class used for storing information about parse errors when parsing filters for revisions. */
template <typename Iterator>
class LogFilterParseError
{
public:

    /** @short Creates error using LogAttributeErrorHandler when some error occures in kind name or metadata name parsing.
    *
    *   @param start Begin of the input being parsed when the error occures
    *   @param end End of the input being parsed when the error occures
    *   @param errorPos Position where the error occures
    *   @param what Expected tokens
    *   @param kinds Symbols table with possible kind names
    *   @param metadatas Symbols table with possible metadata names
    *   @see LogAttributeErrorHandler
    */
    LogFilterParseError(Iterator start, Iterator end, Iterator errorPos, const boost::spirit::info &what,
                        const qi::symbols<char, qi::rule<Iterator, Db::Identifier(), ascii::space_type> > &kinds,
                        const qi::symbols<char, qi::rule<Iterator, Db::MetadataValue(), ascii::space_type> > &metadatas,
                        LogFilterParseErrorType logFilterParseErrorType);

    /** @short Creates error using LogValueErrorHandler when some error occures in object name or metadata value parsing.
    *
    *   @param start Begin of the input being parsed when the error occures
    *   @param end End of the input being parsed when the error occures
    *   @param errorPos Position where the error occures
    *   @param what Expected tokens
    *   @param attributeName Name of kind or metadata which value parsing failed
    *   @see LogValueErrorHandler
    */
    LogFilterParseError(Iterator start, Iterator end, Iterator errorPos, const boost::spirit::info &what,
                        const Db::Identifier &attributeName, LogFilterParseErrorType logFilterParseErrorType);

    /** @short Function for obtaining type of the error.
    *   
    *   @return Error type
    *   @see LogFilterParseErrorType
    */
    LogFilterParseErrorType errorType() const;

    /** @short Converts error to std::string
    *
    *   @return Description of the error
    */
    std::string toString() const;
    
private:

    /** @short Function for extracting kind names from symbols table.
    *
    *   This function should be used for extracting kind names from symbols table using for_each function.
    *
    *   @param name Key from the symbols table
    *   @param rule Value from the symbols table
    */
    void extractKindNames(const Db::Identifier &name,
                          const qi::rule<Iterator, Db::Identifier(), ascii::space_type> &rule);

    /** @short Function for extracting metadata names from symbols table used in filters grammar.
    *
    *   This function should be used for extracting kind names from symbols table using for_each function.
    *
    *   @param name Key from the symbols table
    *   @param grammar Value from the symbols table
    */
    void extractMetadataNames(const Db::Identifier &name,
                              const qi::rule<Iterator, Db::MetadataValue(), ascii::space_type> &rule);

    /** Error type */
    LogFilterParseErrorType m_errorType;

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

    /** Current context of the parser. Kind name when parsing object name, or metadata name,
    *   when parsing it's value.
    */
    std::string m_context;
};



template <typename Iterator>
class LogFilterParser: public qi::grammar<Iterator, Db::Filter(), ascii::space_type>
{

public:

    /** @short Constructor only initializes the grammar with empty symbols table.
    *
    *   @param parent Pointer to Log command for error reporting purposes.
    */
    LogFilterParser(Log *parent);

    ~LogFilterParser();

    /** @short Function used for filling of symbols table of the parser.
    *
    *   @param nestedKindName Name of nested kind.
    *   @param expressionsParser Filter expressions parser for the nested kind.
    */
    void addKind(const Db::Identifier &kindName);

private:

    /** Symbols table with comparison operators. */
    qi::symbols<char, Db::ComparisonOperator> operators;
    /** Kind name - object name parser pairs for purposes of Nabialek trick. */
    qi::symbols<char, qi::rule<Iterator, Db::Identifier(), ascii::space_type> > kinds;
    /** Metadata name - metadata value parser pairs for purposes of Nabialek trick. */
    qi::symbols<char, qi::rule<Iterator, Db::MetadataValue(), ascii::space_type> > metadatas;

    /** Main rule. */
    qi::rule<Iterator, Db::Filter(), ascii::space_type> start;

    qi::rule<Iterator, Db::Filter(), ascii::space_type> expr;

    qi::rule<Iterator, Db::Filter(), ascii::space_type> andFilter;
    qi::rule<Iterator, Db::Filter(), ascii::space_type> orFilter;

    qi::rule<Iterator, Db::Filter(), ascii::space_type, qi::locals<bool> > kindExpr;
    qi::rule<Iterator, Db::Filter(), ascii::space_type, qi::locals<bool> > metadataExpr;

    /** Rule for parsing expressions for kinds. */
    qi::rule<Iterator, Db::Filter(), ascii::space_type,
             qi::locals<qi::rule<Iterator, Db::Identifier(), ascii::space_type>,
                        Db::ComparisonOperator > > kindDispatch;
    /** Rule for parsing expressions for metadata. */
    qi::rule<Iterator, Db::Filter(), ascii::space_type,
             qi::locals<qi::rule<Iterator, Db::MetadataValue(), ascii::space_type>,
                        Db::ComparisonOperator > > metadataDispatch;

    PredefinedRules<Iterator> *predefinedRules;

    Db::Identifier currentKindName;
    Db::Identifier currentMetadataName;

    Log *m_parent;
};

}
}

#endif // DESKA_CLI_CLICOMMANDS_LOG_P_H
