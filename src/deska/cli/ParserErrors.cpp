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

#include <boost/spirit/include/phoenix_bind.hpp>

#include "ParserErrors.h"
#include "InfoExtractor.h"
#include "Parser_p.h"


namespace Deska
{
namespace Cli
{



template <typename Iterator>
void KindConstructErrorHandler<Iterator>::operator()(Iterator start, Iterator end, Iterator errorPos,
                              const spirit::info& what,
                              const qi::symbols<char, qi::rule<Iterator, ascii::space_type> > &kinds,
                              const Db::Identifier &kindName, ParserImpl<Iterator> *parser) const
{
    ParseError<Iterator> error(start, end, errorPos, what, kinds, kindName, PARSE_ERROR_TYPE_KINDS_CONSTRUCT);
    // Because of usage of eps rule in parser grammars, error handler could be invoked even though there is no error.
    // We have to check this case.
    if (error.valid())
        parser->addParseError(error);
}



template <typename Iterator>
void KindErrorHandler<Iterator>::operator()(Iterator start, Iterator end, Iterator errorPos, const spirit::info& what,
                              const qi::symbols<char, qi::rule<Iterator, Db::Identifier(), ascii::space_type> > &kinds,
                              const Db::Identifier &kindName, ParserImpl<Iterator> *parser) const
{
    ParseError<Iterator> error(start, end, errorPos, what, kinds, kindName, PARSE_ERROR_TYPE_KIND);
    // Because of usage of eps rule in parser grammars, error handler could be invoked even though there is no error.
    // We have to check this case. In case of function words "show" or "delete" for kind with no nested kinds,
    // empty expected tokens are allowed.
    if ((error.valid()) || (parser->getParsingMode() == PARSING_MODE_SHOW) || (parser->getParsingMode() == PARSING_MODE_DELETE))
        parser->addParseError(error);
}



template <typename Iterator>
void KindFiltersErrorHandler<Iterator>::operator()(Iterator start, Iterator end, Iterator errorPos,
                              const spirit::info& what,
                              const qi::symbols<char, qi::rule<Iterator, Db::Filter(), ascii::space_type> > &kinds,
                              const Db::Identifier &kindName, ParserImpl<Iterator> *parser) const
{
    ParseError<Iterator> error(start, end, errorPos, what, kinds, kindName, PARSE_ERROR_TYPE_KIND_FILTER);
    // Because of usage of eps rule in parser grammars, error handler could be invoked even though there is no error.
    // We have to check this case. In case of function words "show" or "delete" for kind with no nested kinds,
    // empty expected tokens are allowed.
    if ((error.valid()) || (parser->getParsingMode() == PARSING_MODE_SHOW) || (parser->getParsingMode() == PARSING_MODE_DELETE))
        parser->addParseError(error);
}



template <typename Iterator>
void KindSpecialFiltersErrorHandler<Iterator>::operator()(Iterator start, Iterator end, Iterator errorPos,
                              const spirit::info& what,
                              const qi::symbols<char, qi::rule<Iterator, ascii::space_type> > &kinds,
                              const Db::Identifier &kindName, ParserImpl<Iterator> *parser) const
{
    ParseError<Iterator> error(start, end, errorPos, what, kinds, kindName, PARSE_ERROR_TYPE_KIND_SPECIAL_FILTER);
    // Because of usage of eps rule in parser grammars, error handler could be invoked even though there is no error.
    // We have to check this case. In case of function words "show" or "delete" for kind with no nested kinds,
    // empty expected tokens are allowed.
    if ((error.valid()) || (parser->getParsingMode() == PARSING_MODE_SHOW) || (parser->getParsingMode() == PARSING_MODE_DELETE))
        parser->addParseError(error);
}



template <typename Iterator>
void NestingErrorHandler<Iterator>::operator()(Iterator start, Iterator end, Iterator errorPos,
                                               const spirit::info &what, const std::string &failingToken,
                                               const Db::Identifier &kindName, ParserImpl<Iterator> *parser) const
{
    std::vector<Db::Identifier> definedKinds = parser->getKindNames();
    typename std::vector<Db::Identifier>::iterator it = std::find(definedKinds.begin(), definedKinds.end(), failingToken);
    if (it != definedKinds.end()) {
        ParseError<Iterator> error(start, end, errorPos, what, failingToken, kindName, PARSE_ERROR_TYPE_NESTING);
        //parser->addParseError(error);
        // FIXME: Implement support in the rest of error handler
    }
}



template <typename Iterator>
void AttributeErrorHandler<Iterator>::operator()(Iterator start, Iterator end, Iterator errorPos,
                              const spirit::info& what,
                              const qi::symbols<char, qi::rule<Iterator, Db::Value(), ascii::space_type> > &attributes,
                              const Db::Identifier &kindName, ParserImpl<Iterator> *parser) const
{
    ParseError<Iterator> error(start, end, errorPos, what, attributes, kindName, PARSE_ERROR_TYPE_ATTRIBUTE);
    // Because of usage of eps rule in parser grammars, error handler could be invoked even though there is no error.
    // We have to check this case.
    if (error.valid())
        parser->addParseError(error);
}



template <typename Iterator>
void IdentifiersSetsErrorHandler<Iterator>::operator()(Iterator start, Iterator end, Iterator errorPos,
                              const spirit::info& what,
                              const qi::symbols<char, qi::rule<Iterator, Db::Identifier(), ascii::space_type> > &sets,
                              const Db::Identifier &kindName, ParserImpl<Iterator> *parser) const
{
    ParseError<Iterator> error(start, end, errorPos, what, sets, kindName, PARSE_ERROR_TYPE_IDENTIFIERS_SET);
    // Because of usage of eps rule in parser grammars, error handler could be invoked even though there is no error.
    // We have to check this case.
    if (error.valid())
        parser->addParseError(error);
}



template <typename Iterator>
void AttributeRemovalErrorHandler<Iterator>::operator()(Iterator start, Iterator end, Iterator errorPos,
                              const spirit::info& what,
                              const qi::symbols<char, qi::rule<Iterator, ascii::space_type> > &attributes,
                              const Db::Identifier &kindName, ParserImpl<Iterator> *parser) const
{
    ParseError<Iterator> error(start, end, errorPos, what, attributes, kindName, PARSE_ERROR_TYPE_ATTRIBUTE_REMOVAL);
    // Because of usage of eps rule in parser grammars, error handler could be invoked even though there is no error.
    // We have to check this case.
    if (error.valid())
        parser->addParseError(error);
}



template <typename Iterator>
void IdentifierErrorHandler<Iterator>::operator()(Iterator start, Iterator end, Iterator errorPos,
                                                  const spirit::info& what, const Db::Identifier &kindName,
                                                  const std::vector<Db::Identifier> &objectNames,
                                                  ParserImpl<Iterator> *parser) const
{
    // FIXME: Implement some check of existence of object name and generate error.
}



template <typename Iterator>
void ValueErrorHandler<Iterator>::operator()(Iterator start, Iterator end, Iterator errorPos, const spirit::info& what,
                                             const Db::Identifier &attributeName, ParserImpl<Iterator> *parser) const
{
    ParseError<Iterator> error(start, end, errorPos, what, attributeName, PARSE_ERROR_TYPE_VALUE_TYPE);
    // Because of usage of eps rule in parser grammars, error handler could be invoked even though there is no error.
    // We have to check this case.
    if (error.valid())
        parser->addParseError(error);
}



template <typename Iterator>
void ObjectNameErrorHandler<Iterator>::operator()(Iterator start, Iterator end, Iterator errorPos, const spirit::info& what,
                                                  const Db::Identifier &kindName, ParserImpl<Iterator> *parser) const
{
    ParseError<Iterator> error(start, end, errorPos, what, kindName, PARSE_ERROR_TYPE_OBJECT_NAME);
    // Because of usage of eps rule in parser grammars, error handler could be invoked even though there is no error.
    // We have to check this case.
    if (error.valid())
        parser->addParseError(error);
}



InfoExtractor::InfoExtractor(std::vector<Db::Identifier> *keywordsList, std::vector<std::string> *typesList):
    kList(keywordsList), tList(typesList)
{
}



void InfoExtractor::element(spirit::utf8_string const& tag, spirit::utf8_string const& value, int) const
{
    if (!value.empty())
        kList->push_back(value);
    else
        tList->push_back(tag);
}



std::string parseErrorTypeToString(const ParseErrorType errorType)
{
    switch (errorType) {
        case PARSE_ERROR_TYPE_KINDS_CONSTRUCT:
            return "kind name";
        case PARSE_ERROR_TYPE_KIND:
            return "kind name";
        case PARSE_ERROR_TYPE_KIND_NESTING:
            return "object name";
        case PARSE_ERROR_TYPE_KIND_FILTER:
            return "kind name in a filter";
        case PARSE_ERROR_TYPE_KIND_SPECIAL_FILTER:
            return "kind name in a special filter";
        case PARSE_ERROR_TYPE_NESTING:
            return "kind name";
        case PARSE_ERROR_TYPE_ATTRIBUTE:
            return "attribute name";
        case PARSE_ERROR_TYPE_IDENTIFIERS_SET:
            return "identifiers set name";
        case PARSE_ERROR_TYPE_ATTRIBUTE_REMOVAL:
            return "attribute name";
        case PARSE_ERROR_TYPE_VALUE_TYPE:
            return "argument value";
        case PARSE_ERROR_TYPE_OBJECT_NAME:
            return "object name";
        case PARSE_ERROR_TYPE_OBJECT_DEFINITION_NOT_FOUND:
            return "kind name";
        case PARSE_ERROR_TYPE_OBJECT_NOT_FOUND:
            return "object name";
        case PARSE_ERROR_TYPE_IDENTIFIER_NOT_FOUND:
            return "object identifier";
            break;
    }
    throw std::domain_error("ParseErrorType out of range");
}



template <typename Iterator>
ParseError<Iterator>::ParseError(Iterator start, Iterator end, Iterator errorPos, const spirit::info &what,
                              const qi::symbols<char, qi::rule<Iterator, Db::Identifier(), ascii::space_type> > &kinds,
                              const Db::Identifier &kindName, ParseErrorType parseErrorType):
    m_errorType(parseErrorType), m_start(start), m_end(end), m_errorPos(errorPos), m_context(kindName)
{
    using namespace boost::phoenix::arg_names;
    kinds.for_each(boost::phoenix::bind(&ParseError<Iterator>::extractKindName, this, arg1, arg2));
}



template <typename Iterator>
ParseError<Iterator>::ParseError(Iterator start, Iterator end, Iterator errorPos, const spirit::info &what,
                              const qi::symbols<char, qi::rule<Iterator, Db::Filter(), ascii::space_type> > &kinds,
                              const Db::Identifier &kindName, ParseErrorType parseErrorType):
    m_errorType(parseErrorType), m_start(start), m_end(end), m_errorPos(errorPos), m_context(kindName)
{
    using namespace boost::phoenix::arg_names;
    kinds.for_each(boost::phoenix::bind(&ParseError<Iterator>::extractKindFilterName, this, arg1, arg2));
}



template <typename Iterator>
ParseError<Iterator>::ParseError(Iterator start, Iterator end, Iterator errorPos, const spirit::info &what,
                                 const std::string &failingToken, const Db::Identifier &kindName,
                                 ParseErrorType parseErrorType):
    m_errorType(parseErrorType), m_start(start), m_end(end), m_errorPos(errorPos), m_context(kindName)
{
}



template <typename Iterator>
ParseError<Iterator>::ParseError(Iterator start, Iterator end, Iterator errorPos, const spirit::info &what,
                                 const qi::symbols<char, qi::rule<Iterator, Db::Value(), ascii::space_type> > &attributes,
                                 const Db::Identifier &kindName, ParseErrorType parseErrorType):
    m_errorType(parseErrorType), m_start(start), m_end(end), m_errorPos(errorPos), m_context(kindName)
{
    using namespace boost::phoenix::arg_names;
    attributes.for_each(boost::phoenix::bind(&ParseError<Iterator>::extractAttributeName, this, arg1, arg2));
}



template <typename Iterator>
ParseError<Iterator>::ParseError(Iterator start, Iterator end, Iterator errorPos, const spirit::info &what,
                                 const qi::symbols<char, qi::rule<Iterator, ascii::space_type> > &attributes,
                                 const Db::Identifier &kindName, ParseErrorType parseErrorType):
    m_errorType(parseErrorType), m_start(start), m_end(end), m_errorPos(errorPos), m_context(kindName)
{
    using namespace boost::phoenix::arg_names;
    attributes.for_each(boost::phoenix::bind(&ParseError<Iterator>::extractRemovedAttributeName, this, arg1, arg2));
}



template <typename Iterator>
ParseError<Iterator>::ParseError(Iterator start, Iterator end, Iterator errorPos,
                                 const spirit::info &what, const Db::Identifier &attributeName,
                                 ParseErrorType parseErrorType):
    m_errorType(parseErrorType), m_start(start), m_end(end), m_errorPos(errorPos),
    m_context(attributeName)
{
    InfoExtractor extractor(&m_expectedKeywords, &m_expectedTypes);
    spirit::basic_info_walker<InfoExtractor> walker(extractor, what.tag, 0);
    boost::apply_visitor(walker, what.value);
}



template <typename Iterator>
ParseError<Iterator>::ParseError(Iterator start, Iterator end, Iterator errorPos, const Db::Identifier &kindName,
                                 const std::vector<Db::Identifier> &expectedKinds, ParseErrorType parseErrorType):
    m_errorType(parseErrorType), m_start(start), m_end(end), m_errorPos(errorPos),
    m_context(kindName)
{
    for (std::vector<Db::Identifier>::const_iterator it = expectedKinds.begin(); it != expectedKinds.end(); ++it) {
        m_expectedKeywords.push_back(*it);
    }
}



template <typename Iterator>
ParseError<Iterator>::ParseError(Iterator start, Iterator end, Iterator errorPos, const Db::Identifier &kindName,
                                 const Db::Identifier &objectName, ParseErrorType parseErrorType):
    m_errorType(parseErrorType), m_start(start), m_end(end), m_errorPos(errorPos)
{
    std::ostringstream sout;
    sout << kindName << " " << objectName;
    m_context = sout.str();
}



template <typename Iterator>
ParseError<Iterator>::ParseError(Iterator start, Iterator end, Iterator errorPos, ParseErrorType parseErrorType):
    m_errorType(parseErrorType), m_start(start), m_end(end), m_errorPos(errorPos)
{
    m_context = "";
}



template <typename Iterator>
ParseError<Iterator>::ParseError(const Db::Identifier &kindName, const Db::Identifier &objectName,
                                 ParseErrorType parseErrorType):
    m_errorType(parseErrorType)
{
    std::ostringstream sout;
    sout << kindName << " " << objectName;
    m_context = sout.str();
}



template <typename Iterator>
ParseErrorType ParseError<Iterator>::errorType() const
{
    return m_errorType;
}



template <typename Iterator>
Iterator ParseError<Iterator>::errorPosition() const
{
    return m_errorPos;
}



template <typename Iterator>
std::vector<std::string> ParseError<Iterator>::expectedTypes() const
{
    return m_expectedTypes;
}



template <typename Iterator>
std::vector<Db::Identifier> ParseError<Iterator>::expectedKeywords() const
{
    return m_expectedKeywords;
}



template <typename Iterator>
std::string ParseError<Iterator>::context() const
{
    return m_context;
}



template <typename Iterator>
std::string ParseError<Iterator>::toString() const
{
    std::ostringstream sout;
    sout << "Error while parsing " << parseErrorTypeToString(m_errorType);
    switch (m_errorType) {
        case PARSE_ERROR_TYPE_KIND:
        case PARSE_ERROR_TYPE_KINDS_CONSTRUCT:
            if (m_context.empty())
                sout << ". Unknown top-level kind";
            else
                sout << " of nested object in " << m_context;
            break;
        case PARSE_ERROR_TYPE_KIND_NESTING:
            sout << " for " << m_context << ". Can't find nesting parents";
            break;
        case PARSE_ERROR_TYPE_KIND_FILTER:
        case PARSE_ERROR_TYPE_KIND_SPECIAL_FILTER:
            if (m_context.empty())
                sout << ". Unknown kind";
            else
                sout << " of nested object in " << m_context;
            break;
        case PARSE_ERROR_TYPE_NESTING:
            sout << " of nested object in " << m_context << ". Bad nesting";
            break;
        case PARSE_ERROR_TYPE_ATTRIBUTE:
        case PARSE_ERROR_TYPE_IDENTIFIERS_SET:
        case PARSE_ERROR_TYPE_ATTRIBUTE_REMOVAL:
        case PARSE_ERROR_TYPE_VALUE_TYPE:
        case PARSE_ERROR_TYPE_OBJECT_NAME:
            sout << " for " << m_context;
            break;
        case PARSE_ERROR_TYPE_OBJECT_DEFINITION_NOT_FOUND:
            sout << ". No object definition found";
            break;
        case PARSE_ERROR_TYPE_OBJECT_NOT_FOUND:
            sout << ". Object " << m_context << " does not exist";
            break;
        case PARSE_ERROR_TYPE_IDENTIFIER_NOT_FOUND:
            sout << ". Correct identifier not found or too much data entered";
            break;
    }
    sout << ".";
    if (!(m_expectedKeywords.empty()) || !(m_expectedTypes.empty()))
    {
        sout << " Expected one of [ ";
        for (std::vector<std::string>::const_iterator
            it = m_expectedKeywords.begin(); it != m_expectedKeywords.end(); ++it)
            sout << "\"" << *it << "\" ";
        for (std::vector<std::string>::const_iterator
            it = m_expectedTypes.begin(); it != m_expectedTypes.end(); ++it)
            sout << "<" << *it << "> ";
        sout << "].";
    }
    //sout << "] here: " << std::string(m_errorPos, m_end) << ".";
    return sout.str();
}



template <typename Iterator>
std::string ParseError<Iterator>::toCombinedString(const ParseError<Iterator> &kindError) const
{
    BOOST_ASSERT(m_errorType == PARSE_ERROR_TYPE_ATTRIBUTE);
    BOOST_ASSERT(kindError.errorType() == PARSE_ERROR_TYPE_KIND);
    std::ostringstream sout;
    sout << "Error while parsing attribute name or nested kind name for " << m_context << ". Expected one of [ ";
    for (std::vector<std::string>::const_iterator
        it = m_expectedKeywords.begin(); it != m_expectedKeywords.end(); ++it)
        sout << "\"" << *it << "\" ";
    std::vector<std::string> expectedKinds = kindError.expectedKeywords();
    for (std::vector<std::string>::const_iterator
        it = expectedKinds.begin(); it != expectedKinds.end(); ++it)
        sout << "\"" << *it << "\" ";
    sout << "].";
    //sout << "] here: " << std::string(m_errorPos, m_end) << ".";
    return sout.str();
}



template <typename Iterator>
bool ParseError<Iterator>::valid() const
{
    return (!m_expectedKeywords.empty() || !m_expectedTypes.empty());
}



template <typename Iterator>
void ParseError<Iterator>::extractKindName(const Db::Identifier &name,
                                           const qi::rule<Iterator, Db::Identifier(), ascii::space_type> &rule)
{
    m_expectedKeywords.push_back(name);
}



template <typename Iterator>
void ParseError<Iterator>::extractKindFilterName(const Db::Identifier &name,
                                                 const qi::rule<Iterator, Db::Filter(), ascii::space_type> &rule)
{
    m_expectedKeywords.push_back(name);
}



template <typename Iterator>
void ParseError<Iterator>::extractAttributeName(const Db::Identifier &name,
                                                const qi::rule<Iterator, Db::Value(), ascii::space_type> &rule)
{
    m_expectedKeywords.push_back(name);
}



template <typename Iterator>
void ParseError<Iterator>::extractRemovedAttributeName(const Db::Identifier &name,
                                                       const qi::rule<Iterator, ascii::space_type> &rule)
{
    m_expectedKeywords.push_back(name);
}



/////////////////////////Template instances for linker//////////////////////////

template void KindConstructErrorHandler<iterator_type>::operator()(iterator_type start, iterator_type end, iterator_type errorPos, const spirit::info &what, const qi::symbols<char, qi::rule<iterator_type, ascii::space_type> > &kinds, const Db::Identifier &kindName, ParserImpl<iterator_type>* parser) const;

template void KindErrorHandler<iterator_type>::operator()(iterator_type start, iterator_type end, iterator_type errorPos, const spirit::info &what, const qi::symbols<char, qi::rule<iterator_type, Db::Identifier(), ascii::space_type> > &kinds, const Db::Identifier &kindName, ParserImpl<iterator_type>* parser) const;

template void KindFiltersErrorHandler<iterator_type>::operator()(iterator_type start, iterator_type end, iterator_type errorPos, const spirit::info &what, const qi::symbols<char, qi::rule<iterator_type, Db::Filter(), ascii::space_type> > &kinds, const Db::Identifier &kindName, ParserImpl<iterator_type>* parser) const;

template void KindSpecialFiltersErrorHandler<iterator_type>::operator()(iterator_type start, iterator_type end, iterator_type errorPos, const spirit::info &what, const qi::symbols<char, qi::rule<iterator_type, ascii::space_type> > &kinds, const Db::Identifier &kindName, ParserImpl<iterator_type>* parser) const;

template void NestingErrorHandler<iterator_type>::operator()(iterator_type start, iterator_type end, iterator_type errorPos, const spirit::info &what, const std::string &failingToken, const Db::Identifier &kindName, ParserImpl<iterator_type> *parser) const;

template void AttributeErrorHandler<iterator_type>::operator()(iterator_type start, iterator_type end, iterator_type errorPos, const spirit::info &what, const qi::symbols<char, qi::rule<iterator_type, Db::Value(), ascii::space_type> > &attributes, const Db::Identifier &kindName, ParserImpl<iterator_type>* parser) const;

template void IdentifiersSetsErrorHandler<iterator_type>::operator()(iterator_type start, iterator_type end, iterator_type errorPos, const spirit::info &what, const qi::symbols<char, qi::rule<iterator_type, Db::Identifier(), ascii::space_type> > &sets, const Db::Identifier &kindName, ParserImpl<iterator_type>* parser) const;

template void AttributeRemovalErrorHandler<iterator_type>::operator()(iterator_type start, iterator_type end, iterator_type errorPos, const spirit::info &what, const qi::symbols<char, qi::rule<iterator_type, ascii::space_type> > &attributes, const Db::Identifier &kindName, ParserImpl<iterator_type>* parser) const;

template void IdentifierErrorHandler<iterator_type>::operator()(iterator_type start, iterator_type end, iterator_type errorPos, const spirit::info &what, const Db::Identifier &kindName, const std::vector<Db::Identifier> &objectNames, ParserImpl<iterator_type>* parser) const;

template void ValueErrorHandler<iterator_type>::operator()(iterator_type start, iterator_type end, iterator_type errorPos, const spirit::info &what, const Db::Identifier &attributeName, ParserImpl<iterator_type>* parser) const;

template void ObjectNameErrorHandler<iterator_type>::operator()(iterator_type start, iterator_type end, iterator_type errorPos, const spirit::info &what, const Db::Identifier &kindName, ParserImpl<iterator_type>* parser) const;

template ParseError<iterator_type>::ParseError(iterator_type start, iterator_type end, iterator_type errorPos, const spirit::info &what, const qi::symbols<char, qi::rule<iterator_type, Db::Identifier(), ascii::space_type> > &kinds, const Db::Identifier &kindName, ParseErrorType parseErrorType);

template ParseError<iterator_type>::ParseError(iterator_type start, iterator_type end, iterator_type errorPos, const spirit::info &what, const qi::symbols<char, qi::rule<iterator_type, Db::Filter(), ascii::space_type> > &kinds, const Db::Identifier &kindName, ParseErrorType parseErrorType);

template ParseError<iterator_type>::ParseError(iterator_type start, iterator_type end, iterator_type errorPos, const spirit::info &what, const std::string &failingToken, const Db::Identifier &kindName, ParseErrorType parseErrorType);

template ParseError<iterator_type>::ParseError(iterator_type start, iterator_type end, iterator_type errorPos, const spirit::info &what, const qi::symbols<char, qi::rule<iterator_type, Db::Value(), ascii::space_type> > &attributes, const Db::Identifier &kindName, ParseErrorType parseErrorType);

template ParseError<iterator_type>::ParseError(iterator_type start, iterator_type end, iterator_type errorPos, const spirit::info &what, const qi::symbols<char, qi::rule<iterator_type, ascii::space_type> > &attributes, const Db::Identifier &kindName, ParseErrorType parseErrorType);

template ParseError<iterator_type>::ParseError(iterator_type start, iterator_type end, iterator_type errorPos, const spirit::info &what, const Db::Identifier &attributeName, ParseErrorType parseErrorType);

template ParseError<iterator_type>::ParseError(iterator_type start, iterator_type end, iterator_type errorPos, const Db::Identifier &kindName, const std::vector<Db::Identifier> &expectedKinds, ParseErrorType parseErrorType);

template ParseError<iterator_type>::ParseError(iterator_type start, iterator_type end, iterator_type errorPos, const Db::Identifier &kindName, const Db::Identifier &objectName, ParseErrorType parseErrorType);

template ParseError<iterator_type>::ParseError(const Db::Identifier &kindName, const Db::Identifier &objectName, ParseErrorType parseErrorType);

template ParseError<iterator_type>::ParseError(iterator_type start, iterator_type end, iterator_type errorPos, ParseErrorType parseErrorType);

template ParseErrorType ParseError<iterator_type>::errorType() const;

template iterator_type ParseError<iterator_type>::errorPosition() const;

template std::vector<std::string> ParseError<iterator_type>::expectedTypes() const;

template std::vector<Db::Identifier> ParseError<iterator_type>::expectedKeywords() const;

template std::string ParseError<iterator_type>::context() const;

template std::string ParseError<iterator_type>::toString() const;

template std::string ParseError<iterator_type>::toCombinedString(const ParseError<iterator_type> &kindError) const;

template bool ParseError<iterator_type>::valid() const;

template void ParseError<iterator_type>::extractKindName(const Db::Identifier &name, const qi::rule<iterator_type, std::string(), ascii::space_type> &rule);

template void ParseError<iterator_type>::extractKindFilterName(const Db::Identifier &name, const qi::rule<iterator_type, Db::Filter(), ascii::space_type> &rule);

template void ParseError<iterator_type>::extractAttributeName(const Db::Identifier &name, const qi::rule<iterator_type, Db::Value(), ascii::space_type> &rule);

template void ParseError<iterator_type>::extractRemovedAttributeName(const Db::Identifier &name, const qi::rule<iterator_type, ascii::space_type> &rule);

}
}
