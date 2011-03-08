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

//#define PARSER_DEBUG

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
    qi::symbols<char, qi::rule<Iterator, std::string(), ascii::space_type> > kinds ) const
{
    std::cout << "Error in object type parsing! Expecting ";
    kinds.for_each( printKindName );
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
void KeyErrorHandler<Iterator>::operator()(Iterator start, Iterator end, Iterator errorPos, const spirit::info& what,
    qi::symbols<char, qi::rule<Iterator, Value(), ascii::space_type> > attributes ) const
{
    std::cout << "Error in attribute name parsing! Expecting ";
    attributes.for_each( printAttributeName );
    std::cout << "here: \"" << std::string( errorPos, end ) << "\"" << std::endl;
}



template <typename Iterator>
void KeyErrorHandler<Iterator>::printAttributeName( const std::string &name, const qi::rule<Iterator, Value(), ascii::space_type> &rule )
{
    std::cout << name << " ";
}



template <typename Iterator>
void ValueErrorHandler<Iterator>::operator()(Iterator start, Iterator end, Iterator errorPos, const spirit::info& what ) const
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
        throw std::domain_error( "PredefinedRules::getRule: Internal error: type of the attribute not registered when looking up grammar rule" );
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
void AttributesParser<Iterator>::addAtrribute(const std::string &attributeName, qi::rule<Iterator, Value(), ascii::space_type> attributeParser )
{
    attributes.add( attributeName, attributeParser );
}



template <typename Iterator>
void AttributesParser<Iterator>::parsedAttribute( const std::string &parameter, Value &value )
{
    m_parent->attributeSet( parameter, value );
}



template <typename Iterator>
KindsParser<Iterator>::KindsParser( const std::string &kindName, ParserImpl<Iterator> *parent):
    KindsParser<Iterator>::base_type( start ), m_parent( parent )
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

    start = ( eps( !_a ) > dispatch >> -eoi[ _a = true ] );

    dispatch = ( raw[ kinds[ _a = _1 ] ][ rangeToString( _1, _b ) ]
        > lazy( _a )[ phoenix::bind( &KindsParser::parsedKind, this, _b, _1 ) ] );

    phoenix::function<ObjectErrorHandler<Iterator> > objectErrorHandler = ObjectErrorHandler<Iterator>();
    phoenix::function<ValueErrorHandler<Iterator> > valueErrorHandler = ValueErrorHandler<Iterator>();
    on_error<fail>( start, objectErrorHandler( _1, _2, _3, _4, phoenix::ref( kinds ) ) );
    on_error<fail>( dispatch, valueErrorHandler( _1, _2, _3, _4 ) );
}



template <typename Iterator>
void KindsParser<Iterator>::addKind(const std::string &kindName, qi::rule<Iterator, std::string(), ascii::space_type> identifierParser )
{
    kinds.add( kindName, identifierParser );
}



template <typename Iterator>
void KindsParser<Iterator>::parsedKind( const std::string &kindName, const std::string &objectName )
{
    m_parent->categoryEntered( kindName, objectName );
}



template <typename Iterator>
KindParser<Iterator>::KindParser( const std::string &kindName, AttributesParser<Iterator> *attributesParser,
    KindsParser<Iterator> *nestedKinds, ParserImpl<Iterator> *parent ):
    KindParser<Iterator>::base_type( start ), m_parent( parent )
{

    this->name( kindName );

    start =( *nestedKinds )[ phoenix::bind( &KindParser::parsedSingleKind, this ) ]
        | ( ( *attributesParser ) >> -( *nestedKinds ) )
        | qi::lit("end")[ phoenix::bind( &KindParser::parsedEnd, this ) ];
}



template <typename Iterator>
void KindParser<Iterator>::parsedEnd()
{
    m_parent->categoryLeft();
}



template <typename Iterator>
void KindParser<Iterator>::parsedSingleKind()
{
    m_parent->parsedSingleKind();
}




template <typename Iterator>
ParserImpl<Iterator>::ParserImpl( Parser *parent ): m_parser( parent )
{
    predefinedRules = new PredefinedRules<Iterator>();
    topLevelParser = new KindsParser<Iterator>( std::string( "" ), this );

    // Filling the AttributesParsers map
    std::vector<std::string> kinds = m_parser->m_dbApi->kindNames();

    for( std::vector<std::string>::iterator it = kinds.begin(); it != kinds.end(); ++it ) {
        topLevelParser->addKind( *it, predefinedRules->getObjectIdentifier() );
#ifdef PARSER_DEBUG
        std::cout << "Adding top level kind: " << *it << std::endl;
#endif

        attributesParsers[ *it ] = new AttributesParser<Iterator>( *it, this );
        addKindAttributes( *it, attributesParsers[ *it ] );

        kindsParsers[ *it ] = new KindsParser<Iterator>( *it, this );
        addNestedKinds( *it, kindsParsers[ *it ] );

        // FIXME: this is either a logic error, or a memory leak. We're overwriting the parser created above
        // with a new instance, see Redmine #120.
        kindParsers[ *it ] = new KindParser<Iterator>( *it, attributesParsers[ *it ], kindsParsers[ *it ], this );
    }
}



