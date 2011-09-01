/*
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

#include <fstream>
#include <cstdlib>
#include <boost/algorithm/string/trim.hpp>
#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>

#include "InfoExtractor.h"
#include "CliCommands_Log.h"
#include "CliCommands_Log_p.h"
#include "UserInterface.h"
#include "UserInterfaceIOBase.h"
#include "DbInteraction.h"
#include "Exceptions.h"
#include "deska/db/JsonApi.h"
#include "RangeToString.h"
#include "Parser.h"


namespace Deska {
namespace Cli {



template <typename Iterator>
void LogAttributeErrorHandler<Iterator>::operator()(Iterator start, Iterator end, Iterator errorPos, const boost::spirit::info &what,
                    const qi::symbols<char, qi::rule<Iterator, Db::Identifier(), ascii::space_type> > &kinds,
                    const qi::symbols<char, qi::rule<Iterator, Db::MetadataValue(), ascii::space_type> > &metadatas,
                    Log *parent) const
{
    parent->reportParseError(
        LogFilterParseError<Iterator>(start, end, errorPos, what, kinds, metadatas, LOG_FILTER_PARSE_ERROR_TYPE_ATTRIBUTE));
}



template <typename Iterator>
void LogValueErrorHandler<Iterator>::operator()(Iterator start, Iterator end, Iterator errorPos, const boost::spirit::info &what,
                                                const Db::Identifier &metadataName, Log *parent) const
{
    parent->reportParseError(
        LogFilterParseError<Iterator>(start, end, errorPos, what, metadataName, LOG_FILTER_PARSE_ERROR_TYPE_VALUE_TYPE));
}



template <typename Iterator>
void LogIdentifierErrorHandler<Iterator>::operator()(Iterator start, Iterator end, Iterator errorPos, const boost::spirit::info &what,
                                                const Db::Identifier &kindName, Log *parent) const
{
    parent->reportParseError(
        LogFilterParseError<Iterator>(start, end, errorPos, what, kindName, LOG_FILTER_PARSE_ERROR_TYPE_IDENTIFIER));
}



template <typename Iterator>
LogFilterParseError<Iterator>::LogFilterParseError(Iterator start, Iterator end, Iterator errorPos, const boost::spirit::info &what,
                        const qi::symbols<char, qi::rule<Iterator, Db::Identifier(), ascii::space_type> > &kinds,
                        const qi::symbols<char, qi::rule<Iterator, Db::MetadataValue(), ascii::space_type> > &metadatas,
                        LogFilterParseErrorType logFilterParseErrorType):
    m_errorType(logFilterParseErrorType), m_start(start), m_end(end), m_errorPos(errorPos), m_context("")
{
    using namespace boost::phoenix::arg_names;
    kinds.for_each(boost::phoenix::bind(
        &LogFilterParseError<Iterator>::extractKindNames, this, arg1, arg2));
    metadatas.for_each(boost::phoenix::bind(
        &LogFilterParseError<Iterator>::extractMetadataNames, this, arg1, arg2));
}



template <typename Iterator>
LogFilterParseError<Iterator>::LogFilterParseError(Iterator start, Iterator end, Iterator errorPos, const boost::spirit::info &what,
                        const Db::Identifier &attributeName, LogFilterParseErrorType logFilterParseErrorType):
    m_errorType(logFilterParseErrorType), m_start(start), m_end(end), m_errorPos(errorPos), m_context(attributeName)
{
    InfoExtractor extractor(&m_expectedKeywords, &m_expectedTypes);
    boost::spirit::basic_info_walker<InfoExtractor> walker(extractor, what.tag, 0);
    boost::apply_visitor(walker, what.value);
}



template <typename Iterator>
void LogFilterParseError<Iterator>::extractKindNames(const Db::Identifier &name,
                                                  const qi::rule<Iterator, Db::Identifier(), ascii::space_type> &rule)
{
    m_expectedKeywords.push_back(name);
}



template <typename Iterator>
void LogFilterParseError<Iterator>::extractMetadataNames(const Db::Identifier &name,
                                                  const qi::rule<Iterator, Db::MetadataValue(), ascii::space_type> &rule)
{
    m_expectedKeywords.push_back(name);
}



template <typename Iterator>
LogFilterParseErrorType LogFilterParseError<Iterator>::errorType() const
{
    return m_errorType;
}



template <typename Iterator>
std::string LogFilterParseError<Iterator>::toString() const
{
    std::ostringstream ostr;
    ostr << "Error while parsing ";
    switch (m_errorType) {
        case LOG_FILTER_PARSE_ERROR_TYPE_ATTRIBUTE:
            ostr << "metadata name or kind name.";
            break;
        case LOG_FILTER_PARSE_ERROR_TYPE_VALUE_TYPE:
            ostr << "metadata value for " << m_context << ".";
            break;
        case LOG_FILTER_PARSE_ERROR_TYPE_IDENTIFIER:
            ostr << "object name for " << m_context << ".";
            break;
        default:
            throw std::domain_error("LogFilterParseErrorType out of range");
    }
    if (!(m_expectedKeywords.empty()) || !(m_expectedTypes.empty())) {
        ostr << " Expected one of [ ";
        for (std::vector<std::string>::const_iterator
            it = m_expectedKeywords.begin(); it != m_expectedKeywords.end(); ++it)
            ostr << "\"" << *it << "\" ";
        for (std::vector<std::string>::const_iterator
            it = m_expectedTypes.begin(); it != m_expectedTypes.end(); ++it)
            ostr << "<" << *it << "> ";
        ostr << "].";
    }
    ostr << " At offset " << static_cast<int>(m_errorPos - m_start) << " in the parameter.";
    return ostr.str();
}



template <typename Iterator>
LogFilterParser<Iterator>::LogFilterParser(Log *parent): LogFilterParser<Iterator>::base_type(start), m_parent(parent)
{
    using qi::_val;
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

    predefinedRules = new PredefinedRules<Iterator>();

    phoenix::function<RangeToString<Iterator> > rangeToString = RangeToString<Iterator>();

    // Fill symbols table with conversions from string to Db::ComparisonOperator
    operators.add("=", Db::FILTER_COLUMN_EQ);
    operators.add("==", Db::FILTER_COLUMN_EQ);
    operators.add("!=", Db::FILTER_COLUMN_NE);
    operators.add("<>", Db::FILTER_COLUMN_NE);
    operators.add(">", Db::FILTER_COLUMN_GT);
    operators.add(">=", Db::FILTER_COLUMN_GE);
    operators.add("<", Db::FILTER_COLUMN_LT);
    operators.add("<=", Db::FILTER_COLUMN_LE);

    // Fill symbols table with metadata names with their value types
    metadatas.add("revision", predefinedRules->getMetadataRule(METADATATYPE_REVISION_ID));
    metadatas.add("author", predefinedRules->getMetadataRule(METADATATYPE_AUTHOR));
    metadatas.add("message", predefinedRules->getMetadataRule(METADATATYPE_MESSAGE));
    metadatas.add("timestamp", predefinedRules->getMetadataRule(METADATATYPE_TIMESTAMP));

    start %= ((qi::lit("(") >> andFilter >> qi::lit(")"))
            | (qi::lit("(") >> orFilter >> qi::lit(")"))
            | (qi::lit("(") >> expr >> qi::lit(")")));

    andFilter = (start % qi::lit("&"))[_val = phoenix::construct<Db::AndFilter>(_1)];
    orFilter = (start % qi::lit("|"))[_val = phoenix::construct<Db::OrFilter>(_1)];

    expr %= kindExpr | metadataExpr;

    // When parsing some input using Nabialek trick, the rule, that is using the symbols table will not be entered when
    // the keyword is not found in the table. The eps is there to ensure, that the start rule will be entered every
    // time and so the error handler for bad keywords could be bound to it. The eoi rule is there to avoid the grammar
    // require more input on the end of the line, which is side effect of eps usage in this way.
    kindExpr %= (eps(!_a) > kindDispatch >> -eoi[_a = true]);
    metadataExpr %= (eps(!_a) > metadataDispatch >> -eoi[_a = true]);

    // Kind name recognized -> try to parse object name
    kindDispatch = (raw[kinds[_a = _1]][rangeToString(_1, phoenix::ref(currentKindName))] > operators[_b = _1]
        > lazy(_a)[_val = phoenix::construct<Db::AttributeExpression>(_b, phoenix::ref(currentKindName), 
            "name", phoenix::construct<Db::Value>(_1))]);
    // Metadata name recognized -> try to parse metadata value
    metadataDispatch = (raw[metadatas[_a = _1]][rangeToString(_1, phoenix::ref(currentMetadataName))] > operators[_b = _1]
        > lazy(_a)[_val = phoenix::construct<Db::MetadataExpression>(_b, phoenix::ref(currentMetadataName), _1)]);
    
    phoenix::function<LogAttributeErrorHandler<Iterator> > attributeErrorHandler = LogAttributeErrorHandler<Iterator>();
    phoenix::function<LogValueErrorHandler<Iterator> > valueErrorHandler = LogValueErrorHandler<Iterator>();
    phoenix::function<LogIdentifierErrorHandler<Iterator> > identifierErrorHandler = LogIdentifierErrorHandler<Iterator>();
    on_error<fail>(kindExpr,
        attributeErrorHandler(_1, _2, _3, _4, phoenix::ref(kinds), phoenix::ref(metadatas), m_parent));
    on_error<fail>(metadataExpr,
        attributeErrorHandler(_1, _2, _3, _4, phoenix::ref(kinds), phoenix::ref(metadatas), m_parent));
    on_error<fail>(kindDispatch,
        identifierErrorHandler(_1, _2, _3, _4, phoenix::ref(currentKindName), m_parent));
    on_error<fail>(metadataDispatch,
        valueErrorHandler(_1, _2, _3, _4, phoenix::ref(currentMetadataName), m_parent)); 
}



template <typename Iterator>
LogFilterParser<Iterator>::~LogFilterParser()
{
    delete predefinedRules;
}



template <typename Iterator>
void LogFilterParser<Iterator>::addKind(const Db::Identifier &kindName)
{
    kinds.add(kindName, predefinedRules->getObjectIdentifier());
}



Log::Log(UserInterface *userInterface): Command(userInterface)
{
    cmdName = "log";
    cmdUsage = "Command for operations with revisions and history. Without parameter shows list of revisions.";
    complPatterns.push_back("log");

    filterParser = new LogFilterParser<iterator_type>(this);
    std::vector<Db::Identifier> kinds = ui->m_dbInteraction->kindNames();
    for (std::vector<Db::Identifier>::iterator it = kinds.begin(); it != kinds.end(); ++it) {
        filterParser->addKind(*it);
    }
}



Log::~Log()
{
    delete filterParser;
}



void Log::operator()(const std::string &params)
{
    if (params.empty()) {
        std::vector<Db::RevisionMetadata> revisions = ui->m_dbInteraction->allRevisions();
        ui->io->printRevisions(revisions);
        return;
    }

    std::string::const_iterator iter = params.begin();
    std::string::const_iterator end = params.end();
    Db::Filter filter;
    parseErrors.clear();
    bool r = boost::spirit::qi::phrase_parse(iter, end, *filterParser, boost::spirit::ascii::space, filter);
    if (!r || (iter != end)) {
        std::vector<LogFilterParseError<iterator_type> >::iterator it;

        // Error in the attribute value
        it = std::find_if(parseErrors.begin(), parseErrors.end(), phoenix::bind(&LogFilterParseError<iterator_type>::errorType,
                          phoenix::arg_names::_1) == LOG_FILTER_PARSE_ERROR_TYPE_VALUE_TYPE);
        if (it != parseErrors.end()) {
            ui->io->reportError(it->toString());
            return;
        }
        // Error in attribute name in removal
        it = std::find_if(parseErrors.begin(), parseErrors.end(), phoenix::bind(&LogFilterParseError<iterator_type>::errorType,
                          phoenix::arg_names::_1) == LOG_FILTER_PARSE_ERROR_TYPE_ATTRIBUTE);
        if (it != parseErrors.end()) {
            ui->io->reportError(it->toString());
            return;
        }

        // Other error
        ui->io->reportError("Error while parsing filter for revisions. Check matching braces and operators.");
        return;
    }

    std::vector<Db::RevisionMetadata> revisions = ui->m_dbInteraction->filteredRevisions(filter);
    ui->io->printRevisions(revisions);
}



void Log::reportParseError(const LogFilterParseError<iterator_type> &error)
{
    parseErrors.push_back(error);
}


/////////////////////////Template instances for linker//////////////////////////

template void LogAttributeErrorHandler<iterator_type>::operator()(iterator_type start, iterator_type end, iterator_type errorPos, const boost::spirit::info &what, const qi::symbols<char, qi::rule<iterator_type, Db::Identifier(), ascii::space_type> > &kinds, const qi::symbols<char, qi::rule<iterator_type, Db::MetadataValue(), ascii::space_type> > &metadatas, Log *parent) const;

template void LogValueErrorHandler<iterator_type>::operator()(iterator_type start, iterator_type end, iterator_type errorPos, const boost::spirit::info &what, const Db::Identifier &metadataName, Log *parent) const;

template void LogIdentifierErrorHandler<iterator_type>::operator()(iterator_type start, iterator_type end, iterator_type errorPos, const boost::spirit::info &what, const Db::Identifier &kindName, Log *parent) const;

template LogFilterParseError<iterator_type>::LogFilterParseError(iterator_type start, iterator_type end, iterator_type errorPos, const boost::spirit::info &what, const qi::symbols<char, qi::rule<iterator_type, Db::Identifier(), ascii::space_type> > &kinds, const qi::symbols<char, qi::rule<iterator_type, Db::MetadataValue(), ascii::space_type> > &metadatas, LogFilterParseErrorType logFilterParseErrorType);

template LogFilterParseError<iterator_type>::LogFilterParseError(iterator_type start, iterator_type end, iterator_type errorPos, const boost::spirit::info &what, const Db::Identifier &attributeName, LogFilterParseErrorType logFilterParseErrorType);

template void LogFilterParseError<iterator_type>::extractKindNames(const Db::Identifier &name, const qi::rule<iterator_type, Db::Identifier(), ascii::space_type> &rule);

template void LogFilterParseError<iterator_type>::extractMetadataNames(const Db::Identifier &name, const qi::rule<iterator_type, Db::MetadataValue(), ascii::space_type> &rule);

template LogFilterParseErrorType LogFilterParseError<iterator_type>::errorType() const;

template std::string LogFilterParseError<iterator_type>::toString() const;

template LogFilterParser<iterator_type>::LogFilterParser(Log *parent);

template LogFilterParser<iterator_type>::~LogFilterParser();

template void LogFilterParser<iterator_type>::addKind(const Db::Identifier &kindName);

}
}
