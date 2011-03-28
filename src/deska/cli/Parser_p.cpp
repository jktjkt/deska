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

#include <boost/assert.hpp>
#include "Parser_p.h"
#include "deska/db/Api.h"

#define PARSER_DEBUG
#define PARSER_PRINT_ERRORS

namespace Deska
{
namespace Cli
{


/** @short Convert boost::iterator_range<class> to std::string */
template <typename Iterator>
class RangeToString
{
public:
    template <typename, typename>
        struct result { typedef void type; };

    void operator()( const boost::iterator_range<Iterator> &range, std::string &str ) const
    {
        str.assign( range.begin(), range.end() );
    }
};



template <typename Iterator>
PredefinedRules<Iterator>::PredefinedRules()
{
    tQuotedString %= qi::lexeme[ '"' >> +( ascii::char_ - '"' ) >> '"' ];
    tIdentifier %= qi::lexeme[ +( ascii::alnum | '_' ) ];

    rulesMap[Db::TYPE_INT] = qi::int_
        [ qi::_val = phoenix::static_cast_<int>( qi::_1 ) ];
    rulesMap[Db::TYPE_INT].name( "integer" );

    // FIXME: consider allowing trivial words without quotes
    rulesMap[Db::TYPE_STRING] = tQuotedString
        [ qi::_val = phoenix::static_cast_<std::string>( qi::_1 ) ];
    rulesMap[Db::TYPE_STRING].name( "quoted string" );

    rulesMap[Db::TYPE_DOUBLE] = qi::double_
        [ qi::_val = phoenix::static_cast_<double>( qi::_1 ) ];
    rulesMap[Db::TYPE_DOUBLE].name( "double" );

    rulesMap[Db::TYPE_IDENTIFIER] = tIdentifier
        [ qi::_val = phoenix::static_cast_<std::string>( qi::_1 ) ];
    rulesMap[Db::TYPE_IDENTIFIER].name( "identifier (alphanumerical letters and _)" );

    objectIdentifier %= tIdentifier.alias();
    objectIdentifier.name( "object identifier (alphanumerical letters and _)" );
}



template <typename Iterator>
const qi::rule<Iterator, Db::Value(), ascii::space_type>& PredefinedRules<Iterator>::getRule( const Db::Type attrType )
{
    typename std::map<Db::Type, qi::rule<Iterator, Db::Value(), ascii::space_type> >::const_iterator it = rulesMap.find( attrType );
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
    AttributesParser<Iterator>::base_type( start ), m_name( kindName ), m_parent( parent )
{
    using qi::_1;
    using qi::_2;
    using qi::_3;
    using qi::_4;
    using qi::_a;
    using qi::eps;
    using qi::raw;
    using qi::eoi;
    using qi::on_error;
    using qi::fail;

    // If the boost::spirit::qi::grammar API was sane, the following line would read setName(kindName).
    // The API is not sane, and therefore we have the following crap here.
    this->name( kindName );

    phoenix::function<RangeToString<Iterator> > rangeToString = RangeToString<Iterator>();

    start = ( eps( !_a ) > dispatch >> -eoi[ _a = true ] );

    dispatch = ( ( raw[ attributes[ _a = _1 ] ][ rangeToString( _1, phoenix::ref( currentAttributeName ) ) ]
        > lazy( _a )[ phoenix::bind( &AttributesParser::parsedAttribute, this, phoenix::ref( currentAttributeName ), _1 ) ] ) );

    phoenix::function<KeyErrorHandler<Iterator> > keyErrorHandler = KeyErrorHandler<Iterator>();
    phoenix::function<ValueErrorHandler<Iterator> > valueErrorHandler = ValueErrorHandler<Iterator>();
    on_error<fail>( start, keyErrorHandler( _1, _2, _3, _4, phoenix::ref( attributes ),
        phoenix::ref( m_name ), m_parent ) );
    on_error<fail>( dispatch, valueErrorHandler( _1, _2, _3, _4, phoenix::ref( currentAttributeName ), m_parent ) );
}



template <typename Iterator>
void AttributesParser<Iterator>::addAtrribute(const std::string &attributeName, qi::rule<Iterator, Db::Value(), ascii::space_type> attributeParser )
{
    attributes.add( attributeName, attributeParser );
}



template <typename Iterator>
void AttributesParser<Iterator>::parsedAttribute( const std::string &parameter, Db::Value &value )
{
    m_parent->attributeSet( parameter, value );
}



template <typename Iterator>
KindsOnlyParser<Iterator>::KindsOnlyParser( const std::string &kindName, ParserImpl<Iterator> *parent):
    KindsOnlyParser<Iterator>::base_type( start ), m_name( kindName ), m_parent( parent )
{
    using qi::_1;
    using qi::_2;
    using qi::_3;
    using qi::_4;
    using qi::_a;
    using qi::eps;
    using qi::raw;
    using qi::eoi;
    using qi::on_error;
    using qi::fail;

    // If the boost::spirit::qi::grammar API was sane, the following line would read setName(kindName).
    // The API is not sane, and therefore we have the following crap here.
    this->name( kindName );

    phoenix::function<RangeToString<Iterator> > rangeToString = RangeToString<Iterator>();

    start = ( eps( !_a ) > dispatch >> -eoi[ _a = true ] );

    dispatch = ( raw[ kinds[ _a = _1 ] ][ rangeToString( _1, phoenix::ref( currentKindName ) ) ]
        > lazy( _a )[ phoenix::bind( &KindsOnlyParser::parsedKind, this, phoenix::ref( currentKindName ), _1 ) ] );

    phoenix::function<ObjectErrorHandler<Iterator> > objectErrorHandler = ObjectErrorHandler<Iterator>();
    phoenix::function<ValueErrorHandler<Iterator> > valueErrorHandler = ValueErrorHandler<Iterator>();
    on_error<fail>( start, objectErrorHandler( _1, _2, _3, _4, phoenix::ref( kinds ),
        phoenix::ref( m_name ), m_parent ) );
    on_error<fail>( dispatch, valueErrorHandler( _1, _2, _3, _4, phoenix::ref( currentKindName ), m_parent ) );
}



template <typename Iterator>
void KindsOnlyParser<Iterator>::addKind(const std::string &kindName, qi::rule<Iterator, std::string(), ascii::space_type> identifierParser )
{
    kinds.add( kindName, identifierParser );
}



template <typename Iterator>
void KindsOnlyParser<Iterator>::parsedKind( const std::string &kindName, const std::string &objectName )
{
    m_parent->categoryEntered( kindName, objectName );
}



template <typename Iterator>
WholeKindParser<Iterator>::WholeKindParser( const std::string &kindName, AttributesParser<Iterator> *attributesParser,
    KindsOnlyParser<Iterator> *nestedKinds, ParserImpl<Iterator> *parent ):
    WholeKindParser<Iterator>::base_type( start ), m_parent( parent )
{
    // If the boost::spirit::qi::grammar API was sane, the following line would read setName(kindName).
    // The API is not sane, and therefore we have the following crap here.
    this->name( kindName );

    start =( ( +( *attributesParser ) >> -( *nestedKinds ) )
        | ( ( *nestedKinds )[ phoenix::bind( &WholeKindParser::parsedSingleKind, this ) ] )
        | ( qi::lit("end")[ phoenix::bind( &WholeKindParser::parsedEnd, this ) ] ) );
}



template <typename Iterator>
void WholeKindParser<Iterator>::parsedEnd()
{
    m_parent->categoryLeft();
}



template <typename Iterator>
void WholeKindParser<Iterator>::parsedSingleKind()
{
    m_parent->parsedSingleKind();
}




template <typename Iterator>
ParserImpl<Iterator>::ParserImpl( Parser *parent ): m_parser( parent )
{
    predefinedRules = new PredefinedRules<Iterator>();
    topLevelParser = new KindsOnlyParser<Iterator>( std::string( "" ), this );

    // Filling the AttributesParsers map
    std::vector<std::string> kinds = m_parser->m_dbApi->kindNames();

    for( std::vector<std::string>::iterator it = kinds.begin(); it != kinds.end(); ++it ) {
        topLevelParser->addKind( *it, predefinedRules->getObjectIdentifier() );
#ifdef PARSER_DEBUG
        std::cout << "Adding top level kind: " << *it << std::endl;
#endif

        attributesParsers[ *it ] = new AttributesParser<Iterator>( *it, this );
        addKindAttributes( *it, attributesParsers[ *it ] );

        kindsOnlyParsers[ *it ] = new KindsOnlyParser<Iterator>( *it, this );
        addNestedKinds( *it, kindsOnlyParsers[ *it ] );

        wholeKindParsers[ *it ] = new WholeKindParser<Iterator>( *it, attributesParsers[ *it ], kindsOnlyParsers[ *it ], this );
    }
}



template <typename Iterator>
ParserImpl<Iterator>::~ParserImpl()
{
    for( typename std::map<std::string, AttributesParser<Iterator>* >::iterator it = attributesParsers.begin();
        it != attributesParsers.end(); ++it ) {
        delete it->second;
    }
    for( typename std::map<std::string, WholeKindParser<Iterator>* >::iterator it = wholeKindParsers.begin();
        it != wholeKindParsers.end(); ++it ) {
        delete it->second;
    }
    for( typename std::map<std::string, KindsOnlyParser<Iterator>* >::iterator it = kindsOnlyParsers.begin();
        it != kindsOnlyParsers.end(); ++it ) {
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

    parseErrors.clear();
    
    bool parsingSucceeded;
    int parsingIterations = 0;
    std::vector<ContextStackItem>::size_type previousContextStackSize = contextStack.size();

    while( iter != end ) {
        ++parsingIterations;
#ifdef PARSER_DEBUG
        std::cout << "Parsing: " << std::string( iter, end ) << std::endl;
#endif
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
            parsingSucceeded = phrase_parse( iter, end, *( wholeKindParsers[ contextStack.back().kind ] ), ascii::space );
        }

        // Some bad input
        if ( !parsingSucceeded ) {
#ifdef PARSER_DEBUG
            std::cout << "Parsing failed." << std::endl;
#endif
            // No more than three errors should occure. Three errors occure only when bad identifier of embedded object is set.
            BOOST_ASSERT( parseErrors.size() <= 3 );
            // There have to be some ParseError when parsing fails.
            BOOST_ASSERT( parseErrors.size() != 0 );

            bool argumentTypeError = false;

            for( typename std::vector<ParseError<Iterator> >::iterator
                it = parseErrors.begin();
                it != parseErrors.end();
                ++it ) {

                    if( it->getType() == PARSE_ERROR_TYPE_VALUE_TYPE ) {
                        argumentTypeError = true;
#ifdef PARSER_DEBUG
                        std::cout << it->toString() << std::endl;
#endif
                        throw InvalidAttributeDataTypeError( it->toString(), line, it->getErrorPosition() );
                        break;
                    }   
            }
                if( !argumentTypeError ) {
                    if( parseErrors.size() == 1 ) {
#ifdef PARSER_DEBUG
                        std::cout << parseErrors[0].toString() << std::endl;
#endif
                        if( parseErrors[0].getType() == PARSE_ERROR_TYPE_ATTRIBUTE )
                            throw UndefinedAttributeError( parseErrors[0].toString(), line, parseErrors[0].getErrorPosition() );
                        else if ( parseErrors[0].getType() == PARSE_ERROR_TYPE_KIND )
                            throw InvalidObjectKind( parseErrors[0].toString(), line, parseErrors[0].getErrorPosition() );
                        else
                            throw std::domain_error("ParseErrorType out of range");

                    }
                    else {
#ifdef PARSER_DEBUG
                        std::cout << parseErrors[0].toCombinedString(parseErrors[1]) << std::endl;
#endif
                        throw UndefinedAttributeError(
                            parseErrors[0].toCombinedString(parseErrors[1]), line, parseErrors[0].getErrorPosition() );
                    }
                }

            break;
        }

        parseErrors.clear();

    }

    if( ( parsingIterations == 1 ) && ( previousContextStackSize < contextStack.size() ) ) {
        // Definition of kind found stand-alone on one line -> nest permanently
    } else {
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
void ParserImpl<Iterator>::categoryEntered( const Db::Identifier &kind, const Db::Identifier &name )
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
void ParserImpl<Iterator>::attributeSet( const Db::Identifier &name, const Db::Value &value )
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
void ParserImpl<Iterator>::addParseError( const ParseError<Iterator> &error )
{
    parseErrors.push_back( error );
}



template <typename Iterator>
void ParserImpl<Iterator>::addKindAttributes(std::string &kindName, AttributesParser<Iterator>* attributesParser )
{
    std::vector<Db::KindAttributeDataType> attributes = m_parser->m_dbApi->kindAttributes( kindName );
    for( std::vector<Db::KindAttributeDataType>::iterator it = attributes.begin(); it != attributes.end(); ++it ) {
        attributesParser->addAtrribute( it->name, predefinedRules->getRule( it->type ) );
#ifdef PARSER_DEBUG
        std::cout << "Adding attribute " << it->name << " to " << kindName << std::endl;
#endif
    }
}



template <typename Iterator>
void ParserImpl<Iterator>::addNestedKinds(std::string &kindName, KindsOnlyParser<Iterator>* kindsOnlyParser)
{
    std::vector<Db::Identifier> kinds = m_parser->m_dbApi->kindNames();
    for( std::vector<Db::Identifier>::iterator it = kinds.begin(); it != kinds.end(); ++it ) {
        std::vector<Db::ObjectRelation> relations = m_parser->m_dbApi->kindRelations( *it );
        for( std::vector<Db::ObjectRelation>::iterator itr = relations.begin(); itr != relations.end(); ++itr ) {
            if( ( itr->kind == Db::RELATION_EMBED_INTO ) && ( itr->targetTableName == kindName ) ) {
                kindsOnlyParser->addKind( *it, predefinedRules->getObjectIdentifier() );
#ifdef PARSER_DEBUG
                std::cout << "Embedding kind " << *it << " to " << kindName << std::endl;
#endif
            }
        }
    }
}



/////////////////////////Template instances for linker//////////////////////////

template PredefinedRules<iterator_type>::PredefinedRules();

template const qi::rule<iterator_type, Db::Value(), ascii::space_type>& PredefinedRules<iterator_type>::getRule(const Db::Type attrType);

template const qi::rule<iterator_type, std::string(), ascii::space_type>& PredefinedRules<iterator_type>::getObjectIdentifier();

template AttributesParser<iterator_type>::AttributesParser( const std::string &kindName, ParserImpl<iterator_type> *parent );

template void AttributesParser<iterator_type>::addAtrribute( const std::string &attributeName,qi::rule<iterator_type, Db::Value(), ascii::space_type> attributeParser );

template void AttributesParser<iterator_type>::parsedAttribute(const std::string &parameter, Db::Value &value );

template KindsOnlyParser<iterator_type>::KindsOnlyParser( const std::string &kindName, ParserImpl<iterator_type> *parent );

template void KindsOnlyParser<iterator_type>::addKind( const std::string &kindName,qi::rule<iterator_type, std::string(), ascii::space_type> identifierParser );

template void KindsOnlyParser<iterator_type>::parsedKind(const std::string &kindName, const std::string &objectName );

template WholeKindParser<iterator_type>::WholeKindParser( const std::string &kindName, AttributesParser<iterator_type> *attributesParser, KindsOnlyParser<iterator_type> *nestedKinds, ParserImpl<iterator_type> *parent );

template void WholeKindParser<iterator_type>::parsedEnd();

template void WholeKindParser<iterator_type>::parsedSingleKind();

template ParserImpl<iterator_type>::ParserImpl(Parser *parent);

template ParserImpl<iterator_type>::~ParserImpl();

template void ParserImpl<iterator_type>::parseLine( const std::string &line );

template bool ParserImpl<iterator_type>::isNestedInContext() const;

template std::vector<ContextStackItem> ParserImpl<iterator_type>::currentContextStack() const;

template void ParserImpl<iterator_type>::clearContextStack();

template void ParserImpl<iterator_type>::categoryEntered( const Db::Identifier &kind, const Db::Identifier &name );

template void ParserImpl<iterator_type>::categoryLeft();

template void ParserImpl<iterator_type>::attributeSet( const Db::Identifier &name, const Db::Value &value );

template void ParserImpl<iterator_type>::addParseError( const ParseError<iterator_type> &error );

template void ParserImpl<iterator_type>::addKindAttributes( std::string &kindName, AttributesParser<iterator_type>* attributesParser );

template void ParserImpl<iterator_type>::addNestedKinds( std::string &kindName, KindsOnlyParser<iterator_type>* kindsOnlyParser );

template void ParserImpl<iterator_type>::parsedSingleKind();

template void RangeToString<iterator_type>::operator()( const boost::iterator_range<iterator_type> &rng, std::string &str ) const;

}
}
