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
#include "RangeToString.h"
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


template <typename Iterator> class AttributeRemovalsParser;
template <typename Iterator> class AttributesSettingParser;
template <typename Iterator> class IdentifiersSetsParser;
template <typename Iterator> class AttributesParser;
template <typename Iterator> class FunctionWordsParser;
template <typename Iterator> class KindsOnlyParser;
template <typename Iterator> class KindsConstructParser;
template <typename Iterator> class PredefinedRules;
template <typename Iterator> class FilterExpressionsParser;
template <typename Iterator> class FiltersParser;
template <typename Iterator> class KindsParser;
template <typename Iterator> class KindsFiltersParser;
template <typename Iterator> class WholeKindParser;
template <typename Iterator> class TopLevelParser;



/** @short The main class containing all grammars, holding context and calling the grammars on the input.
*
*   The context is a vector of pairs kind_name-object_name and could be imagined like some kind of path. The context
*   determines which attributes could be set, to which nested kinds could we step, etc.
*
*   The whole parser operates in one loop. At the beginning, before the loop, it tries to parse function words using
*   functionWordsParser to switch the parsing mode. It means whether the whole kinds including attributes definitions
*   or only kind definitions will be parsed etc. After parsing the function words, parser will step into the main loop.
*
*   Here the grammars are invoked until the whole line is parsed or until some parse error occures. We have to parse
*   the line in such loops because we are allowing so called inline opeartions. That means we can step into context of
*   a object and set attributes of this object using only one line. Setting an attribute value requires the parser to
*   be in context of the object, which attribute are we going to set. So as you can see, context before parsing and
*   during parsing differ. The loops ensures, that after each parsed kind definition, the parsing is terminated so
*   parser could change the parsing context and continue parsing by taking another grammar for the new context.
*   The main loop terminates when the grammars in this loop fail or if the whole line was parsed.
*
*   After this main loop, the parser could take some more actions depending on the parse mode. For example parsing of
*   a new name when renaming some object. After these actions the actual parsing finishes.
*
*   Now we check whether the parsing succeeded or not. Each sub parser is connected to several error handlers that
*   generate parse errors (instances of class ParseError) and store them in a stack. If the parsing fails, we can find
*   the reason in this stack. So when this situation happens, reportParseError() function is called to process
*   the errors stack and report the error.
*
*   The whole parser communicates with the outer world through emitting the signals of the parent parser.
*
*   @see Parser
*   @see ContextStack
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

    /** @short Obtains some help for usage of parser keywords.
    *   
    *   @return Map of keywords, where key is the keyword name and value is description of its usage.
    */
    std::map<std::string, std::string> parserKeywordsUsage();

    /** @short Obtains list of embedded kinds for given kind name.
    *   
    *   @param kindName Kind name for which the embedded kinds will be obtained
    *   @return Vector of embedded kinds
    */
    std::vector<Db::Identifier> parserKindsEmbeds(const Db::Identifier &kindName);

    /** @short Obtains list of attributes for given kind name.
    *   
    *   @param kindName Kind name for which the attributes will be obtained
    *   @return Vector of pairs, where first item is attribute name and second is attribute value type name
    */
    std::vector<std::pair<Db::Identifier, std::string> > parserKindsAttributes(const Db::Identifier &kindName);

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
    ContextStack currentContextStack() const;

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
    void newObject(const Db::Identifier &kind);
    void categoryEntered(const Db::Identifier &kind, const Db::Identifier &name);
    void categoryLeft();
    void attributeSet(const Db::Identifier &kind, const Db::Identifier &name, const Db::Value &value);
    void attributeSetInsert(const Db::Identifier &kind, const Db::Identifier &name, const Db::Identifier &value);
    void attributeSetRemove(const Db::Identifier &kind, const Db::Identifier &name, const Db::Identifier &value);
    void attributeRemove(const Db::Identifier &kind, const Db::Identifier &name);
    void objectsFilter(const Db::Identifier &kind, const boost::optional<Db::Filter> &filter);
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
    void setContextStack(const ContextStack &stack);
    
    /** @short Clears context stack.
    *
    *   For testing purposes.
    */
    void clearContextStack();

