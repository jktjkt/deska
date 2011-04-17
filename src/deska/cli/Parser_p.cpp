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
    using qi::rethrow;

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
    on_error<rethrow>(start, attributeErrorHandler(_1, _2, _3, _4,
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
    using qi::rethrow;

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
                                           KindsOnlyParser<Iterator> *nestedKinds, ParserImpl<Iterator> *parent):
    WholeKindParser<Iterator>::base_type(start), m_parent(parent)
{
    // If the boost::spirit::qi::grammar API was sane, the following line would read setName(kindName).
    // The API is not sane, and therefore we have the following crap here.
    this->name(kindName);

    start = ((+(*attributesParser) >> -(*nestedKinds))
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
           | (qi::lit("show")[phoenix::bind(&FunctionWordsParser::actionShow, this)]));
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
        addKindAttributes(*it, attributesParsers[*it]);

        // Create nested kinds parser for the new kind
        kindsOnlyParsers[*it] = new KindsOnlyParser<Iterator>(*it, this);
        addNestedKinds(*it, kindsOnlyParsers[*it]);

        // And combine them in the parser for the whole kind
        wholeKindParsers[*it] = new WholeKindParser<Iterator>(
            *it, attributesParsers[*it], kindsOnlyParsers[*it], this);
    }
}



