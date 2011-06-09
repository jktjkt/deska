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

#include "deska/db/Objects.h"
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
    *   Calls appropriate method in main parser.
    *
    *   @param parameter Name of the attribute.
    *   @param value Parsed value of the attribute.
    *   @see Db::Value
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



/** @short Parser for set of attribute removals of specific top-level grammar.
*
*   This grammar parses only one pair <"no" attribute_name>.
*   For parsing set of thees pairs, use some boost::spirit operator like kleene star.
*/
template <typename Iterator>
class AttributeRemovalsParser: public qi::grammar<Iterator, ascii::space_type>
{

public:

    /** @short Constructor only initializes the grammar with empty symbols table.
    *
    *   @param kindName Name of top-level object type, to which the attributes belong.
    *   @param parent Pointer to main parser for calling its functions as semantic actions.
    */
    AttributeRemovalsParser(const Db::Identifier &kindName, ParserImpl<Iterator> *parent);

    /** @short Function used for filling of symbols table of the parser.
    *
    *   @param attributeName Name of the attribute.
    */
    void addAtrribute(const Db::Identifier &attributeName);

private:

    /** @short Function used as semantic action for each parsed attribute removal.
    *
    *   Calls appropriate method in main parser.
    *
    *   @param attribute Name of the attribute.
    */
    void parsedAttributeRemoval(const Db::Identifier &attribute);

    /** Attribute name - attribute value type pairs definitions for purposes of Nabialek trick. */
    qi::symbols<char, qi::rule<Iterator, ascii::space_type> > attributes;

    /** Rule for parsing "no" keyword. */
    qi::rule<Iterator, ascii::space_type> start;
    /** Rule for parsing attribute names. */
    qi::rule<Iterator, ascii::space_type, qi::locals<qi::rule<Iterator, ascii::space_type> > > dispatch;

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
    *   Calls appropriate method in main parser and updates context stack.
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
    qi::rule<Iterator, ascii::space_type,
             qi::locals<qi::rule<Iterator, Db::Identifier(), ascii::space_type> > > dispatch;

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
    *   @param attributesRemovalParser Grammar used for parsing of attributes removals of the kind.
    *   @param nestedKinds Grammar used for parsing nested kinds definitions of the kind.
    *   @param parent Pointer to main parser for calling its functions as semantic actions.
    */
    WholeKindParser(const Db::Identifier &kindName, AttributesParser<Iterator> *attributesParser,
                    AttributeRemovalsParser<Iterator> *attributeRemovalsParser,
                    KindsOnlyParser<Iterator> *nestedKinds, ParserImpl<Iterator> *parent);

private:

    /** @short Function used as semantic action for parsed end keyword. */
    void parsedEnd();

    /** @short Function used as semantic action when there is only single kind on the line
    *          and so parser should nest into this kind.
    *
    *   Calls appropriate method in main parser.
    */
    void parsedSingleKind();

    qi::rule<Iterator, ascii::space_type > start;

    /** Pointer to main parser for calling its functions as semantic actions. */
    ParserImpl<Iterator> *m_parent;
};



/** @short Parser for parsing function words, that can be typed at the beginning of any line.
*
*   Parser works as alternatives parser with all words inside and invokes appropriate actions.
*/
template <typename Iterator>
class FunctionWordsParser: public qi::grammar<Iterator, ascii::space_type>
{

public:

    /** @short Constructor initializes the grammar with all rules.
    *
    *   @param parent Pointer to main parser for calling its functions as semantic actions.
    */
    FunctionWordsParser(ParserImpl<Iterator> *parent);

private:

    /** @short Function used as semantic action for parsed "delete" function word. */
    void actionDelete();

    /** @short Function used as semantic action for parsed "show" function word. */
    void actionShow();

    /** @short Function used as semantic action for parsed "rename" function word. */
    void actionRename();

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

    /** @short Builds the whole parser.
    *
    *   Createl all sub grammars using pointer to API from parent.
    *
    *   @param parent Pointer to the main parser.
    */
    ParserImpl(Parser *parent);

    /** @short All created sub gramars are deleted there. */
    virtual ~ParserImpl();

    /** @short Main function of the parser. Parses the whole line and invokes appropriate signals.
    *
    *   Sets flag dryRun to false and invokes bool parseLineImpl(const std::string &line).
    *
    *   @param line Line to parse.
    *   @see bool parseLineImpl(const std::string &line)
    */
    void parseLine(const std::string &line);

    /** @short Function for checking whether the parser is nested or ready to parse top-level object.
    *
    *   @return True when the parser is nested, false if it is ready to parse top-level object.
    */
    bool isNestedInContext() const;

    /** @short Current context stack could be obtained by this function.
    *
    *   @return Current context stack.
    *   @see Db::ObjectDefinition
    */
    Db::ContextStack currentContextStack() const;

    /** @short Get list of strings for tab completion of current line.
    *
    *   Parses line using bool parseLineImpl(const std::string &line) with dryRun flag set to true.
    *   Vector of possible continuations construction is based on analysis of the last token and
    *   parse errors.
    *
    *   @param line Line for which the completions will be generated
    *   @return Vector of strings, that are possible continuations of current line.
    *   @see bool parseLineImpl(const std::string &line)
    */
    std::vector<std::string> tabCompletionPossibilities(const std::string &line);

