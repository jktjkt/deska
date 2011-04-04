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
#include "Exceptions.h"

namespace Deska
{
namespace Cli
{

namespace spirit = boost::spirit;
namespace phoenix = boost::phoenix;
namespace ascii = boost::spirit::ascii;
namespace qi = boost::spirit::qi;



/** @short Predefined rules for parsing single attribute values and identifiers.
*   
*   Any new type have to be defined here.
*   String types have to be defined using one extra rule in this way:
*       qi::rule<Iterator, std::string(), ascii::space_type> tStringValue %= ....
*       rulesMap[Db::TYPE_NAME] = tStringValue[qi::_val = phoenix::static_cast_<std::string>(qi::_1)];
*   This is because of proper initialization of boost::variant and because without the extra rule boost will
*   push every character in the variant separately, that is very unefficient.
*/
template <typename Iterator>
class PredefinedRules
{

public:

    /** @short Fills internal map with predefined rules. */
    PredefinedRules();

    /** @short Function for getting single rules, that can be used in attributes grammar.
    *
    *   @param attrType Type of the attribute in question, @see Type.
    *   @return Rule that parses specific type of attribute.
    */
    const qi::rule<Iterator, Db::Value(), ascii::space_type>& getRule(const Db::Type attrType);

    /** @short Function for getting rule used to parse identifier of top-level objects. */
    const qi::rule<Iterator, Db::Identifier(), ascii::space_type>& getObjectIdentifier();

private:

    //@{
    /** Extra rules used for definition of string types. */
    qi::rule<Iterator, std::string(), ascii::space_type> tQuotedString;
    qi::rule<Iterator, std::string(), ascii::space_type> tIdentifier;
    //@}

    /** Map where all rules are stored, @see Type. */
    std::map<Db::Type, qi::rule<Iterator, Db::Value(), ascii::space_type> > rulesMap;
    /** Rule for parsing identifiers of top-level objects. */
    qi::rule<Iterator, Db::Identifier(), ascii::space_type> objectIdentifier;

};



/** @short Parser for set of attributes of specific top-level grammar.
*
*   This grammar parses only one pair from set of <attribute_name attribute_value> definitions.
*   For parsing set of thees pairs, use some boost::spirit operator like kleene star.
*/
template <typename Iterator>
class AttributesParser: public qi::grammar<Iterator, ascii::space_type, qi::locals<bool> >
{

public:

    /** @short Constructor only initializes the grammar with empty symbols table.
    *
    *   @param kindName Name of top-level object type, to which the attributes belong.
    *   @param parent Pointer to main parser for calling its functions as semantic actions.
    */
    AttributesParser(const Db::Identifier &kindName, ParserImpl<Iterator> *parent);

    /** @short Function used for filling of symbols table of the parser.
    *
    *   @param attributeName Name of the attribute.
    *   @param attributeParser Attribute parser obtained from PredefinedRules class.
    *   @see PredefinedRules
    */
    void addAtrribute(const Db::Identifier &attributeName,
                      qi::rule<Iterator, Db::Value(), ascii::space_type> attributeParser);

private:

    /** @short Function used as semantic action for each parsed attribute.
    *
    *   @param parameter Name of the attribute.
    *   @param value Parsed value of the attribute.
    *   @see Value
    */
    void parsedAttribute(const Db::Identifier &parameter, Db::Value &value);

    /** Attribute name - attribute value type pairs definitions for purposes of Nabialek trick. */
    qi::symbols<char, qi::rule<Iterator, Db::Value(), ascii::space_type> > attributes;

    /** Rule for parsing attribute names. */
    qi::rule<Iterator, ascii::space_type, qi::locals<bool> > start;
    /** Rule for parsing attribute values. */
    qi::rule<Iterator, ascii::space_type, qi::locals<qi::rule<Iterator, Db::Value(), ascii::space_type> > > dispatch;

    /** Name of attribute which value is being currently parsed. This variable is used for error handling. */
    Db::Identifier currentAttributeName;
    /** Name of the top-level object, whose attributes are parsed by this grammar.
    *   This variable is used for error handling.
    */
    Db::Identifier m_name;

    /** Pointer to main parser for calling its functions as semantic actions. */
    ParserImpl<Iterator> *m_parent;
};



/** @short Parser for kinds definitions.
*
*   This grammar parses only one pair from set of <kind_name object_name> definitions.
*/
template <typename Iterator>
class KindsOnlyParser: public qi::grammar<Iterator, ascii::space_type, qi::locals<bool> >
{

public:

    /** @short Constructor only initializes the grammar with empty symbols table.
    *
    *   @param kindName Name of top-level object type, to which the parser belongs in case of parser for nested kinds.
    *   @param parent Pointer to main parser for calling its functions as semantic actions.
    */
    KindsOnlyParser(const Db::Identifier &kindName, ParserImpl<Iterator> *parent);

