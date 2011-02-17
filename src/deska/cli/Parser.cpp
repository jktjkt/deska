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

#include <boost/assert.hpp>
#include "Parser.h"


namespace Deska {
namespace CLI {


template < typename Iterator >
void ErrorHandler< Iterator >::operator()(
    Iterator start,
    Iterator end,
    Iterator errorPos,
    const spirit::info& what ) const
{
    std::cout
        << "Error! Expecting " << what
        << " here: \"" << std::string( errorPos, end ) << "\""
        << std::endl;
}


template < typename Iterator >
PredefinedRules< Iterator >::PredefinedRules()
{
    qi::rule< Iterator, boost::variant< int, std::string, double >(), ascii::space_type > t_int;
    t_int %= qi::attr( int() ) >> qi::int_;
    t_int.name( "integer" );
    rulesMap[ "integer" ] = t_int;
   
    qi::rule< Iterator, boost::variant< int, std::string, double>(), ascii::space_type > t_string;
    t_string %= qi::attr( std::string() ) >> qi::lexeme[ '"' >> +( ascii::char_ - '"' ) >> '"' ];
    t_string.name( "quoted string" );
    rulesMap[ "quoted_string" ] = t_string;
    
    qi::rule< Iterator, boost::variant< int, std::string, double >(), ascii::space_type > t_double;
    t_double %= qi::attr( double() ) >> qi::double_;
    t_double.name( "double" );
    rulesMap[ "double" ] = t_double;

    qi::rule< Iterator, boost::variant< int, std::string, double >(), ascii::space_type > identifier;
    identifier %= qi::attr( std::string() ) >> qi::lexeme[ *( ascii::alnum | '_' ) ];
    identifier.name( "identifier (alphanumerical letters and _)" );
    rulesMap[ "identifier" ] = identifier;
}



template < typename Iterator >
qi::rule< Iterator, boost::variant< int, std::string, double >(), ascii::space_type > PredefinedRules< Iterator >::getRule( const std::string &typeName )
{
    return rulesMap[ typeName ];
}


template < typename Iterator >
AttributesParser< Iterator >::AttributesParser(
    const std::string &kindName ): AttributesParser< Iterator >::base_type( start )
{ 
    using qi::_1;
    using qi::_2;
    using qi::_3;
    using qi::_4;
    using qi::_a;
    using qi::on_error;
    using qi::fail;

    name = kindName;

    start = +( attributes[ _a = _1 ] > lazy( _a ) );//[ boost::bind( &AttributesParser::parsedAttribute, this, _a, _1 ) ] );

    phoenix::function< ErrorHandler< Iterator > > errorHandler = ErrorHandler< Iterator >();
    on_error< fail >( start, errorHandler( _1, _2, _3, _4 ) );
}



template < typename Iterator >
void AttributesParser< Iterator >::addAtrribute(
    const std::string &attributeName,
    qi::rule< Iterator, boost::variant< int, std::string, double >(), ascii::space_type > attributeParser )
{
    attributes.add( attributeName, attributeParser );
}



template < typename Iterator >
std::string AttributesParser< Iterator >::getKindName() const
{
   return name;
}



template < typename Iterator >
void AttributesParser< Iterator >::parsedAttribute( const char* parameter, boost::variant< int, std::string, double > value )
{
    std::cout << "Parsed parameter: " << parameter << "=" << value << std::endl;
}



template < typename Iterator >
Parser< Iterator >::Parser( Api *dbApi )
{
    m_dbApi = dbApi;
    BOOST_ASSERT( m_dbApi );



}



template < typename Iterator >
Parser< Iterator >::~Parser()
{
    for( typename std::map< std::string, AttributesParser< Iterator >* >::iterator
        it = attributesParsers.begin();
        it != attributesParsers.end();
        ++it )
        delete it->second;
}



template < typename Iterator >
void Parser< Iterator >::parseLine( const std::string &line )
{
    // FIXME: implement me
}



template < typename Iterator >
bool Parser< Iterator >::isNestedInContext() const
{
    return false;
}



template < typename Iterator >
std::vector< std::pair< Identifier, Identifier > > Parser< Iterator >::currentContextStack() const
{
    return std::vector< std::pair< Identifier, Identifier > >();
}



//TEMPLATE INSTANCES FOR LINKER

template void ErrorHandler< std::string::const_iterator >::operator()(
    std::string::const_iterator start,
    std::string::const_iterator end,
    std::string::const_iterator errorPos,
    const spirit::info& what ) const;

template PredefinedRules< std::string::const_iterator >::PredefinedRules();

template qi::rule<
    std::string::const_iterator,
    boost::variant< int, std::string, double >(),
    ascii::space_type > PredefinedRules< std::string::const_iterator >::getRule( const std::string &typeName );

template AttributesParser< std::string::const_iterator >::AttributesParser(
    const std::string &kindName );

template void AttributesParser< std::string::const_iterator >::addAtrribute(
    const std::string &attributeName,
    qi::rule<
        std::string::const_iterator,
        boost::variant< int, std::string, double >(),
        ascii::space_type > attributeParser );

template std::string AttributesParser< std::string::const_iterator >::getKindName() const;

template Parser< std::string::const_iterator >::Parser( Api* dbApi );

template Parser< std::string::const_iterator >::~Parser();

template void Parser< std::string::const_iterator >::parseLine( const std::string &line );

template bool Parser< std::string::const_iterator >::isNestedInContext() const;

template std::vector< std::pair< Identifier, Identifier > > Parser< std::string::const_iterator >::currentContextStack() const;

}
}
