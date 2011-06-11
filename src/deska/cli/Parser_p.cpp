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

//#define PARSER_DEBUG

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

    void operator()(const boost::iterator_range<Iterator> &range, std::string &str) const
    {
        str.assign(range.begin(), range.end());
    }
};



template <typename Iterator>
PredefinedRules<Iterator>::PredefinedRules()
{
    tQuotedString %= qi::lexeme['"' >> +(ascii::char_ - '"') >> '"'];
    tIdentifier %= qi::lexeme[+(ascii::alnum | '_')];

    rulesMap[Db::TYPE_INT] = qi::int_
        [qi::_val = phoenix::static_cast_<int>(qi::_1)];
    rulesMap[Db::TYPE_INT].name("integer");

    // FIXME: consider allowing trivial words without quotes
    rulesMap[Db::TYPE_STRING] = tQuotedString
        [qi::_val = phoenix::static_cast_<std::string>(qi::_1)];
    rulesMap[Db::TYPE_STRING].name("quoted string");

    rulesMap[Db::TYPE_DOUBLE] = qi::double_
        [qi::_val = phoenix::static_cast_<double>(qi::_1)];
    rulesMap[Db::TYPE_DOUBLE].name("double");

    rulesMap[Db::TYPE_IDENTIFIER] = tIdentifier
        [qi::_val = phoenix::static_cast_<std::string>(qi::_1)];
    rulesMap[Db::TYPE_IDENTIFIER].name("identifier (alphanumerical letters and _)");

    objectIdentifier %= tIdentifier.alias();
    objectIdentifier.name("object identifier (alphanumerical letters and _)");
}



template <typename Iterator>
const qi::rule<Iterator, Db::Value(), ascii::space_type>& PredefinedRules<Iterator>::getRule(const Db::Type attrType)
{
    typename std::map<Db::Type, qi::rule<Iterator, Db::Value(), ascii::space_type> >::const_iterator
        it = rulesMap.find(attrType);
    if (it == rulesMap.end()) {
        // Normally, we'd simply assert() here (see git history), but this is a place which would get hit
        // when people add their own low-level data types to Deska. Given that asserts are optimized away
        // in release mode and that I (Jan) can imagine people not bothering with debug builds (you really
        // should build in debug, though), it's better to be explicit here and maybe even save some poor
        // guy some head scratching in future.
        // So, to some future fellow: if we just saved you time, please buy some beer to Tomas :).
        std::stringstream ss;
        ss << "PredefinedRules::getRule: no available grammar rule for parsing of low-level data type " <<
              attrType << ". See " << __FILE__ << ":" << __LINE__ << " for details.";
        throw std::domain_error(ss.str());
    } else {
        return it->second;
    }
}



template <typename Iterator>
const qi::rule<Iterator, Db::Identifier(), ascii::space_type>& PredefinedRules<Iterator>::getObjectIdentifier()
{
    return objectIdentifier;
}



template <typename Iterator>
AttributesParser<Iterator>::AttributesParser(const Db::Identifier &kindName, ParserImpl<Iterator> *parent):
    AttributesParser<Iterator>::base_type(start), m_name(kindName), m_parent(parent)
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
    //using qi::rethrow;

    // If the boost::spirit::qi::grammar API was sane, the following line would read setName(kindName).
    // The API is not sane, and therefore we have the following crap here.
    this->name(kindName);

    phoenix::function<RangeToString<Iterator> > rangeToString = RangeToString<Iterator>();

    // When parsing some input using Nabialek trick, the rule, that is using the symbols table will not be entered when
    // the keyword is not found in the table. The eps is there to ensure, that the start rule will be entered every
    // time and so the error handler for bad keywords could be bound to it. The eoi rule is there to avoid the grammar
    // require more input on the end of the line, which is side effect of eps usage in this way.
    start = (eps(!_a) > dispatch >> -eoi[_a = true]);

    // Attribute name recognized -> try to parse attribute value. The raw function is here to get the name of the
    // attribute being parsed.
    dispatch = ((raw[attributes[_a = _1]][rangeToString(_1, phoenix::ref(currentAttributeName))]
        > lazy(_a)[phoenix::bind(&AttributesParser::parsedAttribute, this, phoenix::ref(currentAttributeName), _1)]));

    phoenix::function<AttributeErrorHandler<Iterator> > attributeErrorHandler = AttributeErrorHandler<Iterator>();
    //phoenix::function<NestingErrorHandler<Iterator> > nestingErrorHandler = NestingErrorHandler<Iterator>();
    phoenix::function<ValueErrorHandler<Iterator> > valueErrorHandler = ValueErrorHandler<Iterator>();
    on_error<fail>(start, attributeErrorHandler(_1, _2, _3, _4,
                                                phoenix::ref(attributes), phoenix::ref(m_name), m_parent));
    // In case of enabling error handler for nesting, on_error<fail> for attributeErrorHandler have to be changed
    // to on_error<rethrow>.
    //on_error<fail>(start, nestingErrorHandler(_1, _2, _3, _4, phoenix::ref(currentAttributeName),
    //                                          phoenix::ref(m_name), m_parent));
    on_error<fail>(dispatch, valueErrorHandler(_1, _2, _3, _4, phoenix::ref(currentAttributeName), m_parent));
}



template <typename Iterator>
void AttributesParser<Iterator>::addAtrribute(const Db::Identifier &attributeName,
                                              qi::rule<Iterator, Db::Value(), ascii::space_type> attributeParser)
{
    attributes.add(attributeName, attributeParser);
}



template <typename Iterator>
void AttributesParser<Iterator>::parsedAttribute(const Db::Identifier &parameter, Db::Value &value)
{
    m_parent->attributeSet(parameter, value);
}



