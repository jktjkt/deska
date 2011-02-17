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


Parser::Parser( Api *dbApi )
{
    m_dbApi = dbApi;
    BOOST_ASSERT( m_dbApi );
}

Parser::~Parser()
{
}

void Parser::parseLine( const std::string &line )
{
    // FIXME: implement me
}



//TEMPLATE INSTANCES FOR LINKER

template PredefinedRules< std::string::const_iterator >::PredefinedRules();

template qi::rule< std::string::const_iterator, boost::variant< int, std::string, double >(), ascii::space_type > PredefinedRules< std::string::const_iterator >::getRule( const std::string &typeName );

}
}
