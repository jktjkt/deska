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
//#include <boost/regex.hpp>
#include "Parser_p.h"


namespace Deska
{
namespace CLI
{


template <typename Iterator>
void RangeToString<Iterator>::operator()( const boost::iterator_range<Iterator> &rng, std::string &str ) const
{
    str.assign( rng.begin(), rng.end() );
}



template <typename Iterator>
void ObjectErrorHandler<Iterator>::operator()(
    Iterator start,
    Iterator end,
    Iterator errorPos,
    const spirit::info& what,
    qi::symbols<char, qi::rule<Iterator, std::string(), ascii::space_type> > kinds ) const
{
    std::cout << "Error in object type parsing! Expecting ";
    // FIXME: Resolve problem with calling of for_each
    // kinds.for_each( printKindName );
    std::cout << "<here will be list of kinds names> ";
    std::cout << "here: \"" << std::string( errorPos, end ) << "\"" << std::endl;
}



template <typename Iterator>
void ObjectErrorHandler<Iterator>::printKindName(
    const std::string &name,
    const qi::rule<Iterator, std::string(), ascii::space_type> &rule )
{
    std::cout << name << " ";
}



template <typename Iterator>
void KeyErrorHandler<Iterator>::operator()(
    Iterator start,
    Iterator end,
    Iterator errorPos,
    const spirit::info& what,
    qi::symbols<char, qi::rule<Iterator, Value(), ascii::space_type> > attributes ) const
{
    std::cout << "Error in attribute name parsing! Expecting ";
    // FIXME: Resolve problem with calling of for_each
    // attributes.for_each( printAttributeName );
    std::cout << "<here will be list of attributes names> ";
    std::cout << "here: \"" << std::string( errorPos, end ) << "\"" << std::endl;
}



template <typename Iterator>
void KeyErrorHandler<Iterator>::printAttributeName(
    const std::string &name,
    const qi::rule<Iterator, Value(), ascii::space_type> &rule )
{
    std::cout << name << " ";
}



template <typename Iterator>
void ValueErrorHandler<Iterator>::operator()(
    Iterator start,
    Iterator end,
    Iterator errorPos,
    const spirit::info& what ) const
{
    std::cout
        << "Error in value parsing! Expecting " << what
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

    rulesMap[TYPE_IDENTIFIER] %= qi::lexeme[ +( ascii::alnum | '_' ) ];
    rulesMap[TYPE_IDENTIFIER].name( "identifier (alphanumerical letters and _)" );

    objectIdentifier %= qi::lexeme[ +( ascii::alnum | '_' ) ];
    objectIdentifier.name( "object identifier (alphanumerical letters and _)" );
}



template <typename Iterator>
const qi::rule<Iterator, Value(), ascii::space_type>& PredefinedRules<Iterator>::getRule( const Type attrType )
{
    typename std::map<Type, qi::rule<Iterator, Value(), ascii::space_type> >::const_iterator it = rulesMap.find( attrType );
    if ( it == rulesMap.end() )
        // TODO: Create some exceptions hierarchy
        throw std::domain_error( "Unknown type" );
    else
        return it->second;
}



template <typename Iterator>
const qi::rule<Iterator, std::string(), ascii::space_type>& PredefinedRules<Iterator>::getObjectIdentifier()
{
    return objectIdentifier;
}



template <typename Iterator>
AttributesParser<Iterator>::AttributesParser( const std::string &kindName, ParserImpl<Iterator> *parent ):
    AttributesParser<Iterator>::base_type( start ), m_parent( parent )
{
    using qi::_1;
    using qi::_2;
    using qi::_3;
    using qi::_4;
    using qi::_a;
    using qi::_b;
    using qi::eps;
    using qi::raw;
    using qi::eoi;
    using qi::on_error;
    using qi::fail;

    this->name( kindName );

    phoenix::function<RangeToString<Iterator> > rangeToString = RangeToString<Iterator>();

    start = +( eps( !_a ) > dispatch >> -eoi[ _a = true ] );

    dispatch = ( ( raw[ attributes[ _a = _1 ] ][ rangeToString( _1, _b ) ]
        > lazy( _a )[ phoenix::bind( &AttributesParser::parsedAttribute, this, _b, _1 ) ] ) );

    phoenix::function<KeyErrorHandler<Iterator> > keyErrorHandler = KeyErrorHandler<Iterator>();
    phoenix::function<ValueErrorHandler<Iterator> > valueErrorHandler = ValueErrorHandler<Iterator>();
    on_error<fail>( start, keyErrorHandler( _1, _2, _3, _4, phoenix::ref( attributes ) ) );
    on_error<fail>( dispatch, valueErrorHandler( _1, _2, _3, _4 ) );
}



template <typename Iterator>
void AttributesParser<Iterator>::addAtrribute(
    const std::string &attributeName,
    qi::rule<Iterator, Value(), ascii::space_type> attributeParser )
{
    attributes.add( attributeName, attributeParser );
}



template <typename Iterator>
void AttributesParser<Iterator>::parsedAttribute( const std::string &parameter, Value &value )
{
    m_parent->attributeSet( parameter, value );
}



template <typename Iterator>
TopLevelParser<Iterator>::TopLevelParser( ParserImpl<Iterator> *parent):
    TopLevelParser<Iterator>::base_type( start ), m_parent( parent )
{
    using qi::_1;
    using qi::_2;
    using qi::_3;
    using qi::_4;
    using qi::_a;
    using qi::_b;
    using qi::eps;
    using qi::raw;
    using qi::eoi;
    using qi::on_error;
    using qi::fail;

    phoenix::function<RangeToString<Iterator> > rangeToString = RangeToString<Iterator>();

    start = ( eps( !_a ) > dispatch >> -eoi[ _a = true ] );

    dispatch = ( raw[ kinds[ _a = _1 ] ][ rangeToString( _1, _b ) ]
        > lazy( _a )[ phoenix::bind( &TopLevelParser::parsedKind, this, _b, _1 ) ] );

    phoenix::function<ObjectErrorHandler<Iterator> > objectErrorHandler = ObjectErrorHandler<Iterator>();
    phoenix::function<ValueErrorHandler<Iterator> > valueErrorHandler = ValueErrorHandler<Iterator>();
    on_error<fail>( start, objectErrorHandler( _1, _2, _3, _4, phoenix::ref( kinds ) ) );
    on_error<fail>( dispatch, valueErrorHandler( _1, _2, _3, _4 ) );
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
    m_parent->categoryEntered( kindName, objectName );
}



template <typename Iterator>
ParserImpl<Iterator>::ParserImpl( Parser *parent ): m_parser( parent ), leaveCategory( false )
{
    predefinedRules = new PredefinedRules<Iterator>();
    topLevelParser = new TopLevelParser<Iterator>( this );

    // Filling the AttributesParsers map
    std::vector<std::string> kinds = m_parser->m_dbApi->kindNames();

    for( std::vector<std::string>::iterator it = kinds.begin(); it != kinds.end(); ++it ) {
        topLevelParser->addKind( *it, predefinedRules->getObjectIdentifier() );

        attributesParsers[ *it ] = new AttributesParser<Iterator>( *it, this );
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
    std::cout << "Parse line: " << line << std::endl;

    // "end" detected
    if ( matchesEnd( line ) ) {
        categoryLeft();
        return;
    }

    Iterator iter = line.begin();
    Iterator end = line.end(); 
    
    bool parsingSucceeded;
    bool parsingTopLevel;

    if( contextStack.empty() ) {
        // No context, parse top-level objects
        std::cout << "Parsing top level object..." << std::endl;
        parsingSucceeded = phrase_parse( iter, end, *topLevelParser, ascii::space );
        parsingTopLevel = true;
    }
    else {
        // Context -> parse attributes
        std::cout << "Parsing attributes for \"" << contextStack.back().first << "\"..." << std::endl;
        parsingSucceeded = phrase_parse( iter, end, *( attributesParsers[ contextStack.back().first ] ), ascii::space );
        parsingTopLevel = false;
    }

    // Some bad input
    if ( !parsingSucceeded )
    {
        std::cout << "Parsing failed." << std::endl;
        return;
    }

    if( iter == end ) {
        std::cout << "Parsing succeeded. Full match." << std::endl;
        // Entering category permanently. Only top-level object or attributes definition on line
        if ( parsingTopLevel )
            leaveCategory = false;        
    }
    else {
        // Top-level object with attributes definition on line
        leaveCategory = true;
        std::cout << "Parsing succeeded. Partial match." << std::endl;
        std::cout << "Remaining: " << std::string( iter, end ) << std::endl;    
        
        parseLine( std::string( iter, end ) );
    }

    if( leaveCategory && !parsingTopLevel ) {
        categoryLeft();
        leaveCategory = false;
    }
}



template <typename Iterator>
bool ParserImpl<Iterator>::isNestedInContext() const
{
    return !contextStack.empty();
}



template <typename Iterator>
std::vector<AttributeDefinition> ParserImpl<Iterator>::currentContextStack() const
{
    return contextStack;
}



template <typename Iterator>
void ParserImpl<Iterator>::categoryEntered( const Identifier &kind, const Identifier &name )
{
    contextStack.push_back( std::make_pair<Identifier, Identifier>( kind, name ) );
    m_parser->categoryEntered( kind, name );
    // TODO: Delete this
    std::cout << "Parsed kind: " << kind << ": " << name << std::endl;
}



template <typename Iterator>
void ParserImpl<Iterator>::categoryLeft()
{
    contextStack.pop_back();
    m_parser->categoryLeft();
    // TODO: Delete this
    std::cout << "Category left" << std::endl;
}



template <typename Iterator>
void ParserImpl<Iterator>::attributeSet( const Identifier &name, const Value &value )
{
    m_parser->attributeSet( name, value );
    // TODO: Delete this
    std::cout << "Parsed parameter: " << name << "=" << value << std::endl;
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



template <typename Iterator>
bool ParserImpl<Iterator>::matchesEnd( const std::string &word )
{
    /* FIXME: Regex has problems with linker
    boost::regex re( "^end\\s*$}" );
    return boost::regex_match( word, re );
    */

    if( word.size() >= 3 ) {
        if( ( word[0] == 'e' ) && ( word[1] == 'n' ) && ( word[2] == 'd' ) )
            return true;
    }
    return false;
}


/////////////////////////Template instances for linker//////////////////////////

template void RangeToString<iterator_type>::operator()(
    const boost::iterator_range<iterator_type> &rng, std::string &str ) const;

template void ObjectErrorHandler<iterator_type>::operator()(
    iterator_type start,
    iterator_type end,
    iterator_type errorPos,
    const spirit::info &what,
    qi::symbols<char, qi::rule<iterator_type, std::string(), ascii::space_type> > kinds ) const;

template void ObjectErrorHandler<iterator_type>::printKindName(
    const std::string &name,
    const qi::rule<iterator_type, std::string(), ascii::space_type> &rule );

template void KeyErrorHandler<iterator_type>::operator()(
    iterator_type start,
    iterator_type end,
    iterator_type errorPos,
    const spirit::info &what,
    qi::symbols<char, qi::rule<iterator_type, Value(), ascii::space_type> > attributes ) const;

template void KeyErrorHandler<iterator_type>::printAttributeName(
    const std::string &name,
    const qi::rule<iterator_type, Value(), ascii::space_type> &rule );

template void ValueErrorHandler<iterator_type>::operator()(
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
    const std::string &kindName,
    ParserImpl<iterator_type> *parent );

template void AttributesParser<iterator_type>::addAtrribute(
    const std::string &attributeName,
    qi::rule<
        iterator_type,
        Value(),
        ascii::space_type> attributeParser );

template void AttributesParser<iterator_type>::parsedAttribute(
    const std::string &parameter,
    Value &value );

template TopLevelParser<iterator_type>::TopLevelParser( ParserImpl<iterator_type> *parent );

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

template void ParserImpl<iterator_type>::categoryEntered( const Identifier &kind, const Identifier &name );

template void ParserImpl<iterator_type>::categoryLeft();

template void ParserImpl<iterator_type>::attributeSet( const Identifier &name, const Value &value );

template void ParserImpl<iterator_type>::addKindAttributes(
    std::string &kindName,
    AttributesParser<iterator_type>* attributeParser );

template bool ParserImpl<iterator_type>::matchesEnd( const std::string &word );

}
}