private:

    /** @short Fills symbols table of specific attribute parser with all attributes of given kind. */
    void addKindAttributes(const Db::Identifier &kindName, AttributesSettingParser<Iterator> *attributesSettingParser,
                           AttributeRemovalsParser<Iterator> *attributeRemovalsParser,
                           IdentifiersSetsParser<Iterator> *identifiersSetsParser,
                           FilterExpressionsParser<Iterator> *filterExpressionsParser);
    /** @short Fills symbols table of specific kinds parser with all nested kinds of given kind. */
    void addNestedKinds(const Db::Identifier &kindName, KindsOnlyParser<Iterator> *kindsOnlyParser,
                        KindsFiltersParser<Iterator> *kindsFiltersParser,
                        KindsConstructParser<Iterator> *kindsConstructParser);
    void addNestedKinds(const Db::Identifier &kindName, FiltersParser<Iterator> *filtersParser);

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

    /** @short Obtains list of recursively embedded kinds for given kind name.
    *
    *   If kind A embeds kind B and kind B embeds kind C, then this function will return vector containing
    *   B and C for kind A.
    *   
    *   @param kindName Kind name for which the embedded kinds will be obtained
    *   @return Vector of recursively embedded kinds
    */
    std::vector<Db::Identifier> parserKindsEmbedsRecursively(const Db::Identifier &kindName);

    /** @short Checks if the kind contains some attribute of type TYPE_IDENTIFIER_SET
    *
    *   @param kindName Name of the kind
    *   @return True if the kind contains some set
    */
    bool containsIdentifiersSet(const Db::Identifier &kindName);

    //@{
    /** All rules and grammars, that is the whole parser build of are stored there.
    *   Only pointers are used in the main parser.
    */
    std::map<std::string, AttributesSettingParser<Iterator>* > attributesSettingParsers;
    std::map<std::string, AttributeRemovalsParser<Iterator>* > attributeRemovalsParsers;
    std::map<std::string, IdentifiersSetsParser<Iterator>* > identifiersSetsParsers;
    std::map<std::string, AttributesParser<Iterator>* > attributesParsers;
    std::map<std::string, KindsOnlyParser<Iterator>* > kindsOnlyParsers;
    std::map<std::string, KindsConstructParser<Iterator>* > kindsConstructParsers;
    std::map<std::string, FilterExpressionsParser<Iterator>* > filterExpressionsParsers;
    std::map<std::string, FiltersParser<Iterator>* > filtersParsers;
    std::map<std::string, KindsFiltersParser<Iterator>* > kindsFiltersParsers;
    std::map<std::string, KindsParser<Iterator>* > kindsParsers;
    std::map<std::string, WholeKindParser<Iterator>* > wholeKindParsers;
    KindsOnlyParser<Iterator> *topLevelKindsParser;
    KindsFiltersParser<Iterator> *topLevelKindsFiltersParser;
    TopLevelParser<Iterator> *topLevelParser;
    FunctionWordsParser<Iterator> *functionWordsParser;
    PredefinedRules<Iterator> *predefinedRules;
    //@}

    /** Vector of top level objects for tab completion purposes. */
    std::vector<Db::Identifier> topLevelKindsIds;
    /** Identifiers of all kinds. */
    std::vector<Db::Identifier> allKinds;
    /** Map of kind names and kinds, where is the kind embedded. */
    std::map<Db::Identifier, Db::Identifier> embeddedInto;
    /** Map of kind names and kinds, where is the kind embedded including merged kinds. */
    std::map<Db::Identifier, std::vector<Db::Identifier> > embeddedIntoInclMerge;
    /** Map of kinds and vector of kinds, that are embedded in the kind. */
    std::map<Db::Identifier, std::vector<Db::Identifier> > embeds;
    /** Map of kinds and vector of kinds, that are merged with the kind. */
    std::map<Db::Identifier, std::vector<Db::Identifier> > mergeWith;
    /** Map of kinds and that are refferring to another kind. */
    std::map<Db::Identifier, std::vector<std::pair<Db::Identifier, Db::Identifier> > > refersTo;

    /** The parser context is held there. */
    ContextStack contextStack;
    /** Stack with errors, that occures during parsing. @see void addParseError(const ParseError<Iterator> &error) */
    std::vector<ParseError<Iterator> > parseErrors;
    
    /** Pointer to main parser for invoking signals. */
    Parser *m_parser;
    /** Flag, that disables or enables invoking signals. TRUE -> ENABLE, FALSE -> DISABLE */
    bool dryRun;
    /** True when single kind without any attributes was parsed. Means, that nesting will be permanent. */
    bool singleKind;
    /** True when parsing of current line succeeded, alse false. */
    bool parsingSucceededActions;
    /** Current parsing mode. @see ParsingMode */
    ParsingMode parsingMode;
};

}

}



#endif  // DESKA_PARSERPRIVATE_H
