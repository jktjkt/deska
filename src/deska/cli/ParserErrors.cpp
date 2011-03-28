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

#include <boost/spirit/include/phoenix_bind.hpp>

#include "ParserErrors.h"
#include "Parser_p.h"

#define PARSER_DEBUG
#define PARSER_PRINT_ERRORS

namespace Deska
{
namespace Cli
{



template <typename Iterator>
void ObjectErrorHandler<Iterator>::operator()(Iterator start, Iterator end, Iterator errorPos, const spirit::info& what,
    const qi::symbols<char, qi::rule<Iterator, std::string(), ascii::space_type> > kinds,
    const std::string &kindName, ParserImpl<Iterator> *parser ) const
{
    ParseError<Iterator> error( start, end, errorPos, what, kinds, kindName );
    if ( error.valid() )
        parser->addParseError( error );
}



template <typename Iterator>
void KeyErrorHandler<Iterator>::operator()(Iterator start, Iterator end, Iterator errorPos, const spirit::info& what,
    const qi::symbols<char, qi::rule<Iterator, Db::Value(), ascii::space_type> > attributes,
    const std::string &kindName, ParserImpl<Iterator> *parser ) const
{
    ParseError<Iterator> error( start, end, errorPos, what, attributes, kindName );
    if ( error.valid() )
        parser->addParseError( error );
}



template <typename Iterator>
void ValueErrorHandler<Iterator>::operator()(Iterator start, Iterator end, Iterator errorPos, const spirit::info& what,
    const std::string &attributeName, ParserImpl<Iterator> *parser ) const
{
    ParseError<Iterator> error( start, end, errorPos, what, attributeName );
    if ( error.valid() )
        parser->addParseError( error );
}



InfoExtractor::InfoExtractor( std::vector<std::string> *keywordsList, std::vector<std::string> *typesList ):
    kList(keywordsList), tList(typesList)
{
}



void InfoExtractor::element( spirit::utf8_string const& tag, spirit::utf8_string const& value, int ) const
{
    if ( !value.empty() )
        kList->push_back(value);
    else
        tList->push_back(tag);
}



template <typename Iterator>
ParseError<Iterator>::ParseError( Iterator start, Iterator end, Iterator errorPos,
    const spirit::info &what, const std::string &attributeName ):
    errorType(PARSE_ERROR_TYPE_VALUE_TYPE), m_start(start), m_end(end), m_errorPos(errorPos), context(attributeName)
{
    InfoExtractor extractor( &expectedKeywords, &expectedTypes );
    spirit::basic_info_walker<InfoExtractor> walker( extractor, what.tag, 0 );
    boost::apply_visitor( walker, what.value );
}



template <typename Iterator>
ParseError<Iterator>::ParseError( Iterator start, Iterator end, Iterator errorPos, const spirit::info &what,
    const qi::symbols<char, qi::rule<Iterator, std::string(), ascii::space_type> > &kinds, const std::string &kindName ):
    errorType(PARSE_ERROR_TYPE_KIND), m_start(start), m_end(end), m_errorPos(errorPos), context(kindName)
{
    using namespace boost::phoenix::arg_names;
    kinds.for_each( boost::phoenix::bind( &ParseError<Iterator>::extractKindName, this, arg1, arg2 ) );
}



template <typename Iterator>
ParseError<Iterator>::ParseError( Iterator start, Iterator end, Iterator errorPos, const spirit::info &what,
    const qi::symbols<char, qi::rule<Iterator, Db::Value(), ascii::space_type> > &attributes, const std::string &kindName ):
    errorType(PARSE_ERROR_TYPE_ATTRIBUTE), m_start(start), m_end(end), m_errorPos(errorPos), context(kindName)
{
    using namespace boost::phoenix::arg_names;
    attributes.for_each( boost::phoenix::bind( &ParseError<Iterator>::extractAttributeName, this, arg1, arg2 ) );
}



std::string parseErrorTypeToString(const ParseErrorType errorType)
{
    switch( errorType ) {
        case PARSE_ERROR_TYPE_KIND:
            return "kind name";
        case PARSE_ERROR_TYPE_ATTRIBUTE:
            return "attribute name";
        case PARSE_ERROR_TYPE_VALUE_TYPE:
            return "argument value";
            break;
    }
    throw std::domain_error("ParseErrorType out of range");
}



template <typename Iterator>
ParseErrorType ParseError<Iterator>::getType() const
{
    return errorType;
}



template <typename Iterator>
Iterator ParseError<Iterator>::getErrorPosition() const
{
    return m_errorPos;
}



template <typename Iterator>
std::vector<std::string> ParseError<Iterator>::getExpectedTypes() const
{
    return expectedTypes;
}



template <typename Iterator>
std::vector<std::string> ParseError<Iterator>::getExpectedKeywords() const
{
    return expectedKeywords;
}



template <typename Iterator>
std::string ParseError<Iterator>::toString() const
{
    std::ostringstream sout;
    sout << "Error while parsing " << parseErrorTypeToString(errorType);
    switch( errorType ) {
        case PARSE_ERROR_TYPE_KIND:
            if( context.empty() )
                sout << ". Unknown top-level kind";
            else
                sout << " of nested object in " << context;
            break;
        case PARSE_ERROR_TYPE_ATTRIBUTE:
        case PARSE_ERROR_TYPE_VALUE_TYPE:
            sout << " for " << context;
            break;
    }
    sout << ". Expected one of [ ";
    for( std::vector<std::string>::const_iterator it = expectedKeywords.begin(); it != expectedKeywords.end(); ++it )
        sout << "\"" << *it << "\" ";
    for( std::vector<std::string>::const_iterator it = expectedTypes.begin(); it != expectedTypes.end(); ++it )
        sout << "<" << *it << "> ";
    sout << "].";
    //sout << "] here: " << std::string( m_errorPos, m_end ) << ".";
    return sout.str();
}



template <typename Iterator>
std::string ParseError<Iterator>::toCombinedString( const ParseError<Iterator> &kindError ) const
{
    std::ostringstream sout;
    sout << "Error while parsing attribute name or nested kind name in " << context << ". Expected one of [ ";
    for( std::vector<std::string>::const_iterator it = expectedKeywords.begin(); it != expectedKeywords.end(); ++it )
        sout << "\"" << *it << "\" ";
    std::vector<std::string> expectedKinds = kindError.getExpectedKeywords();
    for( std::vector<std::string>::const_iterator it = expectedKinds.begin(); it != expectedKinds.end(); ++it )
        sout << "\"" << *it << "\" ";
    sout << "].";
    //sout << "] here: " << std::string( m_errorPos, m_end ) << ".";
    return sout.str();
}



/** @short Tests if error is a real error, or only consequence of failing alternatives or lazy in the parser */
template <typename Iterator>
bool ParseError<Iterator>::valid() const
{
    return ( !expectedKeywords.empty() || !expectedTypes.empty() );
}



template <typename Iterator>
void ParseError<Iterator>::extractKindName( const std::string &name,
    const qi::rule<Iterator, std::string(), ascii::space_type> &rule )
{
    expectedKeywords.push_back(name);
}



template <typename Iterator>
void ParseError<Iterator>::extractAttributeName( const std::string &name,
    const qi::rule<Iterator, Db::Value(), ascii::space_type> &rule )
{
    expectedKeywords.push_back(name);
}



/////////////////////////Template instances for linker//////////////////////////

template void ObjectErrorHandler<iterator_type>::operator()(iterator_type start, iterator_type end, iterator_type errorPos,
    const spirit::info &what, qi::symbols<char, qi::rule<iterator_type, std::string(), ascii::space_type> > kinds,
    const std::string &kindName, ParserImpl<iterator_type>* parser ) const;

template void KeyErrorHandler<iterator_type>::operator()(iterator_type start, iterator_type end, iterator_type errorPos, const spirit::info &what,
    qi::symbols<char, qi::rule<iterator_type, Db::Value(), ascii::space_type> > attributes,
    const std::string &kindName, ParserImpl<iterator_type>* parser ) const;

template void ValueErrorHandler<iterator_type>::operator()( iterator_type start, iterator_type end, iterator_type errorPos, const spirit::info &what, const std::string &attributeName, ParserImpl<iterator_type>* parser ) const;

template ParseErrorType ParseError<iterator_type>::getType() const;

template iterator_type ParseError<iterator_type>::getErrorPosition() const;

template std::vector<std::string> ParseError<iterator_type>::getExpectedTypes() const;

template std::vector<std::string> ParseError<iterator_type>::getExpectedKeywords() const;

template std::string ParseError<iterator_type>::toString() const;

template std::string ParseError<iterator_type>::toCombinedString( const ParseError<iterator_type> &kindError ) const;

}
}
