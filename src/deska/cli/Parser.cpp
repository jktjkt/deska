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
void RangeToString<Iterator>::operator()( const boost::iterator_range<Iterator> &rng, std::string &str ) const
{
    str.assign( rng.begin(), rng.end() );
}


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
    qi::rule<Iterator, Value(), ascii::space_type> t_int;
    t_int %= qi::attr( int() ) >> qi::int_;
    t_int.name( "integer" );
    rulesMap[ "integer" ] = t_int;
   
    qi::rule<Iterator, Value(), ascii::space_type> t_string;
    t_string %= qi::attr( std::string() ) >> qi::lexeme[ '"' >> +( ascii::char_ - '"' ) >> '"' ];
    t_string.name( "quoted string" );
    rulesMap[ "quoted_string" ] = t_string;
    
    qi::rule<Iterator, Value(), ascii::space_type> t_double;
    t_double %= qi::attr( double() ) >> qi::double_;
    t_double.name( "double" );
    rulesMap[ "double" ] = t_double;

    qi::rule<Iterator, Value(), ascii::space_type> identifier;
    identifier %= qi::attr( std::string() ) >> qi::lexeme[ *( ascii::alnum | '_' ) ];
    identifier.name( "identifier (alphanumerical letters and _)" );
    rulesMap[ "identifier" ] = identifier;
}



template <typename Iterator>
qi::rule<Iterator, Value(), ascii::space_type> PredefinedRules<Iterator>::getRule( const std::string &typeName )
{
    return rulesMap[ typeName ];
}



template <typename Iterator>
qi::rule<Iterator, std::string(), ascii::space_type> PredefinedRules<Iterator>::getObjectIdentifier()
{
    return objectIdentifier;
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
    using qi::_b;
    using qi::_val;
    using qi::raw;
    using qi::on_error;
    using qi::fail;

    name = kindName;
    //this->name( kindName );

    phoenix::function<RangeToString<Iterator> > rangeToString = RangeToString<Iterator>();

    start = +( ( raw[ attributes[ _a = _1 ] ][ rangeToString( _1, _b ) ] > lazy( _a )[ phoenix::bind( &AttributesParser::parsedAttribute, this, _b, _1 ) ] ) );

    phoenix::function<ErrorHandler<Iterator> > errorHandler = ErrorHandler<Iterator>();
    on_error<fail>( start, errorHandler( _1, _2, _3, _4 ) );
}



template <typename Iterator>
void AttributesParser<Iterator>::addAtrribute(
    const std::string &attributeName,
    qi::rule<Iterator, Value(), ascii::space_type> attributeParser )
{
    attributes.add( attributeName, attributeParser );
}



template <typename Iterator>
std::string AttributesParser<Iterator>::getKindName() const
{
   return name;
}



template <typename Iterator>
void AttributesParser<Iterator>::parsedAttribute( const std::string &parameter, Value &value )
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
    using qi::_b;
    using qi::raw;
    using qi::on_error;
    using qi::fail;

    phoenix::function<RangeToString<Iterator> > rangeToString = RangeToString<Iterator>();

    start = ( kinds[ _a = _1 ] > lazy( _a ) );

    start = ( raw[ kinds[ _a = _1 ] ][ rangeToString( _1, _b ) ] > lazy( _a )[ phoenix::bind( &TopLevelParser::parsedKind, this, _b, _1 ) ] );

    phoenix::function<ErrorHandler<Iterator> > errorHandler = ErrorHandler<Iterator>();
    on_error<fail>( start, errorHandler( _1, _2, _3, _4 ) );
}



template <typename Iterator>
void TopLevelParser<Iterator>::addKind( const std::string &kindName )
{
    PredefinedRules<Iterator> predefined = PredefinedRules<Iterator>();
    kinds.add( kindName, predefined.getObjectIdentifier() );
}



template <typename Iterator>
void TopLevelParser<Iterator>::parsedKind( const std::string &kindName, const std::string &objectName )
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
std::vector<AttributeDefinition> Parser<Iterator>::currentContextStack() const
{
    return std::vector<AttributeDefinition>();
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

template void RangeToString<iterator_type>::operator()(
    const boost::iterator_range<iterator_type> &rng, std::string &str ) const;

template void ErrorHandler<iterator_type>::operator()(
    iterator_type start,
    iterator_type end,
    iterator_type errorPos,
    const spirit::info &what ) const;

template PredefinedRules<iterator_type>::PredefinedRules();

template qi::rule<
    iterator_type,
    Value(),
    ascii::space_type> PredefinedRules<iterator_type>::getRule( const std::string &typeName );

template qi::rule<
    iterator_type,
    std::string(),
    ascii::space_type> PredefinedRules<iterator_type>::getObjectIdentifier();

template AttributesParser<iterator_type>::AttributesParser(
    const std::string &kindName );

template void AttributesParser<iterator_type>::addAtrribute(
    const std::string &attributeName,
    qi::rule<
        iterator_type,
        Value(),
        ascii::space_type> attributeParser );

template std::string AttributesParser<iterator_type>::getKindName() const;

template void AttributesParser<iterator_type>::parsedAttribute(
    const std::string &parameter,
    Value &value );

template TopLevelParser<iterator_type>::TopLevelParser();

template void TopLevelParser<iterator_type>::addKind( const std::string &kindName );

template void TopLevelParser<iterator_type>::parsedKind(
    const std::string &kindName,
    const std::string &objectName );

template Parser<iterator_type>::Parser( Api* dbApi );

template Parser<iterator_type>::~Parser();

template void Parser<iterator_type>::parseLine( const std::string &line );

template bool Parser<iterator_type>::isNestedInContext() const;

template std::vector<AttributeDefinition> Parser<iterator_type>::currentContextStack() const;

template void Parser<iterator_type>::addKindAttributes(
    std::string &kindName,
    AttributesParser<iterator_type>* attributeParser );

}
}