template <typename Iterator>
AttributeRemovalsParser<Iterator>::AttributeRemovalsParser(const Db::Identifier &kindName, ParserImpl<Iterator> *parent):
    AttributeRemovalsParser<Iterator>::base_type(start), m_name(kindName), m_parent(parent)
{
    using qi::_1;
    using qi::_2;
    using qi::_3;
    using qi::_4;
    using qi::_a;
    using qi::eps;
    using qi::raw;
    using qi::on_error;
    using qi::fail;

    // If the boost::spirit::qi::grammar API was sane, the following line would read setName(kindName).
    // The API is not sane, and therefore we have the following crap here.
    this->name(kindName);

    phoenix::function<RangeToString<Iterator> > rangeToString = RangeToString<Iterator>();

    start = (qi::lit("no") > dispatch);

    dispatch = raw[attributes[_a = _1]][rangeToString(_1, phoenix::ref(currentAttributeName))] > lazy(_a)
        [phoenix::bind(&AttributeRemovalsParser::parsedAttributeRemoval, this, phoenix::ref(currentAttributeName))];

    phoenix::function<AttributeRemovalErrorHandler<Iterator> > attributeRemovalErrorHandler =
        AttributeRemovalErrorHandler<Iterator>();

    on_error<fail>(start, attributeRemovalErrorHandler(_1, _2, _3, _4,
                                                       phoenix::ref(attributes), phoenix::ref(m_name), m_parent));
}



template <typename Iterator>
void AttributeRemovalsParser<Iterator>::addAtrribute(const Db::Identifier &attributeName)
{
    attributes.add(attributeName, qi::eps);
}



template <typename Iterator>
void AttributeRemovalsParser<Iterator>::parsedAttributeRemoval(const Db::Identifier &attribute)
{
    m_parent->attributeRemove(attribute);
}



template <typename Iterator>
KindsOnlyParser<Iterator>::KindsOnlyParser(const Db::Identifier &kindName, ParserImpl<Iterator> *parent):
    KindsOnlyParser<Iterator>::base_type(start), m_name(kindName), m_parent(parent)
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
    //using qi::rethrow;

    // If the boost::spirit::qi::grammar API was sane, the following line would read setName(kindName).
    // The API is not sane, and therefore we have the following crap here.
    this->name(kindName);

    phoenix::function<RangeToString<Iterator> > rangeToString = RangeToString<Iterator>();

    // When parsing some input using Nabialek trick, the rule, that is using the symbols table will not be entered when
    // the keyword is not found in the table. The eps is there to ensure, that the start rule will be entered every
    // time and so the error handler for bad keywords could be bound to it. The eoi rule is there to avoid the grammar
    // require more input on the end of the line, which is side effect of eps usage in this way.
    start = (eps(!_a) > dispatch >> -eoi[_a = true]);

    // Attribute name recognized -> try to parse attribute value. The raw function is here to get the name of the
    // attribute being parsed.
    dispatch = (raw[kinds[_a = _1]][rangeToString(_1, phoenix::ref(currentKindName))]
        > lazy(_a)[phoenix::bind(&KindsOnlyParser::parsedKind, this, phoenix::ref(currentKindName), _1)]);

    phoenix::function<KindErrorHandler<Iterator> > kindErrorHandler = KindErrorHandler<Iterator>();
    //phoenix::function<NestingErrorHandler<Iterator> > nestingErrorHandler = NestingErrorHandler<Iterator>();
    phoenix::function<ValueErrorHandler<Iterator> > valueErrorHandler = ValueErrorHandler<Iterator>();
    on_error<fail>(start, kindErrorHandler(_1, _2, _3, _4, phoenix::ref(kinds), phoenix::ref(m_name), m_parent));
    // In case of enabling error handler for nesting, on_error<fail> for kindErrorHandler have to be changed
    // to on_error<rethrow>.
    //on_error<fail>(start, nestingErrorHandler(_1, _2, _3, _4, phoenix::ref(currentKindName),
    //                                          phoenix::ref(m_name), m_parent));
    on_error<fail>(dispatch, valueErrorHandler(_1, _2, _3, _4, phoenix::ref(currentKindName), m_parent));
}



template <typename Iterator>
void KindsOnlyParser<Iterator>::addKind(const Db::Identifier &kindName,
                                        qi::rule<Iterator, Db::Identifier(), ascii::space_type> identifierParser)
{
    kinds.add(kindName, identifierParser);
}



template <typename Iterator>
void KindsOnlyParser<Iterator>::parsedKind(const Db::Identifier &kindName, const Db::Identifier &objectName)
{
    m_parent->categoryEntered(kindName, objectName);
}