template <typename Iterator>
ParserImpl<Iterator>::~ParserImpl()
{
    for( typename std::map<std::string, AttributesParser<Iterator>* >::iterator it = attributesParsers.begin();
        it != attributesParsers.end(); ++it ) {
        delete it->second;
    }

    delete topLevelParser;
    delete predefinedRules;
}



template <typename Iterator>
void ParserImpl<Iterator>::parseLine( const std::string &line )
{
    // TODO: Only testing implementation. Reimplement.
#ifdef PARSER_DEBUG
    std::cout << "Parse line: " << line << std::endl;
#endif


    Iterator iter = line.begin();
    Iterator end = line.end(); 
    
    bool parsingSucceeded;
    int parsingIterations = 0;
    std::vector<ContextStackItem>::size_type previousContextStackSize = contextStack.size();

    while( iter != end ) {
        ++parsingIterations;
        if ( contextStack.empty() ) {
            // No context, parse top-level objects
            #ifdef PARSER_DEBUG
            std::cout << "Parsing top level object..." << std::endl;
            #endif
            parsingSucceeded = phrase_parse( iter, end, *topLevelParser, ascii::space );
        } else {
            // Context -> parse attributes
            #ifdef PARSER_DEBUG
            std::cout << "Parsing attributes for \"" << contextStack.back().kind << "\"..." << std::endl;
            #endif
            parsingSucceeded = phrase_parse( iter, end, *( kindParsers[ contextStack.back().kind ] ), ascii::space );
        }

        // Some bad input
        if ( !parsingSucceeded ) {
            #ifdef PARSER_DEBUG
            std::cout << "Parsing failed." << std::endl;
            #endif
            break;
        }

    }

    if( ( parsingIterations == 1 ) && ( previousContextStackSize < contextStack.size() ) ) {
        // Definition of kind found stand-alone on one line -> nest permanently
    }
    else {
        int depthDiff = contextStack.size() - previousContextStackSize;
        if ( depthDiff > 0 )
            for( int i = 0; i < depthDiff; ++i ) {
                categoryLeft();
            }
    }
}



template <typename Iterator>
bool ParserImpl<Iterator>::isNestedInContext() const
{
    return !contextStack.empty();
}



template <typename Iterator>
std::vector<ContextStackItem> ParserImpl<Iterator>::currentContextStack() const
{
    return contextStack;
}



template <typename Iterator>
void ParserImpl<Iterator>::clearContextStack()
{
    contextStack.clear();
}



template <typename Iterator>
void ParserImpl<Iterator>::categoryEntered( const Identifier &kind, const Identifier &name )
{
    contextStack.push_back( ContextStackItem( kind, name ) );
    m_parser->categoryEntered( kind, name );
#ifdef PARSER_DEBUG
    std::cout << "Parsed kind: " << kind << ": " << name << std::endl;
#endif
}



template <typename Iterator>
void ParserImpl<Iterator>::categoryLeft()
{
    contextStack.pop_back();
    m_parser->categoryLeft();
#ifdef PARSER_DEBUG
    std::cout << "Category left" << std::endl;
#endif
}



template <typename Iterator>
void ParserImpl<Iterator>::attributeSet( const Identifier &name, const Value &value )
{
    m_parser->attributeSet( name, value );
#ifdef PARSER_DEBUG
    std::cout << "Parsed parameter: " << name << "=" << value << std::endl;
#endif
}



template <typename Iterator>
void ParserImpl<Iterator>::parsedSingleKind()
{
#ifdef PARSER_DEBUG
    std::cout << "Parsed single kind" << std::endl;
#endif
}



template <typename Iterator>
void ParserImpl<Iterator>::addKindAttributes(std::string &kindName, AttributesParser<Iterator>* attributesParser )
{
    std::vector<KindAttributeDataType> attributes = m_parser->m_dbApi->kindAttributes( kindName );
    for( std::vector<KindAttributeDataType>::iterator it = attributes.begin(); it != attributes.end(); ++it ) {
        attributesParser->addAtrribute( it->name, predefinedRules->getRule( it->type ) );
#ifdef PARSER_DEBUG
    std::cout << "Adding attribute " << it->name << " to " << kindName << std::endl;
#endif
    }
}