    //@{
    /** @short Functions that only invokes signals of main parser. */
    void categoryEntered(const Db::Identifier &kind, const Db::Identifier &name);
    void categoryLeft();
    void attributeSet(const Db::Identifier &name, const Db::Value &value);
    void attributeRemove(const Db::Identifier &name);
    //@}

    /** @short Sets flag singleKind to true.
    *
    *   Function called when single kind without any attributes was parsed.
    *   Setting the flag means, that nesting will be permanent.
    */
    void parsedSingleKind();

    /** @short Function adding any error, that occures during parsing to parseErrors stack.
    *
    *   Errors added do not have to be actual errors. Some errors could be generated even though parsing succeeded,
    *   because of the way how boost::spirit works in case of alternatives grammar. When trying to parse some input,
    *   spirit tries to pass it to the first rule in the grammar and when the rule fails, it tries to pass the input
    *   to the next rule. The next rule can succeed, but an error handler of the first rule is already invoked. So we
    *   have to clear these errors, before call of void reportParseError(const std::string& line) function.
    *
    *   @param error Error, that occures during parsing.
    *   @see void reportParseError(const std::string& line)
    */
    void addParseError(const ParseError<Iterator> &error);

    /** @short Function for changing parsing mode of the parser.
    *
    *   @param mode Parsing mode
    */
    void setParsingMode(ParsingMode mode);

    /** @short Function for obtaining parsing mode of the parser.
    *
    *   @returns Parsing mode
    */
    ParsingMode getParsingMode();

    /** @short Function for obtaining list of all defined kind names.
    *   
    *   For purposes of bad nesting reporting.
    *
    *   @return Vector of kind names obtained from API
    */
    std::vector<Db::Identifier> getKindNames();

    /** @short Replaces current context stack with new one.
    *
    *   @param stack Vector of object definitions representing new context stack
    */
    void setContextStack(const Db::ContextStack &stack);
    
    /** @short Clears context stack.
    *
    *   For testing purposes.
    */
    void clearContextStack();

private:

    /** @short Fills symbols table of specific attribute parser with all attributes of given kind. */
    void addKindAttributes(const Db::Identifier &kindName, AttributesParser<Iterator>* attributesParser,
                           AttributeRemovalsParser<Iterator>* attributeRemovalsParser);
    /** @short Fills symbols table of specific kinds parser with all nested kinds of given kind. */
    void addNestedKinds(const Db::Identifier &kindName, KindsOnlyParser<Iterator>* kindsOnlyParser);

    /** @short Parses the whole line and invokes appropriate signals.
    *
    *   @param line Line to parse.
    *   @return True if the parsing succeeded
    */
    bool parseLineImpl(const std::string &line);

    /** @short Function reporting parse errors to main parser.
    *   
    *   Invokes appropriate signal.
    */
    void reportParseError(const std::string& line);

    /** @short Function for finding tab completion possibilities of current context when parsing succeeded.
    *
    *   Extracts nested kind names and attribute names.
    *
    *   @param line Line for which the completions will be generated.
    *   @param possibilities Pushes all possibilities in this vector.
    */
    void insertTabPossibilitiesOfCurrentContext(const std::string &line, std::vector<std::string> &possibilities);

    /** @short Function for finding tab completion possibilities of current context when parsing failed.
    *
    *   Extracts nested kind names and attribute names and object names.
    *
    *   @param line Line for which the completions will be generated.
    *   @param possibilities Pushes all possibilities in this vector.
    */
    void insertTabPossibilitiesFromErrors(const std::string &line, std::vector<std::string> &possibilities);

    //@{
    /** All rules and grammars, that is the whole parser build of are stored there.
    *   Only pointers are used in the main parser.
    */
    std::map<std::string, AttributesParser<Iterator>* > attributesParsers;
    std::map<std::string, AttributeRemovalsParser<Iterator>* > attributeRemovalsParsers;
    std::map<std::string, KindsOnlyParser<Iterator>* > kindsOnlyParsers;
    std::map<std::string, WholeKindParser<Iterator>* > wholeKindParsers;
    KindsOnlyParser<Iterator> *topLevelParser;
    FunctionWordsParser<Iterator> *functionWordsParser;
    PredefinedRules<Iterator> *predefinedRules;
    //@}

    /** The parser context is held there. */
    Db::ContextStack contextStack;
    /** Stack with errors, that occures during parsing. @see void addParseError(const ParseError<Iterator> &error) */
    std::vector<ParseError<Iterator> > parseErrors;
    
    /** Pointer to main parser for invoking signals. */
    Parser *m_parser;
    /** Flag, that disables or enables invoking signals. TRUE -> ENABLE, FALSE -> DISABLE */
    bool dryRun;
    /** True when single kind without any attributes was parsed. Means, that nesting will be permanent. */
    bool singleKind;
    /** Current parsing mode. @see ParsingMode */
    ParsingMode parsingMode;
};

}

}



#endif  // DESKA_PARSERPRIVATE_H