template <typename Iterator>
WholeKindParser<Iterator>::WholeKindParser(const Db::Identifier &kindName,
                                           AttributesParser<Iterator> *attributesParser,
                                           AttributeRemovalsParser<Iterator> *attributeRemovalsParser,
                                           KindsOnlyParser<Iterator> *nestedKinds, ParserImpl<Iterator> *parent):
    WholeKindParser<Iterator>::base_type(start), m_parent(parent)
{
    // If the boost::spirit::qi::grammar API was sane, the following line would read setName(kindName).
    // The API is not sane, and therefore we have the following crap here.
    this->name(kindName);

    start = ((+((*attributesParser) | (*attributeRemovalsParser)) >> -(*nestedKinds))
        | ((*nestedKinds)[phoenix::bind(&WholeKindParser::parsedSingleKind, this)])
        | (qi::lit("end")[phoenix::bind(&WholeKindParser::parsedEnd, this)]));
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
FunctionWordsParser<Iterator>::FunctionWordsParser(ParserImpl<Iterator> *parent):
    FunctionWordsParser<Iterator>::base_type(start), m_parent(parent)
{
    start = ((qi::lit("delete")[phoenix::bind(&FunctionWordsParser::actionDelete, this)])
           | (qi::lit("show")[phoenix::bind(&FunctionWordsParser::actionShow, this)])
           | (qi::lit("rename")[phoenix::bind(&FunctionWordsParser::actionRename, this)]));
}



template <typename Iterator>
void FunctionWordsParser<Iterator>::actionDelete()
{
    m_parent->setParsingMode(PARSING_MODE_DELETE);
}



template <typename Iterator>
void FunctionWordsParser<Iterator>::actionShow()
{
    m_parent->setParsingMode(PARSING_MODE_SHOW);
}



template <typename Iterator>
void FunctionWordsParser<Iterator>::actionRename()
{
    m_parent->setParsingMode(PARSING_MODE_RENAME);
}



template <typename Iterator>
ParserImpl<Iterator>::ParserImpl(Parser *parent): m_parser(parent)
{
    predefinedRules = new PredefinedRules<Iterator>();
    topLevelParser = new KindsOnlyParser<Iterator>(std::string(""),this);
    functionWordsParser = new FunctionWordsParser<Iterator>(this);

    std::vector<Db::Identifier> kinds = m_parser->m_dbApi->kindNames();

    for (std::vector<Db::Identifier>::iterator it = kinds.begin(); it != kinds.end(); ++it) {
        // Add new kind to the top-level parser
        topLevelParser->addKind(*it, predefinedRules->getObjectIdentifier());

        // Create attributes parser for new kind
        attributesParsers[*it] = new AttributesParser<Iterator>(*it, this);
        attributeRemovalsParsers[*it] = new AttributeRemovalsParser<Iterator>(*it, this);
        addKindAttributes(*it, attributesParsers[*it], attributeRemovalsParsers[*it]);

        // Create nested kinds parser for the new kind
        kindsOnlyParsers[*it] = new KindsOnlyParser<Iterator>(*it, this);
        addNestedKinds(*it, kindsOnlyParsers[*it]);

        // And combine them in the parser for the whole kind
        wholeKindParsers[*it] = new WholeKindParser<Iterator>(
            *it, attributesParsers[*it], attributeRemovalsParsers[*it], kindsOnlyParsers[*it], this);
    }
}



template <typename Iterator>
ParserImpl<Iterator>::~ParserImpl()
{
    for (typename std::map<std::string, AttributesParser<Iterator>* >::iterator it = attributesParsers.begin();
        it != attributesParsers.end(); ++it) {
        delete it->second;
    }
    for (typename std::map<std::string, AttributeRemovalsParser<Iterator>* >::iterator it = attributeRemovalsParsers.begin();
        it != attributeRemovalsParsers.end(); ++it) {
        delete it->second;
    }
    for (typename std::map<std::string, WholeKindParser<Iterator>* >::iterator it = wholeKindParsers.begin();
        it != wholeKindParsers.end(); ++it) {
        delete it->second;
    }
    for (typename std::map<std::string, KindsOnlyParser<Iterator>* >::iterator it = kindsOnlyParsers.begin();
        it != kindsOnlyParsers.end(); ++it) {
        delete it->second;
    }

    delete topLevelParser;
    delete functionWordsParser;
    delete predefinedRules;
}



template <typename Iterator>
std::map<std::string, std::string> ParserImpl<Iterator>::parserKeywordsUsage()
{
    std::map<std::string, std::string> usages;
    usages["delete"] = "Deletes object given as parameter (e.g. delete hardware hp456). Longer parameters are also allowed (e.g. delete host golias120 interface eth0) This will delete only interface eth0 in the object host golias120.";
    usages["show"] = "Shows attributes and nested kinds of the object. Parameter is here optional and works in the same way as for delete. When executed without parameter at top-level, it shows all object kinds and names.";
    usages["end"] = "Leaves one level of current context.";
    usages["no"] = "When entered in front of an attribute name, it removes it's value.";
    return usages;
}



template <typename Iterator>
std::vector<Db::Identifier> ParserImpl<Iterator>::parserKindsEmbeds(const Db::Identifier &kindName)
{
    std::vector<std::string> embeds;
    std::vector<Db::Identifier> kinds = m_parser->m_dbApi->kindNames();
    for (std::vector<Db::Identifier>::iterator itk = kinds.begin(); itk != kinds.end(); ++itk) {
        std::vector<Db::ObjectRelation> relations = m_parser->m_dbApi->kindRelations(*itk);
        for (std::vector<Db::ObjectRelation>::iterator itr = relations.begin(); itr != relations.end(); ++itr) {
            if ((itr->kind == Db::RELATION_EMBED_INTO) && (itr->target == kindName)) {
                embeds.push_back(*itk);
            }
        }
    }
    return embeds;
}



template <typename Iterator>
std::vector<std::pair<Db::Identifier, std::string> > ParserImpl<Iterator>::parserKindsAttributes(const Db::Identifier &kindName)
{
    std::vector<std::pair<Db::Identifier, std::string> > attrs;
    std::vector<Db::KindAttributeDataType> attributes = m_parser->m_dbApi->kindAttributes(kindName);
    for (std::vector<Db::KindAttributeDataType>::iterator ita = attributes.begin(); ita != attributes.end(); ++ita) {
        attrs.push_back(std::make_pair<Db::Identifier, std::string>(ita->name, predefinedRules->getRule(ita->type).name()));
    }
    return attrs;
}



template <typename Iterator>
void ParserImpl<Iterator>::parseLine(const std::string &line)
{
    dryRun = false;
    if (parseLineImpl(line))
        m_parser->parsingFinished();
}



template <typename Iterator>
bool ParserImpl<Iterator>::isNestedInContext() const
{
    return !contextStack.empty();
}



template <typename Iterator>
Db::ContextStack ParserImpl<Iterator>::currentContextStack() const
{
    return contextStack;
}



template <typename Iterator>
std::vector<std::string> ParserImpl<Iterator>::tabCompletionPossibilities(const std::string &line)
{
    // We have to restore previous context stack
    Db::ContextStack contextStackBackup = contextStack;
    std::vector<std::string> possibilities;
    if (line.empty()) {
        insertTabPossibilitiesOfCurrentContext(line, possibilities);
    } else {
        dryRun = true;
        bool parsingSucceeded;
        parsingSucceeded = parseLineImpl(line);
        if (parsingSucceeded) {
            if (*(line.end()-1) == ' ') {
                insertTabPossibilitiesOfCurrentContext(line, possibilities);
            } else {
                // This should not happen, because CliCompleter truncates the last uncomplete token
            }
        } else {
            if (*(line.end()-1) == ' ') {
                insertTabPossibilitiesFromErrors(line, possibilities);
            } else {
                // This should not happen, because CliCompleter truncates the last uncomplete token
            }
        }
    }
    // Restore previous context stack
    contextStack = contextStackBackup;
    return possibilities;
}



template <typename Iterator>
void ParserImpl<Iterator>::categoryEntered(const Db::Identifier &kind, const Db::Identifier &name)
{
    contextStack.push_back(Db::ObjectDefinition(kind, name));
    if (!dryRun)
        m_parser->categoryEntered(kind, name);
#ifdef PARSER_DEBUG
    std::cout << "Category entered: " << kind << ": " << name << std::endl;
#endif
}



template <typename Iterator>
void ParserImpl<Iterator>::categoryLeft()
{
    contextStack.pop_back();
    if (!dryRun)
        m_parser->categoryLeft();
#ifdef PARSER_DEBUG
    std::cout << "Category left" << std::endl;
#endif
}



template <typename Iterator>
void ParserImpl<Iterator>::attributeSet(const Db::Identifier &name, const Db::Value &value)
{
    if (!dryRun)
        m_parser->attributeSet(name, value);
#ifdef PARSER_DEBUG
    std::cout << "Set attribute: " << name << "=" << value << std::endl;
#endif
}



template <typename Iterator>
void ParserImpl<Iterator>::attributeRemove(const Db::Identifier &name)
{
    if (!dryRun)
        m_parser->attributeRemove(name);
#ifdef PARSER_DEBUG
    std::cout << "Remove attribute: " << name << std::endl;
#endif
}



template <typename Iterator>
void ParserImpl<Iterator>::parsedSingleKind()
{
    singleKind = true;
}



template <typename Iterator>
void ParserImpl<Iterator>::addParseError(const ParseError<Iterator> &error)
{
    parseErrors.push_back(error);
}



template <typename Iterator>
void ParserImpl<Iterator>::setParsingMode(ParsingMode mode)
{
    parsingMode = mode;
}



template <typename Iterator>
ParsingMode ParserImpl<Iterator>::getParsingMode()
{
    return parsingMode;
}



template <typename Iterator>
std::vector<Db::Identifier> ParserImpl<Iterator>::getKindNames()
{
    return m_parser->m_dbApi->kindNames();
}



template <typename Iterator>
void ParserImpl<Iterator>::setContextStack(const Db::ContextStack &stack)
{
    contextStack = stack;
}



template <typename Iterator>
void ParserImpl<Iterator>::clearContextStack()
{
    contextStack.clear();
}



template <typename Iterator>
void ParserImpl<Iterator>::addKindAttributes(const Db::Identifier &kindName,
                                             AttributesParser<Iterator>* attributesParser,
                                             AttributeRemovalsParser<Iterator>* attributeRemovalsParser)
{
    std::vector<Db::KindAttributeDataType> attributes = m_parser->m_dbApi->kindAttributes(kindName);
    for (std::vector<Db::KindAttributeDataType>::iterator it = attributes.begin(); it != attributes.end(); ++it) {
        attributesParser->addAtrribute(it->name, predefinedRules->getRule(it->type));
        attributeRemovalsParser->addAtrribute(it->name);
    }
}



template <typename Iterator>
void ParserImpl<Iterator>::addNestedKinds(const Db::Identifier &kindName, KindsOnlyParser<Iterator>* kindsOnlyParser)
{
    std::vector<Db::Identifier> kinds = m_parser->m_dbApi->kindNames();
    for (std::vector<Db::Identifier>::iterator it = kinds.begin(); it != kinds.end(); ++it) {
        std::vector<Db::ObjectRelation> relations = m_parser->m_dbApi->kindRelations(*it);
        for (std::vector<Db::ObjectRelation>::iterator itr = relations.begin(); itr != relations.end(); ++itr) {
            if ((itr->kind == Db::RELATION_EMBED_INTO) && (itr->target == kindName)) {
                kindsOnlyParser->addKind(*it, predefinedRules->getObjectIdentifier());
            }
        }
    }
}



template <typename Iterator>
bool ParserImpl<Iterator>::parseLineImpl(const std::string &line)
{
#ifdef PARSER_DEBUG
    std::cout << "Parse line: \"" << line << "\"" << std::endl;
    std::cout << "Start context stack: " << Db::contextStackToString(contextStack) << std::endl;
#endif

    Iterator iter = line.begin();
    Iterator end = line.end(); 

    parseErrors.clear();

    parsingMode = PARSING_MODE_STANDARD;
    bool parsingSucceeded = true;
    singleKind = false;
    bool topLevel = false;
    int parsingIterations = 0;
    bool functionWordParsed = false;
    bool nonexistantObject = false;
    Db::ContextStack::size_type previousContextStackSize = contextStack.size();

    // Check if there are any function words at the beginning of the line.
    functionWordParsed = phrase_parse(iter, end, *functionWordsParser, ascii::space);

    // Function word parsed alone
    if ((iter == end) && (functionWordParsed))
    {
        switch (parsingMode) {
            case PARSING_MODE_SHOW:
                // Function show does not require any parameters -> emit signals
#ifdef PARSER_DEBUG
                std::cout << "Action Show" << std::endl;
                std::cout << "End context stack: " << Db::contextStackToString(contextStack) << std::endl;
#endif
                if (!dryRun)
                    m_parser->functionShow();
                return true;
                break;
            case PARSING_MODE_DELETE:
                // Function delete requires parameter -> report error
                if (contextStack.empty()) {
                    addParseError(ParseError<Iterator>(line.begin(), end, iter, "", m_parser->m_dbApi->kindNames()));
                    parsingSucceeded = false;
                } else {
                    std::vector<Db::Identifier> nestedKinds;
                    std::vector<Db::Identifier> kinds = m_parser->m_dbApi->kindNames();
                    for (std::vector<Db::Identifier>::iterator it = kinds.begin(); it != kinds.end(); ++it) {
                        std::vector<Db::ObjectRelation> relations = m_parser->m_dbApi->kindRelations(*it);
                        for (std::vector<Db::ObjectRelation>::iterator itr = relations.begin();
                             itr != relations.end(); ++itr) {
                            if ((itr->kind == Db::RELATION_EMBED_INTO) &&
                                (itr->target == contextStack.back().kind)) {
                                nestedKinds.push_back(*it);
                            }
                        }
                    }
                    addParseError(ParseError<Iterator>(line.begin(), end, iter, contextStack.back().kind, nestedKinds));
                    parsingSucceeded = false;
                }
                break;
            case PARSING_MODE_RENAME:
                // Function rename requires parameter -> report error
                if (contextStack.empty()) {
                    addParseError(ParseError<Iterator>(line.begin(), end, iter, "", m_parser->m_dbApi->kindNames()));
                    parsingSucceeded = false;
                } else {
                    addParseError(ParseError<Iterator>(line.begin(), end, iter));
                    parsingSucceeded = false;
                }
                break;
            case PARSING_MODE_STANDARD:
                // Parsed function word -> parsing mode should change from PARSING_MODE_STANDARD to another
                throw std::logic_error("Function words should change the parsing mode");
                break;
            default:
                throw std::domain_error("Invalid value of parsingMode");
        }
    }

    while (iter != end) {
        singleKind = false;
        ++parsingIterations;
        if (contextStack.empty()) {
            // No context, parse top-level objects
            topLevel = true;
            parsingSucceeded = phrase_parse(iter, end, *topLevelParser, ascii::space);
        } else {
            // Context -> parse attributes or nested kinds
            topLevel = false;         
            switch (parsingMode) {
                case PARSING_MODE_STANDARD:
                    parsingSucceeded = phrase_parse(iter, end, *(wholeKindParsers[contextStack.back().kind]),
                                                    ascii::space);
                    break;
                case PARSING_MODE_DELETE:
                case PARSING_MODE_SHOW:
                case PARSING_MODE_RENAME:
                    parsingSucceeded = phrase_parse(iter, end, *(kindsOnlyParsers[contextStack.back().kind]),
                                                    ascii::space);
                    break;
                default:
                    throw std::domain_error("Invalid value of parsingMode");
            }        
        }

        // Check for existence of parsed kind instance and add parse error based on parsing mode.
        if (!contextStack.empty() && parsingSucceeded) {
            std::vector<Db::Identifier> instances;
            switch (parsingMode) {
                case PARSING_MODE_STANDARD:
                    break;
                case PARSING_MODE_DELETE:
                case PARSING_MODE_SHOW:
                case PARSING_MODE_RENAME:
                    // Modes SHOW, DELETE and RENAME requires existing kind instances.
                    instances = m_parser->m_dbApi->kindInstances(contextStack.back().kind);
                    if (std::find(instances.begin(), instances.end(), Db::contextStackToPath(contextStack)) == instances.end()) {
                        addParseError(ParseError<Iterator>(line.begin(), end, iter - contextStack.back().name.size() - 1,
                                                           contextStack.back().kind, contextStack.back().name));
                        parsingSucceeded = false;
                        nonexistantObject = true;
                    }
                    break;
                default:
                    throw std::domain_error("Invalid value of parsingMode");
            }
        }

        if (!parsingSucceeded) {
            // Some bad input
            break;
        } else {
            // Some errors could be generated even though parsing succeeded, because of the way how boost::spirit
            // works in case of alternatives grammar. When trying to parse some input, spirit tries to pass it to
            // the first rule in the grammar and when the rule fails, it tries to pass the input to the next rule.
            // The next rule can succeed, but an error handler of the first rule is already invoked. So we have to
            // clear these errors, that are not actual errors.
            parseErrors.clear();
        }     
    }

    // New name when renaming an object
    Db::Identifier newName;

    // Handle parsing of new name here
    // Parsing mode is rename and some object to be renamed was parsed
    if ((parsingMode == PARSING_MODE_RENAME) && (contextStack.size() > previousContextStackSize)) {
        if ((parsingIterations > 0) && (!nonexistantObject)) {
            // Function word not parsed alone -> error was not reported yet
            if (iter == end) {
                // Missing new name
                parsingSucceeded = false;
                addParseError(ParseError<Iterator>(line.begin(), end, iter));
            } else if (contextStack.empty()) {
                // Missing object to rename
                parsingSucceeded = false;
                addParseError(ParseError<Iterator>(line.begin(), end, iter, "", m_parser->m_dbApi->kindNames()));
            } else {
                // We are ready to parse new name
                parsingSucceeded = phrase_parse(iter, end, predefinedRules->getObjectIdentifier(),
                                                ascii::space, newName);
                if (iter != end)
                    parsingSucceeded = false;
                if (!parsingSucceeded)
                    addParseError(ParseError<Iterator>(line.begin(), end, iter));
            }
        }
    }

    if (!parsingSucceeded) {
         // Some bad input
        if (!dryRun)
            reportParseError(line);
    }

    if ((!dryRun) && (parsingSucceeded)) {
        // Emit signals, when there is some function word used.
        switch (parsingMode) {
            case PARSING_MODE_STANDARD:
                // No special signal to be triggered inthis case.
                break;
            case PARSING_MODE_DELETE:
#ifdef PARSER_DEBUG
                std::cout << "Action Delete" << std::endl;
#endif
                m_parser->functionDelete();
                break;
            case PARSING_MODE_SHOW:
#ifdef PARSER_DEBUG
                std::cout << "Action Show" << std::endl;
#endif
                m_parser->functionShow();
                break;
            case PARSING_MODE_RENAME:
#ifdef PARSER_DEBUG
                std::cout << "Action Rename: " << newName << std::endl;
#endif
                m_parser->functionRename(newName);
                break;
            default:
                throw std::domain_error("Invalid value of parsingMode");
        } 
    }
    // Invoke categoryLeft signals when parsing in-line definitions. Do not invoke categoryLeft
    // when in dryRun for purposes of generating tab completions.
    if (((parsingMode == PARSING_MODE_STANDARD) && (singleKind || topLevel)) | (dryRun)) {
        // Definition of kind found stand-alone in standard mode on one line -> nest permanently
    } else {
        int depthDiff = contextStack.size() - previousContextStackSize;
        if (depthDiff > 0) {
            for (int i = 0; i < depthDiff; ++i) {
                categoryLeft();
            }
        }
    }
#ifdef PARSER_DEBUG
    std::cout << "End context stack: " << Db::contextStackToString(contextStack) << std::endl;
#endif
    return parsingSucceeded;
}



template <typename Iterator>
void ParserImpl<Iterator>::reportParseError(const std::string& line)
{
    // No more than three errors should occur. Three errors occur only when bad identifier of embedded object is set.
    BOOST_ASSERT(parseErrors.size() <= 3);

    // If there is no error, it means, that we entered some bad input fot the kind, that has no attributes and no
    // nested kinds, so there was no grammar to generate the error.
    if (parseErrors.empty()) {
        m_parser->parseError(NoAttributesOrKindsDefined("No attributes or nested kind names expected here.",
                                                        line, line.begin()));
        return;
    }

    typename std::vector<ParseError<Iterator> >::iterator it;

    // At first, find out if it's caused by a non-conforming data type. That would mean that it's caused
    // by an error in the attribute value
    it = std::find_if(parseErrors.begin(), parseErrors.end(), phoenix::bind(&ParseError<Iterator>::errorType,
                      phoenix::arg_names::_1) == PARSE_ERROR_TYPE_VALUE_TYPE);
    if (it != parseErrors.end()) {
        // Yes, error in an attribute's value. That's all what's interesting for us, so let's ignore any other errors
        // which could be reported by spirit as a result of the error propagation.
#ifdef PARSER_DEBUG
        std::cout << it->toString() << std::endl;
#endif
        m_parser->parseError(InvalidAttributeDataTypeError(it->toString(), line, it->errorPosition()));
        return;
    }

    // Find out, if the error occured when parsing attribute name for value removal
    it = std::find_if(parseErrors.begin(), parseErrors.end(), phoenix::bind(&ParseError<Iterator>::errorType,
                      phoenix::arg_names::_1) == PARSE_ERROR_TYPE_ATTRIBUTE_REMOVAL);
    if (it != parseErrors.end()) {
        // Yes, error occured when parsing attribute name for value removal
#ifdef PARSER_DEBUG
        std::cout << it->toString() << std::endl;
#endif
        m_parser->parseError(UndefinedAttributeError(it->toString(), line, it->errorPosition()));
        return;
    }

    // Find out, if the error occured when parsing identifier
    it = std::find_if(parseErrors.begin(), parseErrors.end(), phoenix::bind(&ParseError<Iterator>::errorType,
                      phoenix::arg_names::_1) == PARSE_ERROR_TYPE_IDENTIFIER_NOT_FOUND);
    if (it != parseErrors.end()) {
        // Yes, error occured when parsing identifier
#ifdef PARSER_DEBUG
        std::cout << it->toString() << std::endl;
#endif
        m_parser->parseError(MalformedIdentifier(it->toString(), line, it->errorPosition()));
        return;
    }

    // There's no trace of an error in the attribute data anywhere
    if (parseErrors.size() == 1) {
        // whatever it is, let's just store it
        const ParseError<Iterator> &err = parseErrors.front();
#ifdef PARSER_DEBUG
        std::cout << err.toString() << std::endl;
#endif
        switch (err.errorType()) {
        case PARSE_ERROR_TYPE_ATTRIBUTE:
            m_parser->parseError(UndefinedAttributeError(err.toString(), line, err.errorPosition()));
            break;
        case PARSE_ERROR_TYPE_KIND:
            m_parser->parseError(InvalidObjectKind(err.toString(), line, err.errorPosition()));
            break;
        case PARSE_ERROR_TYPE_OBJECT_DEFINITION_NOT_FOUND:
            m_parser->parseError(ObjectDefinitionNotFound(err.toString(), line, err.errorPosition()));
            break;
        case PARSE_ERROR_TYPE_OBJECT_NOT_FOUND:
            m_parser->parseError(ObjectNotFound(err.toString(), line, err.errorPosition()));
            break;
        default:
            throw std::domain_error("Invalid value of ParseErrorType");
        }
    } else if (parseErrors.size() == 2) {
        // Two errors can occur only when bad identifier of attribute or nested kind for some kind with embedded
        // kinds is set. These errors are PARSE_ERROR_TYPE_ATTRIBUTE and PARSE_ERROR_TYPE_KIND. Lets merge them.
#ifdef PARSER_DEBUG
        std::cout << parseErrors[0].toCombinedString(parseErrors[1]) << std::endl;
#endif
        m_parser->parseError(UndefinedAttributeError(
            parseErrors[0].toCombinedString(parseErrors[1]), line, parseErrors[0].errorPosition()));
    } else {
        throw std::out_of_range(
            "Parse error reporting: got more than two errors, but none of them is a PARSE_ERROR_TYPE_VALUE_TYPE");
    }
}



template <typename Iterator>
void ParserImpl<Iterator>::insertTabPossibilitiesOfCurrentContext(const std::string &line,
                                                                  std::vector<std::string> &possibilities)
{
    if (line.empty())
        possibilities.push_back("show");
    if (contextStack.empty()) {
        // No context -> add names of top-level kinds
        std::vector<Db::Identifier> kinds = m_parser->m_dbApi->kindNames();
        for (std::vector<Db::Identifier>::iterator it = kinds.begin(); it != kinds.end(); ++it) {
            possibilities.push_back(line + *it);
        }
        if ((!kinds.empty()) && (line.empty())) {
            possibilities.push_back(line + "delete");
            possibilities.push_back(line + "rename");
        }
    } else {
        // Do not add completions of attributes when in non-standard mode.
        if (parsingMode == PARSING_MODE_STANDARD) {
            // Add names of attributes of current kind
            std::vector<Db::KindAttributeDataType> attributes = m_parser->m_dbApi->kindAttributes(contextStack.back().kind);
            for (std::vector<Db::KindAttributeDataType>::iterator it = attributes.begin(); it != attributes.end(); ++it) {
                possibilities.push_back(line + it->name);
            }
            if (!attributes.empty())
                possibilities.push_back(line + "no");
            possibilities.push_back(line + "end");
        }
        // Add names of nested kinds of current kind
        bool embedFound = false;
        std::vector<Db::Identifier> kinds = m_parser->m_dbApi->kindNames();
        for (std::vector<Db::Identifier>::iterator it = kinds.begin(); it != kinds.end(); ++it) {
            std::vector<Db::ObjectRelation> relations = m_parser->m_dbApi->kindRelations(*it);
            for (std::vector<Db::ObjectRelation>::iterator itr = relations.begin(); itr != relations.end(); ++itr) {
                if ((itr->kind == Db::RELATION_EMBED_INTO) && (itr->target == contextStack.back().kind)) {
                    possibilities.push_back(line + *it);
                    embedFound = true;
                }
            }
        }
        if ((embedFound) && (line.empty())) {
            possibilities.push_back(line + "delete");
            possibilities.push_back(line + "rename");
        }
    }
}



template <typename Iterator>
void ParserImpl<Iterator>::insertTabPossibilitiesFromErrors(const std::string &line,
                                                                  std::vector<std::string> &possibilities)
{
    std::string::const_iterator realEnd = line.end() - 1;
    while (*realEnd != ' ') {
        if (realEnd == line.begin())
            break;
        --realEnd;
    }
    // Step to our imaginary line end.
    ++realEnd;

    typename std::vector<ParseError<Iterator> >::iterator it;

    // At first, find out if the user wants to enter some value
    it = std::find_if(parseErrors.begin(), parseErrors.end(), phoenix::bind(&ParseError<Iterator>::errorType,
                      phoenix::arg_names::_1) == PARSE_ERROR_TYPE_VALUE_TYPE);
    if (it != parseErrors.end()) {
        // Error have to occur at the end of the line
        if ((realEnd - it->errorPosition()) != 0)
            return;
        std::vector<std::string> expectations = it->expectedTypes();
        // Check if the user is supposed to enter some objects name, that we can complete
        if (expectations.front() == "object identifier (alphanumerical letters and _)") {
            if (!(it->context().empty())) {
                std::vector<Db::Identifier> objects = m_parser->m_dbApi->kindInstances(it->context());
                for (std::vector<Db::Identifier>::iterator iti = objects.begin(); iti != objects.end(); ++iti) {
                    possibilities.push_back(line + *iti);
                }
            }
        }
        return;
    }

    // Find out, if the user wants to enter attribute name for value removal
    it = std::find_if(parseErrors.begin(), parseErrors.end(), phoenix::bind(&ParseError<Iterator>::errorType,
                      phoenix::arg_names::_1) == PARSE_ERROR_TYPE_ATTRIBUTE_REMOVAL);
    if (it != parseErrors.end()) {
        // Error have to occur at the end of the line
        // Because of parsing the pair no <attribute name> using sequence parser with space skipper error occures
        // right after no keyword. That means it is one character before end of the line.
        if ((realEnd - it->errorPosition() - 1) != 0)
            return;
        std::vector<std::string> expectations = it->expectedKeywords();
        for (std::vector<std::string>::iterator iti = expectations.begin(); iti != expectations.end(); ++iti) {
            possibilities.push_back(line + *iti);
        }
        return;
    }

    // Find out, if the user wants to enter kind name and object name for renaming or deleting
    it = std::find_if(parseErrors.begin(), parseErrors.end(), phoenix::bind(&ParseError<Iterator>::errorType,
                      phoenix::arg_names::_1) == PARSE_ERROR_TYPE_OBJECT_DEFINITION_NOT_FOUND);
    if (it != parseErrors.end()) {
        // Error have to occur at the end of the line
        if ((realEnd - it->errorPosition()) != 0)
            return;
        std::vector<std::string> expectations = it->expectedKeywords();
        for (std::vector<std::string>::iterator iti = expectations.begin(); iti != expectations.end(); ++iti) {
            possibilities.push_back(line + *iti);
        }
        return;
    }

    // Find out, if the user wants to enter kind name
    it = std::find_if(parseErrors.begin(), parseErrors.end(), phoenix::bind(&ParseError<Iterator>::errorType,
                      phoenix::arg_names::_1) == PARSE_ERROR_TYPE_KIND);
    if (it != parseErrors.end()) {
        // Error have to occur at the end of the line
        if ((realEnd - it->errorPosition()) != 0)
            return;
        std::vector<std::string> expectations = it->expectedKeywords();
        for (std::vector<std::string>::iterator iti = expectations.begin(); iti != expectations.end(); ++iti) {
            possibilities.push_back(line + *iti);
        }
        return;
    }

    // Find out, if the user wants to enter attribute name
    it = std::find_if(parseErrors.begin(), parseErrors.end(), phoenix::bind(&ParseError<Iterator>::errorType,
                      phoenix::arg_names::_1) == PARSE_ERROR_TYPE_ATTRIBUTE);
    if (it != parseErrors.end()) {
        // Error have to occur at the end of the line
        if ((realEnd - it->errorPosition()) != 0)
            return;
        std::vector<std::string> expectations = it->expectedKeywords();
        for (std::vector<std::string>::iterator iti = expectations.begin(); iti != expectations.end(); ++iti) {
            possibilities.push_back(line + *iti);
        }
        return;
    }
}



/////////////////////////Template instances for linker//////////////////////////

template void RangeToString<iterator_type>::operator()(const boost::iterator_range<iterator_type> &rng, std::string &str) const;

template PredefinedRules<iterator_type>::PredefinedRules();

template const qi::rule<iterator_type, Db::Value(), ascii::space_type>& PredefinedRules<iterator_type>::getRule(const Db::Type attrType);

template const qi::rule<iterator_type, Db::Identifier(), ascii::space_type>& PredefinedRules<iterator_type>::getObjectIdentifier();

template AttributesParser<iterator_type>::AttributesParser(const Db::Identifier &kindName, ParserImpl<iterator_type> *parent);

template void AttributesParser<iterator_type>::addAtrribute(const Db::Identifier &attributeName, qi::rule<iterator_type, Db::Value(), ascii::space_type> attributeParser);

template void AttributesParser<iterator_type>::parsedAttribute(const Db::Identifier &parameter, Db::Value &value);

template AttributeRemovalsParser<iterator_type>::AttributeRemovalsParser(const Db::Identifier &kindName, ParserImpl<iterator_type> *parent);

template void AttributeRemovalsParser<iterator_type>::addAtrribute(const Db::Identifier &attributeName);

template void AttributeRemovalsParser<iterator_type>::parsedAttributeRemoval(const Db::Identifier &parameter);

template KindsOnlyParser<iterator_type>::KindsOnlyParser(const Db::Identifier &kindName, ParserImpl<iterator_type> *parent);

template void KindsOnlyParser<iterator_type>::addKind(const Db::Identifier &kindName,qi::rule<iterator_type, Db::Identifier(), ascii::space_type> identifierParser);

template void KindsOnlyParser<iterator_type>::parsedKind(const Db::Identifier &kindName, const Db::Identifier &objectName);

template WholeKindParser<iterator_type>::WholeKindParser(const std::string &kindName, AttributesParser<iterator_type> *attributesParser, AttributeRemovalsParser<iterator_type> *attributeRemovalsParser, KindsOnlyParser<iterator_type> *nestedKinds, ParserImpl<iterator_type> *parent);

template void WholeKindParser<iterator_type>::parsedEnd();

template void WholeKindParser<iterator_type>::parsedSingleKind();

template FunctionWordsParser<iterator_type>::FunctionWordsParser(ParserImpl<iterator_type> *parent);

template void FunctionWordsParser<iterator_type>::actionDelete();

template void FunctionWordsParser<iterator_type>::actionShow();

template ParserImpl<iterator_type>::ParserImpl(Parser *parent);

template ParserImpl<iterator_type>::~ParserImpl();

template std::map<std::string, std::string> ParserImpl<iterator_type>::parserKeywordsUsage();

template std::vector<Db::Identifier> ParserImpl<iterator_type>::parserKindsEmbeds(const Db::Identifier &kindName);

template std::vector<std::pair<Db::Identifier, std::string> > ParserImpl<iterator_type>::parserKindsAttributes(const Db::Identifier &kindName);

template void ParserImpl<iterator_type>::parseLine(const std::string &line);

template bool ParserImpl<iterator_type>::isNestedInContext() const;

template Db::ContextStack ParserImpl<iterator_type>::currentContextStack() const;

template std::vector<std::string> ParserImpl<iterator_type>::tabCompletionPossibilities(const std::string &line);

template void ParserImpl<iterator_type>::categoryEntered(const Db::Identifier &kind, const Db::Identifier &name);

template void ParserImpl<iterator_type>::categoryLeft();

template void ParserImpl<iterator_type>::attributeSet(const Db::Identifier &name, const Db::Value &value);

template void ParserImpl<iterator_type>::attributeRemove(const Db::Identifier &name);

template void ParserImpl<iterator_type>::parsedSingleKind();

template void ParserImpl<iterator_type>::addParseError(const ParseError<iterator_type> &error);

template void ParserImpl<iterator_type>::setParsingMode(ParsingMode mode);

template ParsingMode ParserImpl<iterator_type>::getParsingMode();

template std::vector<Db::Identifier> ParserImpl<iterator_type>::getKindNames();

template void ParserImpl<iterator_type>::setContextStack(const Db::ContextStack &stack);

template void ParserImpl<iterator_type>::clearContextStack();

template void ParserImpl<iterator_type>::addKindAttributes(const Db::Identifier &kindName, AttributesParser<iterator_type> *attributesParser, AttributeRemovalsParser<iterator_type> *attributeRemovalsParser);

template void ParserImpl<iterator_type>::addNestedKinds(const Db::Identifier &kindName, KindsOnlyParser<iterator_type> *kindsOnlyParser);

template bool ParserImpl<iterator_type>::parseLineImpl(const std::string &line);

template void ParserImpl<iterator_type>::reportParseError(const std::string& line);

template void ParserImpl<iterator_type>::insertTabPossibilitiesOfCurrentContext(const std::string &line, std::vector<std::string> &possibilities);

template void ParserImpl<iterator_type>::insertTabPossibilitiesFromErrors(const std::string &line, std::vector<std::string> &possibilities);

}
}