template <typename Iterator>
ParserImpl<Iterator>::~ParserImpl()
{
    for (typename std::map<std::string, AttributesParser<Iterator>* >::iterator it = attributesParsers.begin();
        it != attributesParsers.end(); ++it) {
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
void ParserImpl<Iterator>::parseLine(const std::string &line)
{
    dryRun = false;
    parseLineImpl(line);
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
std::vector<std::string> ParserImpl<Iterator>::tabCompletitionPossibilities(const std::string &line)
{
    // FIXME: return correct result
    dryRun = true;
    if (parseLineImpl(line))
        return std::vector<std::string>();

    return std::vector<std::string>();
}



template <typename Iterator>
void ParserImpl<Iterator>::categoryEntered(const Db::Identifier &kind, const Db::Identifier &name)
{
    contextStack.push_back(ContextStackItem(kind, name));
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
std::vector<Db::Identifier> ParserImpl<Iterator>::getKindNames()
{
    return m_parser->m_dbApi->kindNames();
}



template <typename Iterator>
void ParserImpl<Iterator>::clearContextStack()
{
    contextStack.clear();
}



template <typename Iterator>
void ParserImpl<Iterator>::addKindAttributes(const Db::Identifier &kindName,
                                             AttributesParser<Iterator>* attributesParser)
{
    std::vector<Db::KindAttributeDataType> attributes = m_parser->m_dbApi->kindAttributes(kindName);
    for (std::vector<Db::KindAttributeDataType>::iterator it = attributes.begin(); it != attributes.end(); ++it) {
        attributesParser->addAtrribute(it->name, predefinedRules->getRule(it->type));
    }
}



template <typename Iterator>
void ParserImpl<Iterator>::addNestedKinds(const Db::Identifier &kindName, KindsOnlyParser<Iterator>* kindsOnlyParser)
{
    std::vector<Db::Identifier> kinds = m_parser->m_dbApi->kindNames();
    for (std::vector<Db::Identifier>::iterator it = kinds.begin(); it != kinds.end(); ++it) {
        std::vector<Db::ObjectRelation> relations = m_parser->m_dbApi->kindRelations(*it);
        for (std::vector<Db::ObjectRelation>::iterator itr = relations.begin(); itr != relations.end(); ++itr) {
            if ((itr->kind == Db::RELATION_EMBED_INTO) && (itr->targetTableName == kindName)) {
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
#endif

    Iterator iter = line.begin();
    Iterator end = line.end(); 

    parseErrors.clear();

    parsingMode = PARSING_MODE_STANDARD;
    bool parsingSucceeded = false;
    singleKind = false;
    bool topLevel = false;
    int parsingIterations = 0;
    bool functionWordParsed = false;
    std::vector<ContextStackItem>::size_type previousContextStackSize = contextStack.size();

    // Check if there are any function words at the beginning of the line.
    functionWordParsed = phrase_parse(iter, end, *functionWordsParser, ascii::space);

    // Function word "show" parsed alone
    if ((iter == end) && (parsingMode == PARSING_MODE_SHOW) && (functionWordParsed))
    {
#ifdef PARSER_DEBUG
            std::cout << "Action Show" << std::endl;
#endif
            if (!dryRun)
                m_parser->functionShow();
        return true;
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
                    parsingSucceeded = phrase_parse(iter, end, *(kindsOnlyParsers[contextStack.back().kind]),
                                                    ascii::space);
                    break;
                default:
                    throw std::domain_error("Invalid value of parsingMode");
            }        
        }

        if (!parsingSucceeded) {
            // Some bad input
            if (!dryRun)
                reportParseError(line);
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
            default:
                throw std::domain_error("Invalid value of parsingMode");
        } 
    }

    // Invoke categoryLeft signals when parsing in-line definitions
    if ((parsingMode == PARSING_MODE_STANDARD) && (singleKind || topLevel)) {
        // Definition of kind found stand-alone in standard mode on one line -> nest permanently
    } else {
        int depthDiff = contextStack.size() - previousContextStackSize;
        if (depthDiff > 0) {
            for (int i = 0; i < depthDiff; ++i) {
                categoryLeft();
            }
        }
    }

    return parsingSucceeded;
}



template <typename Iterator>
void ParserImpl<Iterator>::reportParseError(const std::string& line)
{
    // No more than three errors should occur. Three errors occur only when bad identifier of embedded object is set.
    BOOST_ASSERT(parseErrors.size() <= 3);
    // There have to be some ParseError when parsing fails.
    BOOST_ASSERT(parseErrors.size() != 0);

    // At first, find out if it's caused by a non-conforming data type. That would mean that it's caused
    // by an error in the attribute value
    typename std::vector<ParseError<Iterator> >::iterator it = std::find_if(
        parseErrors.begin(), parseErrors.end(),
        phoenix::bind(&ParseError<Iterator>::errorType, phoenix::arg_names::_1) == PARSE_ERROR_TYPE_VALUE_TYPE);
    if (it != parseErrors.end()) {
        // Yes, error in an attribute's value. That's all what's interesting for us, so let's ignore any other errors
        // which could be reported by spirit as a result of the error propagation.
#ifdef PARSER_DEBUG
        std::cout << it->toString() << std::endl;
#endif
        m_parser->parseError(InvalidAttributeDataTypeError(it->toString(), line, it->errorPosition()));
    } else {
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
}



/////////////////////////Template instances for linker//////////////////////////

template void RangeToString<iterator_type>::operator()(const boost::iterator_range<iterator_type> &rng, std::string &str) const;

template PredefinedRules<iterator_type>::PredefinedRules();

template const qi::rule<iterator_type, Db::Value(), ascii::space_type>& PredefinedRules<iterator_type>::getRule(const Db::Type attrType);

template const qi::rule<iterator_type, Db::Identifier(), ascii::space_type>& PredefinedRules<iterator_type>::getObjectIdentifier();

template AttributesParser<iterator_type>::AttributesParser(const Db::Identifier &kindName, ParserImpl<iterator_type> *parent);

template void AttributesParser<iterator_type>::addAtrribute(const Db::Identifier &attributeName,qi::rule<iterator_type, Db::Value(), ascii::space_type> attributeParser);

template void AttributesParser<iterator_type>::parsedAttribute(const Db::Identifier &parameter, Db::Value &value);

template KindsOnlyParser<iterator_type>::KindsOnlyParser(const Db::Identifier &kindName, ParserImpl<iterator_type> *parent);

template void KindsOnlyParser<iterator_type>::addKind(const Db::Identifier &kindName,qi::rule<iterator_type, Db::Identifier(), ascii::space_type> identifierParser);

template void KindsOnlyParser<iterator_type>::parsedKind(const Db::Identifier &kindName, const Db::Identifier &objectName);

template WholeKindParser<iterator_type>::WholeKindParser(const std::string &kindName, AttributesParser<iterator_type> *attributesParser, KindsOnlyParser<iterator_type> *nestedKinds, ParserImpl<iterator_type> *parent);

template void WholeKindParser<iterator_type>::parsedEnd();

template void WholeKindParser<iterator_type>::parsedSingleKind();

template FunctionWordsParser<iterator_type>::FunctionWordsParser(ParserImpl<iterator_type> *parent);

template void FunctionWordsParser<iterator_type>::actionDelete();

template void FunctionWordsParser<iterator_type>::actionShow();

template ParserImpl<iterator_type>::ParserImpl(Parser *parent);

template ParserImpl<iterator_type>::~ParserImpl();

template void ParserImpl<iterator_type>::parseLine(const std::string &line);

template bool ParserImpl<iterator_type>::isNestedInContext() const;

template std::vector<ContextStackItem> ParserImpl<iterator_type>::currentContextStack() const;

template std::vector<std::string> ParserImpl<iterator_type>::tabCompletitionPossibilities(const std::string &line);

template void ParserImpl<iterator_type>::categoryEntered(const Db::Identifier &kind, const Db::Identifier &name);

template void ParserImpl<iterator_type>::categoryLeft();

template void ParserImpl<iterator_type>::attributeSet(const Db::Identifier &name, const Db::Value &value);

template void ParserImpl<iterator_type>::parsedSingleKind();

template void ParserImpl<iterator_type>::addParseError(const ParseError<iterator_type> &error);

template void ParserImpl<iterator_type>::setParsingMode(ParsingMode mode);

template std::vector<Db::Identifier> ParserImpl<iterator_type>::getKindNames();

template void ParserImpl<iterator_type>::clearContextStack();

template void ParserImpl<iterator_type>::addKindAttributes(const Db::Identifier &kindName, AttributesParser<iterator_type>* attributesParser);

template void ParserImpl<iterator_type>::addNestedKinds(const Db::Identifier &kindName, KindsOnlyParser<iterator_type>* kindsOnlyParser);

template bool ParserImpl<iterator_type>::parseLineImpl(const std::string &line);

template void ParserImpl<iterator_type>::reportParseError(const std::string& line);

}
}
