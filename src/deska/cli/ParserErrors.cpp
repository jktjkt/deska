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
namespace CLI
{

template <typename Iterator>
void RangeToString<Iterator>::operator()( const boost::iterator_range<Iterator> &range, std::string &str ) const
{
    str.assign( range.begin(), range.end() );
}



template <typename Iterator>
void ObjectErrorHandler<Iterator>::operator()(Iterator start, Iterator end, Iterator errorPos, const spirit::info& what,
    const qi::symbols<char, qi::rule<Iterator, std::string(), ascii::space_type> > kinds,
    ParserImpl<Iterator> *parser ) const
{
    parser->addParseError( ParseError<Iterator>( start, end, errorPos, what, kinds ) );
}



template <typename Iterator>
void KeyErrorHandler<Iterator>::operator()(Iterator start, Iterator end, Iterator errorPos, const spirit::info& what,
    const qi::symbols<char, qi::rule<Iterator, Value(), ascii::space_type> > attributes,
    ParserImpl<Iterator> *parser ) const
{
    parser->addParseError( ParseError<Iterator>( start, end, errorPos, what, attributes ) );
}



template <typename Iterator>
void ValueErrorHandler<Iterator>::operator()(Iterator start, Iterator end, Iterator errorPos, const spirit::info& what,
    ParserImpl<Iterator> *parser ) const
{
    parser->addParseError( ParseError<Iterator>( start, end, errorPos, what ) );
}



InfoExtractor::InfoExtractor( std::vector<std::string> *keywordsList ):
    list(keywordsList)
{
}



void InfoExtractor::element( spirit::utf8_string const& tag, spirit::utf8_string const& value, int ) const
{
    if ( value.empty() )
        list->push_back( "\"" + value + "\"" );
    else
        list->push_back( "<" + tag + ">" );
}



template <typename Iterator>
ParseError<Iterator>::ParseError( Iterator start, Iterator end, Iterator errorPos, const spirit::info &what ):
    errorType(PARSE_ERROR_TYPE_VALUE_TYPE), m_start(start), m_end(end), m_errorPos(errorPos)
{
    InfoExtractor extractor( &expectedKeywords );
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
    const qi::symbols<char, qi::rule<Iterator, Value(), ascii::space_type> > &attributes ):
    errorType(PARSE_ERROR_TYPE_ATTRIBUTE), m_start(start), m_end(end), m_errorPos(errorPos)
{
    attributes.for_each( boost::bind( &ParseError<Iterator>::extractAttributeName, this, _1, _2 ) );
}



template <typename Iterator>
std::string ParseError<Iterator>::toString()
{
    std::ostringstream sout;
    switch( errorType ) {
        case PARSE_ERROR_TYPE_KIND:
            sout << "Error while parsing kind name. Epected";
            break;
        case PARSE_ERROR_TYPE_ATTRIBUTE:
            sout << "Error while parsing attribute name. Epected";
            break;
        case PARSE_ERROR_TYPE_VALUE_TYPE:
            sout << "Error while parsing attribute value. Epected";
            break;
    }
    for( std::vector<std::string>::iterator it = expectedKeywords.begin(); it != expectedKeywords.end(); ++it ) {
        sout << " " << *it;
    }
    sout << " here: " << std::string( m_errorPos, m_end ) << ".";

    return sout.str();
}



template <typename Iterator>
void ParseError<Iterator>::extractKindName( const std::string &name,
    const qi::rule<Iterator, std::string(), ascii::space_type> &rule )
{
    expectedKeywords.push_back( "\"" + name + "\"" );
}



template <typename Iterator>
void ParseError<Iterator>::extractAttributeName( const std::string &name,
    const qi::rule<Iterator, Value(), ascii::space_type> &rule )
{
    expectedKeywords.push_back( "\"" + name + "\"" );
}



/////////////////////////Template instances for linker//////////////////////////

template void RangeToString<iterator_type>::operator()(
    const boost::iterator_range<iterator_type> &rng, std::string &str ) const;

template void ObjectErrorHandler<iterator_type>::operator()(iterator_type start, iterator_type end, iterator_type errorPos,
    const spirit::info &what, qi::symbols<char, qi::rule<iterator_type, std::string(), ascii::space_type> > kinds,
    ParserImpl<iterator_type>* parser ) const;

template void KeyErrorHandler<iterator_type>::operator()(iterator_type start, iterator_type end, iterator_type errorPos, const spirit::info &what,
    qi::symbols<char, qi::rule<iterator_type, Value(), ascii::space_type> > attributes,
    ParserImpl<iterator_type>* parser ) const;

template void ValueErrorHandler<iterator_type>::operator()( iterator_type start, iterator_type end, iterator_type errorPos, const spirit::info &what, ParserImpl<iterator_type>* parser ) const;

}
}
