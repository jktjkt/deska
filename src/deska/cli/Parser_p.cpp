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
#include "Parser_p.h"


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
    rulesMap[TYPE_INT] = qi::int_
        [ qi::_val = phoenix::static_cast_<int>( qi::_1 ) ];
    rulesMap[TYPE_INT].name( "integer" );

    // FIXME: consider allowing trivial words without quotes
    rulesMap[TYPE_STRING] %= qi::lexeme[ '"' >> +( ascii::char_ - '"' ) >> '"' ];
    rulesMap[TYPE_STRING].name( "quoted string" );

    rulesMap[TYPE_DOUBLE] = qi::double_
        [ qi::_val = phoenix::static_cast_<double>( qi::_1 ) ];
    rulesMap[TYPE_DOUBLE].name( "double" );

    rulesMap[TYPE_IDENTIFIER] %= qi::lexeme[ *( ascii::alnum | '_' ) ];
    rulesMap[TYPE_IDENTIFIER].name( "identifier (alphanumerical letters and _)" );

    objectIdentifier %= qi::lexeme[ *( ascii::alnum | '_' ) ];
    objectIdentifier.name( "object identifier (alphanumerical letters and _)" );
}



template <typename Iterator>
const qi::rule<Iterator, Value(), ascii::space_type>& PredefinedRules<Iterator>::getRule(const Type attrType)
{
    std::map<Type, qi::rule<Iterator, Value(), ascii::space_type> >::const_iterator it = rulesMap.find( attrType );
    if ( it == rulesMap.end() )
        throw 123;
    else
        return it->second;
}



template <typename Iterator>
const qi::rule<Iterator, std::string(), ascii::space_type>& PredefinedRules<Iterator>::getObjectIdentifier()
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

    objectKindName = kindName;
    this->name( kindName );

    phoenix::function<RangeToString<Iterator> > rangeToString = RangeToString<Iterator>();

    start = +( ( raw[ attributes[ _a = _1 ] ][ rangeToString( _1, _b ) ]
        > lazy( _a )[ phoenix::bind( &AttributesParser::parsedAttribute, this, _b, _1 ) ] ) );

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
   return objectKindName;
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

    start = ( raw[ kinds[ _a = _1 ] ][ rangeToString( _1, _b ) ]
        > lazy( _a )[ phoenix::bind( &TopLevelParser::parsedKind, this, _b, _1 ) ] );

    phoenix::function<ErrorHandler<Iterator> > errorHandler = ErrorHandler<Iterator>();
    on_error<fail>( start, errorHandler( _1, _2, _3, _4 ) );
}



template <typename Iterator>
void TopLevelParser<Iterator>::addKind(
    const std::string &kindName,
    qi::rule<Iterator, std::string(), ascii::space_type> identifierParser )
{
    kinds.add( kindName, identifierParser );
}



template <typename Iterator>
void TopLevelParser<Iterator>::parsedKind( const std::string &kindName, const std::string &objectName )
{
    std::cout << "Parsed kind: " << kindName << ": " << objectName << std::endl;
    kindParsed = kindName;
}



template <typename Iterator>
ParserImpl<Iterator>::ParserImpl(Parser *parent): m_parser(parent)
{
    predefinedRules = new PredefinedRules<Iterator>();
    topLevelParser = new TopLevelParser<Iterator>();

    // Filling the AttributesParsers map
    std::vector<std::string> kinds = m_parser->m_dbApi->kindNames();

    for( std::vector<std::string>::iterator it = kinds.begin(); it != kinds.end(); ++it ) {
        topLevelParser->addKind( *it, predefinedRules->getObjectIdentifier() );

        attributesParsers[ *it ] = new AttributesParser<Iterator>( *it );
        addKindAttributes( *it, attributesParsers[ *it ] );

        std::vector<ObjectRelation> relations = m_parser->m_dbApi->kindRelations( *it );
        for( std::vector<ObjectRelation>::iterator itRel = relations.begin(); itRel != relations.end(); ++itRel )
            if( itRel->kind == RELATION_MERGE_WITH )
                addKindAttributes( itRel->destinationAttribute, attributesParsers[ *it ] );
    }
}



