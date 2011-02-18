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

#include <vector>
#include <boost/assert.hpp>
#include "Parser.h"


namespace Deska {
namespace CLI {


template <typename Iterator>
void ErrorHandler<Iterator>::operator()(
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


template <typename Iterator>
PredefinedRules<Iterator>::PredefinedRules()
{
    qi::rule<Iterator, Variant(), ascii::space_type> t_int;
    t_int %= qi::attr( int() ) >> qi::int_;
    t_int.name( "integer" );
    rulesMap[ "integer" ] = t_int;
   
    qi::rule<Iterator, Variant(), ascii::space_type> t_string;
    t_string %= qi::attr( std::string() ) >> qi::lexeme[ '"' >> +( ascii::char_ - '"' ) >> '"' ];
    t_string.name( "quoted string" );
    rulesMap[ "quoted_string" ] = t_string;
    
    qi::rule<Iterator, Variant(), ascii::space_type> t_double;
    t_double %= qi::attr( double() ) >> qi::double_;
    t_double.name( "double" );
    rulesMap[ "double" ] = t_double;

    qi::rule<Iterator, Variant(), ascii::space_type> identifier;
    identifier %= qi::attr( std::string() ) >> qi::lexeme[ *( ascii::alnum | '_' ) ];
    identifier.name( "identifier (alphanumerical letters and _)" );
    rulesMap[ "identifier" ] = identifier;
}



template <typename Iterator>
qi::rule<Iterator, Variant(), ascii::space_type> PredefinedRules<Iterator>::getRule( const std::string &typeName )
{
    return rulesMap[ typeName ];
}


template <typename Iterator>
AttributesParser<Iterator>::AttributesParser(
    const std::string &kindName ): AttributesParser<Iterator>::base_type( start )
{ 
    using qi::_1;
    using qi::_2;
    using qi::_3;
    using qi::_4;
    using qi::_a;
    using qi::on_error;
    using qi::fail;

    name = kindName;

    start = +( attributes[ _a = _1 ] > lazy( _a ) );// FIXME [ boost::bind( &AttributesParser::parsedAttribute, this, _a, _1 ) ] );

    phoenix::function<ErrorHandler<Iterator> > errorHandler = ErrorHandler<Iterator>();
    on_error<fail>( start, errorHandler( _1, _2, _3, _4 ) );
}



template <typename Iterator>
void AttributesParser<Iterator>::addAtrribute(
    const std::string &attributeName,
    qi::rule<Iterator, Variant(), ascii::space_type> attributeParser )
{
    attributes.add( attributeName, attributeParser );
}



template <typename Iterator>
std::string AttributesParser<Iterator>::getKindName() const
{
   return name;
}



template <typename Iterator>
void AttributesParser<Iterator>::parsedAttribute( const char* parameter, Variant value )
{
    std::cout << "Parsed parameter: " << parameter << "=" << value << std::endl;
}



template <typename Iterator>
TopLevelParser<Iterator>::TopLevelParser(): TopLevelParser<Iterator>::base_type( start )
{ 
    using qi::_1;
    using qi::_2;
    using qi::_3;
    using qi::_4;
    using qi::_a;
    using qi::on_error;
    using qi::fail;

    start = ( kinds[ _a = _1 ] > lazy( _a ) );//FIXME [ boost::bind( &TopLevelParser::parsedKind, this, _a, _1 ) ] );

    phoenix::function<ErrorHandler<Iterator> > errorHandler = ErrorHandler<Iterator>();
    on_error<fail>( start, errorHandler( _1, _2, _3, _4 ) );
}



template <typename Iterator>
void TopLevelParser<Iterator>::addKind( const std::string &kindName )
{
    PredefinedRules<Iterator> predefined = PredefinedRules<Iterator>();
   // FIXME kinds.add( kindName, predefined.getRule( "identifier" ) );
}



template <typename Iterator>
void TopLevelParser<Iterator>::parsedKind( const char* kindName, const std::string &objectName )
{
    std::cout << "Parsed kind: " << kindName << " " << objectName << std::endl;
}



template <typename Iterator>
Parser<Iterator>::Parser( Api *dbApi )
{
    m_dbApi = dbApi;
    BOOST_ASSERT( m_dbApi );

    topLevelParser = new TopLevelParser<Iterator>();

    // Filling the AttributesParsers map
    std::vector<std::string> kinds = m_dbApi->kindNames();

    for( std::vector<std::string>::iterator it = kinds.begin(); it != kinds.end(); ++it ) {
        topLevelParser->addKind( *it );

        attributesParsers[ *it ] = new AttributesParser<Iterator>( *it );
        addKindAttributes( *it, attributesParsers[ *it ] );
        
        std::vector<ObjectRelation> relations = m_dbApi->kindRelations( *it );
        for( std::vector<ObjectRelation>::iterator itRel = relations.begin(); itRel != relations.end(); ++itRel )
            if( itRel->kind == RELATION_MERGE_WITH )
                addKindAttributes( itRel->destinationAttribute, attributesParsers[ *it ] );
    }
}



template <typename Iterator>
Parser<Iterator>::~Parser()
{
    for( typename std::map<std::string, AttributesParser<Iterator>* >::iterator
        it = attributesParsers.begin();
        it != attributesParsers.end();
        ++it )
        delete it->second;

    delete topLevelParser;
}



template <typename Iterator>
void Parser<Iterator>::parseLine( const std::string &line )
{
    // FIXME: implement me
}



template <typename Iterator>
bool Parser<Iterator>::isNestedInContext() const
{
    return false;
}



template <typename Iterator>
std::vector<std::pair<Identifier, Identifier> > Parser<Iterator>::currentContextStack() const
{
    return std::vector<std::pair<Identifier, Identifier> >();
}



template <typename Iterator>
void Parser<Iterator>::addKindAttributes(
    std::string &kindName,
    AttributesParser<Iterator>* attributeParser )
{
    PredefinedRules<Iterator> predefined = PredefinedRules<Iterator>();

    std::vector<KindAttributeDataType> attributes = m_dbApi->kindAttributes( kindName );
    for( std::vector<KindAttributeDataType>::iterator it = attributes.begin(); it != attributes.end(); ++it )
        attributeParser->addAtrribute( it->name, predefined.getRule( it->type ) );
}



/////////////////////////Template instances for linker//////////////////////////

template void ErrorHandler<std::string::const_iterator>::operator()(
    std::string::const_iterator start,
    std::string::const_iterator end,
    std::string::const_iterator errorPos,
    const spirit::info& what ) const;

template PredefinedRules<std::string::const_iterator>::PredefinedRules();

template qi::rule<
    std::string::const_iterator,
    Variant(),
    ascii::space_type> PredefinedRules<std::string::const_iterator>::getRule( const std::string &typeName );

template AttributesParser<std::string::const_iterator>::AttributesParser(
    const std::string &kindName );

template void AttributesParser<std::string::const_iterator>::addAtrribute(
    const std::string &attributeName,
    qi::rule<
        std::string::const_iterator,
        Variant(),
        ascii::space_type> attributeParser );

template std::string AttributesParser<std::string::const_iterator>::getKindName() const;

template void AttributesParser<std::string::const_iterator>::parsedAttribute(
    const char* parameter,
    Variant value );

template TopLevelParser<std::string::const_iterator>::TopLevelParser();

template void TopLevelParser<std::string::const_iterator>::addKind( const std::string &kindName );

template void TopLevelParser<std::string::const_iterator>::parsedKind(
    const char* kindName,
    const std::string &objectName );

template Parser<std::string::const_iterator>::Parser( Api* dbApi );

template Parser<std::string::const_iterator>::~Parser();

template void Parser<std::string::const_iterator>::parseLine( const std::string &line );

template bool Parser<std::string::const_iterator>::isNestedInContext() const;

template std::vector<std::pair<Identifier, Identifier> > Parser<std::string::const_iterator>::currentContextStack() const;

template void Parser<std::string::const_iterator>::addKindAttributes(
    std::string &kindName,
    AttributesParser<std::string::const_iterator>* attributeParser );

}
}
