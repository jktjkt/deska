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

#include <boost/bind.hpp>

#include "ParserErrors.h"
#include "Parser_p.h"

//#define PARSER_DEBUG
//#define PARSER_PRINT_ERRORS

namespace Deska
{
namespace Cli
{



template <typename Iterator>
void ObjectErrorHandler<Iterator>::operator()(Iterator start, Iterator end, Iterator errorPos, const spirit::info& what,
    const qi::symbols<char, qi::rule<Iterator, std::string(), ascii::space_type> > kinds,
    ParserImpl<Iterator> *parser ) const
{
    ParseError<Iterator> error( start, end, errorPos, what, kinds );
    if ( error.valid() )
        parser->addParseError( error );
}



template <typename Iterator>
void KeyErrorHandler<Iterator>::operator()(Iterator start, Iterator end, Iterator errorPos, const spirit::info& what,
    const qi::symbols<char, qi::rule<Iterator, Db::Value(), ascii::space_type> > attributes,
    ParserImpl<Iterator> *parser ) const
{
    ParseError<Iterator> error( start, end, errorPos, what, attributes );
    if ( error.valid() )
        parser->addParseError( error );
}



template <typename Iterator>
void ValueErrorHandler<Iterator>::operator()(Iterator start, Iterator end, Iterator errorPos, const spirit::info& what,
    ParserImpl<Iterator> *parser ) const
{
    ParseError<Iterator> error( start, end, errorPos, what );
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
        kList->push_back( "\"" + value + "\"" );
    else
        tList->push_back( "<" + tag + ">" );
}



template <typename Iterator>
ParseError<Iterator>::ParseError( Iterator start, Iterator end, Iterator errorPos, const spirit::info &what ):
    errorType(PARSE_ERROR_TYPE_VALUE_TYPE), m_start(start), m_end(end), m_errorPos(errorPos)
{
    InfoExtractor extractor( &expectedKeywords, &expectedTypes );
    spirit::basic_info_walker<InfoExtractor> walker( extractor, what.tag, 0 );
    boost::apply_visitor( walker, what.value );
}



template <typename Iterator>
ParseError<Iterator>::ParseError( Iterator start, Iterator end, Iterator errorPos, const spirit::info &what,
    const qi::symbols<char, qi::rule<Iterator, std::string(), ascii::space_type> > &kinds ):
    errorType(PARSE_ERROR_TYPE_KIND), m_start(start), m_end(end), m_errorPos(errorPos)
{
    kinds.for_each( boost::bind( &ParseError<Iterator>::extractKindName, this, _1, _2 ) );
}


template <typename Iterator>
ParseError<Iterator>::ParseError( Iterator start, Iterator end, Iterator errorPos, const spirit::info &what,
    const qi::symbols<char, qi::rule<Iterator, Db::Value(), ascii::space_type> > &attributes ):
    errorType(PARSE_ERROR_TYPE_ATTRIBUTE), m_start(start), m_end(end), m_errorPos(errorPos)
{
    attributes.for_each( boost::bind( &ParseError<Iterator>::extractAttributeName, this, _1, _2 ) );
}

std::string parseErrorTypeToString(const ParseErrorType errorType)
{
    switch( errorType ) {
        case PARSE_ERROR_TYPE_KIND:
            return "kind name";
        case PARSE_ERROR_TYPE_ATTRIBUTE:
            return "attribute name";
        case PARSE_ERROR_TYPE_VALUE_TYPE:
            return "attribute value";
            break;
    }
    throw std::domain_error("ParseErrorType out of range");
}

template <typename Iterator>
std::string ParseError<Iterator>::toString()
{
    std::ostringstream sout;
    sout << "Error while parsing " << parseErrorTypeToString(errorType) << ". Expected one of [";
    std::copy( expectedKeywords.begin(), expectedKeywords.end(), std::ostream_iterator<std::string>(sout, " ") );
    std::copy( expectedTypes.begin(), expectedTypes.end(), std::ostream_iterator<std::string>(sout, " ") );
    sout << "] here: " << std::string( m_errorPos, m_end ) << ".";
    return sout.str();
}



/** @short Tests if error is a real error, or only consequence of failing alternatives or lazy in the parser */
template <typename Iterator>
bool ParseError<Iterator>::valid()
{
    return ( !expectedKeywords.empty() || !expectedTypes.empty() );
}



template <typename Iterator>
void ParseError<Iterator>::extractKindName( const std::string &name,
    const qi::rule<Iterator, std::string(), ascii::space_type> &rule )
{
    expectedKeywords.push_back( "\"" + name + "\"" );
}



template <typename Iterator>
void ParseError<Iterator>::extractAttributeName( const std::string &name,
    const qi::rule<Iterator, Db::Value(), ascii::space_type> &rule )
{
    expectedKeywords.push_back( "\"" + name + "\"" );
}



/////////////////////////Template instances for linker//////////////////////////

template void ObjectErrorHandler<iterator_type>::operator()(iterator_type start, iterator_type end, iterator_type errorPos,
    const spirit::info &what, qi::symbols<char, qi::rule<iterator_type, std::string(), ascii::space_type> > kinds,
    ParserImpl<iterator_type>* parser ) const;

template void KeyErrorHandler<iterator_type>::operator()(iterator_type start, iterator_type end, iterator_type errorPos, const spirit::info &what,
    qi::symbols<char, qi::rule<iterator_type, Db::Value(), ascii::space_type> > attributes,
    ParserImpl<iterator_type>* parser ) const;

template void ValueErrorHandler<iterator_type>::operator()( iterator_type start, iterator_type end, iterator_type errorPos, const spirit::info &what, ParserImpl<iterator_type>* parser ) const;

template std::string ParseError<iterator_type>::toString();

}
}
