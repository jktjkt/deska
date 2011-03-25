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

#ifndef DESKA_PARSERPRIVATE_H
#define DESKA_PARSERPRIVATE_H

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>

#include "Parser.h"
#include "ParserErrors.h"

namespace Deska
{
namespace CLI
{

namespace spirit = boost::spirit;
namespace phoenix = boost::phoenix;
namespace ascii = boost::spirit::ascii;
namespace qi = boost::spirit::qi;



/** @short Predefined rules for parsing single parameters */
template <typename Iterator>
class PredefinedRules
{

public:

    /** @short Fills internal map with predefined rules, that can be used to parse attributes of top-level objects */
    PredefinedRules();

    /** @short Function for getting single rules, that can be used in attributes grammar
    *
    *   @param attrType Type of the attribute in question, @see Type
    *   @return Rule that parses specific type of attribute
    */
    const qi::rule<Iterator, Db::Value(), ascii::space_type>& getRule( const Db::Type attrType );

    /** @short Function for getting rule used to parse identifier of top-level objects */
    const qi::rule<Iterator, std::string(), ascii::space_type>& getObjectIdentifier();

private:

    std::map<Db::Type, qi::rule<Iterator, Db::Value(), ascii::space_type> > rulesMap;
    qi::rule<Iterator, std::string(), ascii::space_type> objectIdentifier;

};



/** @short Parser for set of attributes of specific top-level grammar */
template <typename Iterator>
class AttributesParser: public qi::grammar<Iterator, ascii::space_type, qi::locals<bool> >
{

public:

    /** @short Constructor only initializes the grammar with empty symbols table
    *
    *   @param kindName Name of top-level object type, to which the attributes belong
    */
    AttributesParser( const std::string &kindName, ParserImpl<Iterator> *parent );

    /** @short Function used for filling of symbols table of the parser
    *
    *   @param attributeName Name of the attribute
    *   @param attributeParser  Attribute parser obtained from PredefinedRules class
    *   @see PredefinedRules
    */
    void addAtrribute( const std::string &attributeName, qi::rule<Iterator, Db::Value(), ascii::space_type> attributeParser );

private:

    /** @short Function used as semantic action for each parsed attribute
    *
    *   @param parameter Name of the attribute
    *   @param value Parsed value of the attribute
    */
    void parsedAttribute( const std::string &parameter, Db::Value &value );


    qi::symbols<char, qi::rule<Iterator, Db::Value(), ascii::space_type> > attributes;

    qi::rule<Iterator, ascii::space_type, qi::locals<bool> > start;

    qi::rule<Iterator, ascii::space_type, qi::locals<qi::rule<Iterator, Db::Value(), ascii::space_type>, std::string> > dispatch;

    ParserImpl<Iterator> *m_parent;
};



/** @short Parser for kinds definitions */
template <typename Iterator>
class KindsParser: public qi::grammar<Iterator, ascii::space_type, qi::locals<bool> >
{

public:

    /** @short Constructor only initializes the grammar with empty symbols table */
    KindsParser( const std::string &kindName, ParserImpl<Iterator> *parent );

    /** @short Function used for filling of symbols table of the parser
    *
    *   @param kindName Name of the kind
    */
    void addKind( const std::string &kindName, qi::rule<Iterator, std::string(), ascii::space_type> identifierParser );

private:

    /** @short Function used as semantic action for parsed kind
    *
    *   @param kindName Name of the kind
    *   @param objectName Parsed name of the object
    */
    void parsedKind( const std::string &kindName, const std::string &objectName );

    qi::symbols<char, qi::rule<Iterator, std::string(), ascii::space_type> > kinds;

    qi::rule<Iterator, ascii::space_type, qi::locals<bool> > start;

    qi::rule<Iterator, ascii::space_type, qi::locals<qi::rule<Iterator, std::string(), ascii::space_type>, std::string> > dispatch;

    ParserImpl<Iterator> *m_parent;
};



/** @short Parser for set of attributes and nested objects of specific top-level grammar */
template <typename Iterator>
class KindParser: public qi::grammar<Iterator, ascii::space_type>
{

public:

    /** @short Constructor initializes the grammar with all rules */
    KindParser( const std::string &kindName, AttributesParser<Iterator> *attributesParser,
        KindsParser<Iterator> *nestedKinds, ParserImpl<Iterator> *parent );

private:

    /** @short Function used as semantic action for parsed end keyword */
    void parsedEnd();

    /** @short Function used as semantic action when there is only single kind on the line and so parser
    *          should nest into this kind
    */
    void parsedSingleKind();

    qi::rule<Iterator, ascii::space_type > start;

    ParserImpl<Iterator> *m_parent;
};



template <typename Iterator>
class ParserImpl: boost::noncopyable
{
public:
    ParserImpl( Parser *parent );
    virtual ~ParserImpl();

    void parseLine( const std::string &line );
    bool isNestedInContext() const;
    std::vector<ContextStackItem> currentContextStack() const;
    void clearContextStack();

    void categoryEntered( const Db::Identifier &kind, const Db::Identifier &name );
    void categoryLeft();
    void attributeSet( const Db::Identifier &name, const Db::Value &value );

    void parsedSingleKind();

    void addParseError( const ParseError<Iterator> &error );

private:
    Parser *m_parser;

    /** @short Fills symbols table of specific attribute parser with all attributes of given kind */
    void addKindAttributes( std::string &kindName, AttributesParser<Iterator>* attributesParser );

    /** @short Fills symbols table of specific kinds parser with all nested kinds of given kind */
    void addNestedKinds( std::string &kindName, KindsParser<Iterator>* kindsParser );

    std::map<std::string, AttributesParser<Iterator>* > attributesParsers;
    std::map<std::string, KindsParser<Iterator>* > kindsParsers;
    std::map<std::string, KindParser<Iterator>* > kindParsers;
    KindsParser<Iterator> *topLevelParser;
    PredefinedRules<Iterator> *predefinedRules;

    std::vector<ContextStackItem> contextStack;

    std::vector<ParseError<Iterator> > parseErrors;
};

}

}



#endif  // DESKA_PARSERPRIVATE_H