template <typename Iterator>
ParserImpl<Iterator>::~ParserImpl()
{
    for( typename std::map<std::string, AttributesParser<Iterator>* >::iterator
        it = attributesParsers.begin();
        it != attributesParsers.end();
        ++it )
        delete it->second;

    delete topLevelParser;

    delete predefinedRules;
}



template <typename Iterator>
void ParserImpl<Iterator>::parseLine( const std::string &line )
{
    // TODO: Only testing implementation. Reimplement.
    Iterator iter = line.begin();
    Iterator end = line.end();
    std::cout << "Parse line: " << std::string( iter, end ) << std::endl;
    std::cout << "Parsing top level object..." << std::endl;
    bool r = phrase_parse( iter, end, *topLevelParser, ascii::space );

    if ( r ) {
        if ( iter == end ) {
            std::cout << "Parsing succeeded. Full match." << std::endl;
        }
        else {
            std::cout << "Parsing succeeded. Partial match." << std::endl;
            std::cout << "Remaining: " << std::string( iter, end ) << std::endl;
            std::cout << "Parsing attributes for \"" << kindParsed << "\"..." << std::endl;
            bool r2 = phrase_parse( iter, end, *( attributesParsers[ kindParsed ] ), ascii::space );
            if ( r2 ) {
                if ( iter == end ) {
                    std::cout << "Parsing succeeded. Full match." << std::endl;
                }
                else {
                    std::cout << "Parsing succeeded. Partial match." << std::endl;
                    std::cout << "Remaining: " << std::string( iter, end ) << std::endl;
                }
            }
            else {
                std::cout << "Parsing failed." << std::endl;
            }
        }
    }
    else {
        std::cout << "Parsing failed." << std::endl;
    }
}



template <typename Iterator>
bool ParserImpl<Iterator>::isNestedInContext() const
{
    return false;
}



template <typename Iterator>
std::vector<AttributeDefinition> ParserImpl<Iterator>::currentContextStack() const
{
    return std::vector<AttributeDefinition>();
}



template <typename Iterator>
void ParserImpl<Iterator>::addKindAttributes(
    std::string &kindName,
    AttributesParser<Iterator>* attributeParser )
{
    std::vector<KindAttributeDataType> attributes = m_parser->m_dbApi->kindAttributes( kindName );
    for( std::vector<KindAttributeDataType>::iterator it = attributes.begin(); it != attributes.end(); ++it )
        attributeParser->addAtrribute( it->name, predefinedRules->getRule( it->type ) );
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

template const qi::rule<
    iterator_type,
    Value(),
    ascii::space_type>& PredefinedRules<iterator_type>::getRule(const Type attrType);

template const qi::rule<
    iterator_type,
    std::string(),
    ascii::space_type>& PredefinedRules<iterator_type>::getObjectIdentifier();

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

template void TopLevelParser<iterator_type>::addKind(
    const std::string &kindName,
    qi::rule<iterator_type, std::string(), ascii::space_type> identifierParser );

template void TopLevelParser<iterator_type>::parsedKind(
    const std::string &kindName,
    const std::string &objectName );

template ParserImpl<iterator_type>::ParserImpl(Parser *parent);

template ParserImpl<iterator_type>::~ParserImpl();

template void ParserImpl<iterator_type>::parseLine( const std::string &line );

template bool ParserImpl<iterator_type>::isNestedInContext() const;

template std::vector<AttributeDefinition> ParserImpl<iterator_type>::currentContextStack() const;

template void ParserImpl<iterator_type>::addKindAttributes(
    std::string &kindName,
    AttributesParser<iterator_type>* attributeParser );

}
}