    /** @short Function used for filling of symbols table of the parser.
    *
    *   @param kindName Name of the kind.
    *   @param identifierParser Identifier parser obtained from PredefinedRules class.
    */
    void addKind(const Db::Identifier &kindName,
                 qi::rule<Iterator, Db::Identifier(), ascii::space_type> identifierParser);

private:

    /** @short Function used as semantic action for parsed kind.
    *
    *   @param kindName Name of the kind.
    *   @param objectName Parsed name of the object.
    */
    void parsedKind(const Db::Identifier &kindName, const Db::Identifier &objectName);

    /** Kind name - identifier type pairs definitions for purposes of Nabialek trick. */
    qi::symbols<char, qi::rule<Iterator, Db::Identifier(), ascii::space_type> > kinds;

    /** Rule for parsing kind names. */
    qi::rule<Iterator, ascii::space_type, qi::locals<bool> > start;
    /** Rule for parsing object names. */
    qi::rule<Iterator, ascii::space_type, qi::locals<qi::rule<Iterator, Db::Identifier(), ascii::space_type> > > dispatch;

    /** Name of kind which identifier is being currently parsed. This variable is used for error handling. */
    Db::Identifier currentKindName;
    /** Name of the top-level object, whose nested kind definitions are parsed by this grammar.
    *   This variable is used for error handling.
    */
    Db::Identifier m_name;

    /** Pointer to main parser for calling its functions as semantic actions. */
    ParserImpl<Iterator> *m_parent;
};



/** @short Parser for set of attributes and nested objects of specific top-level grammar.
*
*   Combines all needed grammars into one parser for parsing the whole kind with its all attributes and nested kinds.
*/
template <typename Iterator>
class WholeKindParser: public qi::grammar<Iterator, ascii::space_type>
{

public:

    /** @short Constructor initializes the grammar with all rules.
    *
    *   @param kindName Name of top-level object type, to which the parser belongs.
    *   @param attributesParser Grammar used for parsing of attributes of the kind.
    *   @param nestedKinds Grammar used for parsing nested kinds definitions of the kind.
    *   @param parent Pointer to main parser for calling its functions as semantic actions.
    */
    WholeKindParser(const Db::Identifier &kindName, AttributesParser<Iterator> *attributesParser,
        KindsOnlyParser<Iterator> *nestedKinds, ParserImpl<Iterator> *parent);

private:

    /** @short Function used as semantic action for parsed end keyword. */
    void parsedEnd();

    /** @short Function used as semantic action when there is only single kind on the line
               and so parser should nest into this kind.
    */
    void parsedSingleKind();

    qi::rule<Iterator, ascii::space_type > start;

    /** Pointer to main parser for calling its functions as semantic actions. */
    ParserImpl<Iterator> *m_parent;
};



/** @short The main class containing all grammars, holding context and calling the grammars on the input.
*
*   
*/
template <typename Iterator>
class ParserImpl: boost::noncopyable
{

public:

    ParserImpl(Parser *parent);

    virtual ~ParserImpl();

    void parseLine(const std::string &line);

    bool isNestedInContext() const;

    std::vector<ContextStackItem> currentContextStack() const;

    void clearContextStack();

    void categoryEntered(const Db::Identifier &kind, const Db::Identifier &name);
    void categoryLeft();
    void attributeSet(const Db::Identifier &name, const Db::Value &value);

    void parsedSingleKind();

    void addParseError(const ParseError<Iterator> &error);
    
    std::vector<std::string> tabCompletitionPossibilities(const std::string &line);

private:

    Parser *m_parser;

    /** @short Fills symbols table of specific attribute parser with all attributes of given kind. */
    void addKindAttributes(const Db::Identifier &kindName, AttributesParser<Iterator>* attributesParser);

    /** @short Fills symbols table of specific kinds parser with all nested kinds of given kind. */
    void addNestedKinds(const Db::Identifier &kindName, KindsOnlyParser<Iterator>* kindsOnlyParser);

    bool parseLineImpl(const std::string &line);
    void reportParseError(const std::string& line);

    std::map<std::string, AttributesParser<Iterator>* > attributesParsers;
    std::map<std::string, KindsOnlyParser<Iterator>* > kindsOnlyParsers;
    std::map<std::string, WholeKindParser<Iterator>* > wholeKindParsers;
    KindsOnlyParser<Iterator> *topLevelParser;
    PredefinedRules<Iterator> *predefinedRules;

    std::vector<ContextStackItem> contextStack;

    std::vector<ParseError<Iterator> > parseErrors;

    bool dryRun;
};

}

}



#endif  // DESKA_PARSERPRIVATE_H