template <typename Iterator>
void ParserImpl<Iterator>::addNestedKinds(std::string &kindName, KindsParser<Iterator>* kindsParser)
{
    std::vector<Identifier> kinds = m_parser->m_dbApi->kindNames();
    for( std::vector<Identifier>::iterator it = kinds.begin(); it != kinds.end(); ++it ) {
        std::vector<ObjectRelation> relations = m_parser->m_dbApi->kindRelations( *it );
        for( std::vector<ObjectRelation>::iterator itr = relations.begin(); itr != relations.end(); ++itr ) {
            if( ( itr->kind == RELATION_EMBED_INTO ) && ( itr->targetTableName == kindName ) ) {
                kindsParser->addKind( *it, predefinedRules->getObjectIdentifier() );
#ifdef PARSER_DEBUG
                std::cout << "Embedding kind " << *it << " to " << kindName << std::endl;
#endif
            }
        }
    }
}



/////////////////////////Template instances for linker//////////////////////////

template void RangeToString<iterator_type>::operator()(
    const boost::iterator_range<iterator_type> &rng, std::string &str ) const;

template void ObjectErrorHandler<iterator_type>::operator()(iterator_type start, iterator_type end, iterator_type errorPos,
    const spirit::info &what, qi::symbols<char, qi::rule<iterator_type, std::string(), ascii::space_type> > kinds ) const;

template void ObjectErrorHandler<iterator_type>::printKindName( const std::string &name,
    const qi::rule<iterator_type, std::string(), ascii::space_type> &rule );

template void KeyErrorHandler<iterator_type>::operator()(iterator_type start, iterator_type end, iterator_type errorPos, const spirit::info &what,
    qi::symbols<char, qi::rule<iterator_type, Value(), ascii::space_type> > attributes ) const;

template void KeyErrorHandler<iterator_type>::printAttributeName( const std::string &name,
    const qi::rule<iterator_type, Value(), ascii::space_type> &rule );

template void ValueErrorHandler<iterator_type>::operator()( iterator_type start, iterator_type end, iterator_type errorPos, const spirit::info &what ) const;

template PredefinedRules<iterator_type>::PredefinedRules();

template const qi::rule<iterator_type, Value(), ascii::space_type>& PredefinedRules<iterator_type>::getRule(const Type attrType);

template const qi::rule<iterator_type, std::string(), ascii::space_type>& PredefinedRules<iterator_type>::getObjectIdentifier();

template AttributesParser<iterator_type>::AttributesParser( const std::string &kindName, ParserImpl<iterator_type> *parent );

template void AttributesParser<iterator_type>::addAtrribute( const std::string &attributeName,qi::rule<iterator_type, Value(), ascii::space_type> attributeParser );

template void AttributesParser<iterator_type>::parsedAttribute(const std::string &parameter, Value &value );

template KindsParser<iterator_type>::KindsParser( const std::string &kindName, ParserImpl<iterator_type> *parent );

template void KindsParser<iterator_type>::addKind( const std::string &kindName,qi::rule<iterator_type, std::string(), ascii::space_type> identifierParser );

template void KindsParser<iterator_type>::parsedKind(const std::string &kindName, const std::string &objectName );

template KindParser<iterator_type>::KindParser( const std::string &kindName, AttributesParser<iterator_type> *attributesParser, KindsParser<iterator_type> *nestedKinds, ParserImpl<iterator_type> *parent );

template void KindParser<iterator_type>::parsedEnd();

template void KindParser<iterator_type>::parsedSingleKind();

template ParserImpl<iterator_type>::ParserImpl(Parser *parent);

template ParserImpl<iterator_type>::~ParserImpl();

template void ParserImpl<iterator_type>::parseLine( const std::string &line );

template bool ParserImpl<iterator_type>::isNestedInContext() const;

template std::vector<ContextStackItem> ParserImpl<iterator_type>::currentContextStack() const;

template void ParserImpl<iterator_type>::clearContextStack();

template void ParserImpl<iterator_type>::categoryEntered( const Identifier &kind, const Identifier &name );

template void ParserImpl<iterator_type>::categoryLeft();

template void ParserImpl<iterator_type>::attributeSet( const Identifier &name, const Value &value );

template void ParserImpl<iterator_type>::addKindAttributes( std::string &kindName, AttributesParser<iterator_type>* attributesParser );

template void ParserImpl<iterator_type>::addNestedKinds( std::string &kindName, KindsParser<iterator_type>* kindsParser );

template void ParserImpl<iterator_type>::parsedSingleKind();

}
}
