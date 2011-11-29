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
#include "Parser_p_AttributeRemovalsParser.h"
#include "Parser_p_AttributesSettingParser.h"
#include "Parser_p_IdentifiersSetsParser.h"
#include "Parser_p_AttributesParser.h"
#include "Parser_p_FunctionWordsParser.h"
#include "Parser_p_KindsOnlyParser.h"
#include "Parser_p_KindsConstructParser.h"
#include "PredefinedRules.h"
#include "Parser_p_FilterExpressionsParser.h"
#include "Parser_p_FiltersParser.h"
#include "Parser_p_KindsParser.h"
#include "Parser_p_KindsFiltersParser.h"
#include "Parser_p_WholeKindParser.h"
#include "Parser_p_TopLevelParser.h"
#include "deska/db/Api.h"
#include "deska/db/Filter.h"

//#define PARSER_DEBUG

namespace Deska
{
namespace Cli
{


template <typename Iterator>
ParserImpl<Iterator>::ParserImpl(Parser *parent): m_parser(parent)
{
    predefinedRules = new PredefinedRules<Iterator>();
    topLevelKindsParser = new KindsOnlyParser<Iterator>(std::string(""), this);
    topLevelKindsFiltersParser = new KindsFiltersParser<Iterator>(std::string(""), this);
    topLevelParser = new TopLevelParser<Iterator>(topLevelKindsParser, topLevelKindsFiltersParser, this);
    functionWordsParser = new FunctionWordsParser<Iterator>(this);

    allKinds = m_parser->m_dbApi->kindNames();
    bool isEmbedded = false;
    for (std::vector<Db::Identifier>::iterator itk = allKinds.begin(); itk != allKinds.end(); ++itk) {
        std::vector<Db::ObjectRelation> relations = m_parser->m_dbApi->kindRelations(*itk);
        isEmbedded = false;
        for (std::vector<Db::ObjectRelation>::iterator itr = relations.begin(); itr != relations.end(); ++itr) {
            if (itr->kind == Db::RELATION_EMBED_INTO) {
                isEmbedded = true;
                embeds[itr->target].push_back(*itk);
                embeddedInto[*itk] = std::make_pair<Db::Identifier, Db::Identifier>(itr->column, itr->target);
            }

            if (itr->kind == Db::RELATION_CONTAINS) {
                contains[*itk].push_back(std::make_pair<Db::Identifier, Db::Identifier>(itr->column, itr->target));
                roAttributes[*itk].push_back(itr->column);
            }

            if (itr->kind == Db::RELATION_CONTAINABLE) {
                containable[*itk].push_back(std::make_pair<Db::Identifier, Db::Identifier>(itr->column, itr->target));
                roAttributes[*itk].push_back(itr->column);
            }

            if (itr->kind == Db::RELATION_REFERS_TO) {
                refersTo[*itk].push_back(std::make_pair<Db::Identifier, Db::Identifier>(itr->column, itr->target));
                referredBy[itr->target].push_back(*itk);
            }

            if (itr->kind == Db::RELATION_TEMPLATIZED) {
                templatized[*itk].push_back(std::make_pair<Db::Identifier, Db::Identifier>(itr->column, itr->target));
                templateFor[itr->target].push_back(*itk);
            }
        }
        if (!isEmbedded)
            topLevelKindsIds.push_back(*itk);
    }

    for (std::map<Db::Identifier, std::pair<Db::Identifier, Db::Identifier> >::iterator it = embeddedInto.begin(); it != embeddedInto.end(); ++it) {
        embeddedIntoInclContaining[it->first].push_back(it->second.second);
        for (std::vector<std::pair<Db::Identifier, Db::Identifier> >::iterator itm = contains[it->second.second].begin(); itm != contains[it->second.second].end(); ++itm) {
            embeddedIntoInclContaining[it->first].push_back(itm->second);
        }
    }

    // Create attributes parser and filters expressions parser for each kind
    for (std::vector<Db::Identifier>::iterator it = allKinds.begin(); it != allKinds.end(); ++it) {
        attributesSettingParsers[*it] = new AttributesSettingParser<Iterator>(*it, this);
        attributeRemovalsParsers[*it] = new AttributeRemovalsParser<Iterator>(*it, this);
        identifiersSetsParsers[*it] = new IdentifiersSetsParser<Iterator>(*it, this);
        filterExpressionsParsers[*it] = new FilterExpressionsParser<Iterator>(*it, this);
        addKindAttributes(*it, attributesSettingParsers[*it], attributeRemovalsParsers[*it],
                          identifiersSetsParsers[*it], filterExpressionsParsers[*it]);
        attributesParsers[*it] = new AttributesParser<Iterator>(*it, attributesSettingParsers[*it],
                                                                attributeRemovalsParsers[*it], 
                                                                identifiersSetsParsers[*it], this);
    }

    // Create filters parser for each kind
    for (std::vector<Db::Identifier>::iterator it = allKinds.begin(); it != allKinds.end(); ++it) {
        filtersParsers[*it] = new FiltersParser<Iterator>(*it, filterExpressionsParsers[*it], this);
        addNestedKinds(*it, filtersParsers[*it]);
    }

    // Add new kind to the top-level parser if it is not embedded anywhere
    for (std::vector<Db::Identifier>::iterator it = allKinds.begin(); it != allKinds.end(); ++it) {
        topLevelKindsParser->addKind(*it, predefinedRules->getObjectIdentifier());
    }

    for (std::vector<Db::Identifier>::iterator it = allKinds.begin(); it != allKinds.end(); ++it) {
        // Add filters parser
        topLevelKindsFiltersParser->addKindFilter(*it, filtersParsers[*it]);

        // Create nested kinds parser and nested kinds filters parser for the new kind
        kindsOnlyParsers[*it] = new KindsOnlyParser<Iterator>(*it, this);
        kindsFiltersParsers[*it] = new KindsFiltersParser<Iterator>(*it, this);
        kindsConstructParsers[*it] = new KindsConstructParser<Iterator>(*it, this);
        addNestedKinds(*it, kindsOnlyParsers[*it], kindsFiltersParsers[*it], kindsConstructParsers[*it]);
        kindsParsers[*it] = new KindsParser<Iterator>(*it, kindsOnlyParsers[*it], kindsFiltersParsers[*it],
                                                      kindsConstructParsers[*it], this);

        // And combine them in the parser for the whole kind
        wholeKindParsers[*it] = new WholeKindParser<Iterator>(*it, attributesParsers[*it], kindsParsers[*it], this);
    }
}



template <typename Iterator>
ParserImpl<Iterator>::~ParserImpl()
{
    for (typename std::map<std::string, AttributesSettingParser<Iterator>* >::iterator it = attributesSettingParsers.begin();
        it != attributesSettingParsers.end(); ++it) {
        delete it->second;
    }
    for (typename std::map<std::string, AttributeRemovalsParser<Iterator>* >::iterator it = attributeRemovalsParsers.begin();
        it != attributeRemovalsParsers.end(); ++it) {
        delete it->second;
    }
    for (typename std::map<std::string, IdentifiersSetsParser<Iterator>* >::iterator it = identifiersSetsParsers.begin();
        it != identifiersSetsParsers.end(); ++it) {
        delete it->second;
    }
    for (typename std::map<std::string, AttributesParser<Iterator>* >::iterator it = attributesParsers.begin();
        it != attributesParsers.end(); ++it) {
        delete it->second;
    }
    for (typename std::map<std::string, FilterExpressionsParser<Iterator>* >::iterator it = filterExpressionsParsers.begin();
        it != filterExpressionsParsers.end(); ++it) {
        delete it->second;
    }
    for (typename std::map<std::string, FiltersParser<Iterator>* >::iterator it = filtersParsers.begin();
        it != filtersParsers.end(); ++it) {
        delete it->second;
    }
    for (typename std::map<std::string, KindsFiltersParser<Iterator>* >::iterator it = kindsFiltersParsers.begin();
        it != kindsFiltersParsers.end(); ++it) {
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
    for (typename std::map<std::string, KindsConstructParser<Iterator>* >::iterator it = kindsConstructParsers.begin();
        it != kindsConstructParsers.end(); ++it) {
        delete it->second;
    }
    for (typename std::map<std::string, KindsParser<Iterator>* >::iterator it = kindsParsers.begin();
        it != kindsParsers.end(); ++it) {
        delete it->second;
    }

    delete topLevelParser;
    delete topLevelKindsParser;
    delete topLevelKindsFiltersParser;
    delete functionWordsParser;
    delete predefinedRules;
}



template <typename Iterator>
std::map<std::string, std::string> ParserImpl<Iterator>::parserKeywordsUsage()
{
    std::map<std::string, std::string> usages;
    usages["create"] = "Creates object given as parameter (e.g. create hardware hp456). Longer parameters are also allowed (e.g. create host golias120 interface eth0) This will create both objects.";
    usages["new"] = "Creates a new object of kind given as parameter. Name will be generated (e.g. new failure).";
    usages["last"] = "Selects an object of given kind with the highest numerical name (e.g. last failure).";
    usages["all"] = "Selects all objects of given kind (e.g. all host).";
    usages["delete"] = "Deletes object given as parameter (e.g. delete hardware hp456). Longer parameters are also allowed (e.g. delete host golias120 interface eth0) This will delete only interface eth0 in the object host golias120.";
    usages["show"] = "Shows attributes and nested kinds of the object. Parameter is here optional and works in the same way as for delete. When executed without parameter at top-level, it shows all object kinds and names.";
    usages["end"] = "Leaves one level of current context.";
    usages["no"] = "When entered in front of an attribute name, it removes it's value.";
    usages["add"] = "Inserts an identifier into an identifier set (e.g. insert role www).";
    usages["remove"] = "Removes an identifier from an identifier set (e.g. remove role www).";
    usages["where"] = "Selects objects satisfying given filter. You can write filter like \"host where (expression)\". Here expression is some comparison of attributes or conjunction or disjunction between two expressions. Note, that each expression must be in braces. You can also access attributes of any connected object using dot. For example \"host where ((host_note == \"something\") & (interface.ip4 == 192.168.1.5))\". There are supported operators <, >, <=, >=, == and != for standard attributes and \"contains\" and \"not_contains\" for sets.";
    return usages;
}



template <typename Iterator>
std::vector<Db::Identifier> ParserImpl<Iterator>::parserKindsEmbeds(const Db::Identifier &kindName)
{
    std::map<Db::Identifier, std::vector<Db::Identifier> >::const_iterator it = embeds.find(kindName);
    if (it == embeds.end()) {
        return std::vector<Db::Identifier>();
    } else {
        return it->second;
    }
}



template <typename Iterator>
std::vector<std::pair<Db::Identifier, std::string> > ParserImpl<Iterator>::parserKindsAttributes(const Db::Identifier &kindName)
{
    std::vector<std::pair<Db::Identifier, std::string> > attrs;
    std::vector<Db::KindAttributeDataType> attributes = m_parser->m_dbApi->kindAttributes(kindName);
    for (std::vector<Db::KindAttributeDataType>::iterator ita = attributes.begin(); ita != attributes.end(); ++ita) {
        std::vector<Db::Identifier>::iterator itroat = std::find(roAttributes[kindName].begin(),
            roAttributes[kindName].end(), ita->name);
        if (itroat != roAttributes[kindName].end())
            attrs.push_back(std::make_pair<Db::Identifier, std::string>(ita->name, predefinedRules->getRule(ita->type).name() + " - read only"));
        else
            attrs.push_back(std::make_pair<Db::Identifier, std::string>(ita->name, predefinedRules->getRule(ita->type).name()));
    }
    return attrs;
}



template <typename Iterator>
void ParserImpl<Iterator>::parseLine(const std::string &line)
{
    m_parser->parsingStarted();
    dryRun = false;
    if (parseLineImpl(line)) {
        m_parser->parsingFinished();
#ifdef PARSER_DEBUG
        std::cout << "Parsing finished." << std::endl;
#endif
    }
}



template <typename Iterator>
bool ParserImpl<Iterator>::isNestedInContext() const
{
    return !contextStack.empty();
}



template <typename Iterator>
ContextStack ParserImpl<Iterator>::currentContextStack() const
{
    return contextStack;
}



template <typename Iterator>
std::vector<std::string> ParserImpl<Iterator>::tabCompletionPossibilities(const std::string &line)
{
    // We have to restore previous context stack
    ContextStack contextStackBackup = contextStack;
    std::vector<std::string> possibilities;
    if (line.empty()) {
        insertTabPossibilitiesOfCurrentContext(line, possibilities);
    } else {
        dryRun = true;
        bool parsingSucceeded;
        parsingSucceeded = parseLineImpl(line);
        if (parsingSucceeded) {
            if ((*(line.end() - 1) == ' ') || (*(line.end() - 1) == '\t') || (*(line.end() - 1) == '\n') ||
                (*(line.end() - 1) == '(') || (*(line.end() - 1) == ')')) {
                insertTabPossibilitiesOfCurrentContext(line, possibilities);
            } else {
                // This should not happen, because CliCompleter truncates the last uncomplete token
            }
        } else {
            if ((*(line.end() - 1) == ' ') || (*(line.end() - 1) == '\t') || (*(line.end() - 1) == '\n') ||
                (*(line.end() - 1) == '(') || (*(line.end() - 1) == ')')) {
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
void ParserImpl<Iterator>::newObject(const Db::Identifier &kind)
{
    contextStack.push_back(ContextStackItem(kind, Db::Identifier()));
    if (!dryRun)
        m_parser->createObject(kind, "");
#ifdef PARSER_DEBUG
    std::cout << "New object: " << kind << std::endl;
#endif
}



template <typename Iterator>
void ParserImpl<Iterator>::categoryEntered(const Db::Identifier &kind, const Db::Identifier &name)
{
    std::vector<Db::Identifier> objectNames = pathToVector(name);
    std::vector<std::pair<Db::Identifier, Db::Identifier> > objects;
    std::vector<Db::Identifier>::reverse_iterator it = objectNames.rbegin();
    objects.push_back(std::make_pair<Db::Identifier, Db::Identifier>(kind, *it));
    ++it;
    for (; it != objectNames.rend(); ++it) {
        std::map<Db::Identifier, std::pair<Db::Identifier, Db::Identifier> >::const_iterator emb = embeddedInto.find(objects.back().first);
        if (emb == embeddedInto.end()) {
            addParseError(ParseError<Iterator>(kind, name, PARSE_ERROR_TYPE_KIND_NESTING));
            parsingSucceededActions = false;
            return;
        }
        objects.push_back(std::make_pair<Db::Identifier, Db::Identifier>(emb->second.second, *it));
    }
    std::map<Db::Identifier, std::pair<Db::Identifier, Db::Identifier> >::const_iterator emb = embeddedInto.find(objects.back().first);
    if (emb != embeddedInto.end()) {
        if (contextStack.empty()) {
            addParseError(ParseError<Iterator>(kind, name, PARSE_ERROR_TYPE_KIND_NESTING));
            parsingSucceededActions = false;
            return;
        } else {
            std::vector<Db::Identifier>::const_iterator embcont = find(embeddedIntoInclContaining[objects.back().first].begin(),
                embeddedIntoInclContaining[objects.back().first].end(), contextStack.back().kind);
            if (embcont == embeddedIntoInclContaining[objects.back().first].end()){
                addParseError(ParseError<Iterator>(kind, name, PARSE_ERROR_TYPE_KIND_NESTING));
                parsingSucceededActions = false;
                return;
            }
        }
    }
    std::vector<std::pair<Db::Identifier, Db::Identifier> >::reverse_iterator ito;
    for (ito = objects.rbegin(); ito != objects.rend() - 1; ++ito)
    {
        contextStack.push_back(ContextStackItem(ito->first, ito->second));
        if (!dryRun)
            m_parser->categoryEntered(ito->first, ito->second);
#ifdef PARSER_DEBUG
        std::cout << "Category entered: " << ito->first << ": " << ito->second << std::endl;
#endif
    }
    contextStack.push_back(ContextStackItem(ito->first, ito->second));
    if (parsingMode == PARSING_MODE_CREATE) {
        if (!dryRun)
            m_parser->createObject(ito->first, ito->second);
#ifdef PARSER_DEBUG
        std::cout << "Create object: " << ito->first << ": " << ito->second << std::endl;
#endif
    } else {
        if (!dryRun)
            m_parser->categoryEntered(ito->first, ito->second);
#ifdef PARSER_DEBUG
        std::cout << "Category entered: " << ito->first << ": " << ito->second << std::endl;
#endif
    }
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
void ParserImpl<Iterator>::attributeSet(const Db::Identifier &kind, const Db::Identifier &name, const Db::Value &value)
{
    if (!dryRun)
        m_parser->attributeSet(kind, name, value);
#ifdef PARSER_DEBUG
    std::cout << "Set attribute: " << kind << " - " << name << "=" << *value << std::endl;
#endif
}



template <typename Iterator>
void ParserImpl<Iterator>::attributeSetInsert(const Db::Identifier &kind, const Db::Identifier &name, const Db::Identifier &value)
{
    if (!dryRun)
        m_parser->attributeSetInsert(kind, name, value);
#ifdef PARSER_DEBUG
    std::cout << "Inserting into set: " << kind << " - " << name << ": " << value << std::endl;
#endif
}



template <typename Iterator>
void ParserImpl<Iterator>::attributeSetRemove(const Db::Identifier &kind, const Db::Identifier &name, const Db::Identifier &value)
{
    if (!dryRun)
        m_parser->attributeSetRemove(kind, name, value);
#ifdef PARSER_DEBUG
    std::cout << "Removing from set: " << kind << " - " << name << ": " << value << std::endl;
#endif
}



template <typename Iterator>
void ParserImpl<Iterator>::attributeRemove(const Db::Identifier &kind, const Db::Identifier &name)
{
    if (!dryRun)
        m_parser->attributeRemove(kind, name);
#ifdef PARSER_DEBUG
    std::cout << "Remove attribute: " << kind << " - " << name << std::endl;
#endif
}



template <typename Iterator>
void ParserImpl<Iterator>::objectsFilter(const Db::Identifier &kind, const boost::optional<Db::Filter> &filter)
{
    contextStack.push_back(ContextStackItem(kind, filter));
    if (!dryRun)
        m_parser->objectsFilter(kind, filter);
#ifdef PARSER_DEBUG
    if (filter)
        std::cout << "Objects filter: " << kind << ": " << *filter << std::endl;
    else
        std::cout << "Objects filter: " << kind << ": *" << std::endl;
#endif
}



template <typename Iterator>
void ParserImpl<Iterator>::parsedSingleKind()
{
#ifdef PARSER_DEBUG
    std::cout << "Single kind" << std::endl;
#endif
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
    return allKinds;
}



template <typename Iterator>
void ParserImpl<Iterator>::setContextStack(const ContextStack &stack)
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
                                             AttributesSettingParser<Iterator> *attributesSettingParser,
                                             AttributeRemovalsParser<Iterator> *attributeRemovalsParser,
                                             IdentifiersSetsParser<Iterator> *identifiersSetsParser,
                                             FilterExpressionsParser<Iterator> *filterExpressionsParser)
{
    // Adding own attributes
    std::vector<Db::KindAttributeDataType> attributes = m_parser->m_dbApi->kindAttributes(kindName);
    for (std::vector<Db::KindAttributeDataType>::iterator it = attributes.begin(); it != attributes.end(); ++it) {
        std::vector<Db::Identifier>::iterator itroat = std::find(roAttributes[kindName].begin(),
            roAttributes[kindName].end(), it->name);
        if (itroat == roAttributes[kindName].end()) {
            attributesSettingParser->addAtrribute(kindName, it->name, predefinedRules->getRule(it->type));
            attributeRemovalsParser->addAtrribute(kindName, it->name);
        }
        if (it->type == Db::TYPE_IDENTIFIER_SET) {
            identifiersSetsParser->addIdentifiersSet(kindName, it->name, predefinedRules->getObjectIdentifier());
            filterExpressionsParser->addIdentifiersSetToFilter(kindName, it->name, predefinedRules->getObjectIdentifier());
        } else {
            filterExpressionsParser->addAtrributeToFilter(kindName, it->name, predefinedRules->getRule(it->type));
        }
    }
    // Adding attributes from contained kinds
    for (std::vector<std::pair<Db::Identifier, Db::Identifier> >::iterator itm = contains[kindName].begin(); itm != contains[kindName].end(); ++itm) {
        std::vector<Db::KindAttributeDataType> attributes = m_parser->m_dbApi->kindAttributes(itm->second);
        for (std::vector<Db::KindAttributeDataType>::iterator it = attributes.begin(); it != attributes.end(); ++it) {
            std::vector<Db::Identifier>::iterator itroat = std::find(roAttributes[itm->second].begin(),
                roAttributes[itm->second].end(), it->name);
            if (itroat == roAttributes[itm->second].end()) {
                attributesSettingParser->addAtrribute(itm->second, it->name, predefinedRules->getRule(it->type));
                attributeRemovalsParser->addAtrribute(itm->second, it->name);
            } 
            if (it->type == Db::TYPE_IDENTIFIER_SET) {
                identifiersSetsParser->addIdentifiersSet(itm->second, it->name, predefinedRules->getObjectIdentifier());
                filterExpressionsParser->addIdentifiersSetToFilter(itm->second, it->name, predefinedRules->getObjectIdentifier());
            } else {
                filterExpressionsParser->addAtrributeToFilter(itm->second, it->name, predefinedRules->getRule(it->type));
            }
        }
    }
    // Adding attribute "name" for filters
    filterExpressionsParser->addAtrributeToFilter(kindName, "name", predefinedRules->getRule(Db::TYPE_IDENTIFIER));
}



template <typename Iterator>
void ParserImpl<Iterator>::addNestedKinds(const Db::Identifier &kindName, KindsOnlyParser<Iterator> *kindsOnlyParser,
                                          KindsFiltersParser<Iterator> *kindsFiltersParser,
                                          KindsConstructParser<Iterator> *kindsConstructParser)
{
    // Adding directly embedded kinds
    for (std::vector<Db::Identifier>::iterator it = embeds[kindName].begin(); it != embeds[kindName].end(); ++it) {
        kindsFiltersParser->addKindFilter(*it, filtersParsers[*it]);
        kindsConstructParser->addKind(*it);
    }

    // Adding recursively embedded kinds
    std::vector<Db::Identifier> nestedKinds = parserKindsEmbedsRecursively(kindName);
    for (std::vector<Db::Identifier>::iterator it = nestedKinds.begin(); it != nestedKinds.end(); ++it) {
        kindsOnlyParser->addKind(*it, predefinedRules->getObjectIdentifier());
    }

    // Adding kinds from directly contained kinds
    for (std::vector<std::pair<Db::Identifier, Db::Identifier> >::iterator itm = contains[kindName].begin(); itm != contains[kindName].end(); ++itm) {
        for (std::vector<Db::Identifier>::iterator it = embeds[itm->second].begin(); it != embeds[itm->second].end(); ++it) {
            kindsConstructParser->addKind(*it);
        }
    }

    // Adding kinds from recursively contained kinds
    for (std::vector<std::pair<Db::Identifier, Db::Identifier> >::iterator itm = contains[kindName].begin(); itm != contains[kindName].end(); ++itm) {
        std::vector<Db::Identifier> nestedKinds = parserKindsEmbedsRecursively(itm->second);
        for (std::vector<Db::Identifier>::iterator it = nestedKinds.begin(); it != nestedKinds.end(); ++it) {
            kindsOnlyParser->addKind(*it, predefinedRules->getObjectIdentifier());
        }
    }
}



template <typename Iterator>
void ParserImpl<Iterator>::addNestedKinds(const Db::Identifier &kindName, FiltersParser<Iterator> *filtersParser)
{
    // Adding embedded kinds
    std::vector<Db::Identifier> nestedKinds = parserKindsEmbeds(kindName);
    for (std::vector<Db::Identifier>::iterator it = nestedKinds.begin(); it != nestedKinds.end(); ++it) {
        filtersParser->addNestedKindExpressionsParser(*it, filterExpressionsParsers[*it]);
    }

    // Adding embedding kind
    std::map<Db::Identifier, std::pair<Db::Identifier, Db::Identifier> >::const_iterator i = embeddedInto.find(kindName);
    if (i != embeddedInto.end()) {
        filtersParser->addNestedKindExpressionsParser(i->second.second, filterExpressionsParsers[i->second.second]);
    }

    // Adding contained kinds
    for (std::vector<std::pair<Db::Identifier, Db::Identifier> >::iterator it = contains[kindName].begin(); it != contains[kindName].end(); ++it) {
        filtersParser->addNestedKindExpressionsParser(it->second, filterExpressionsParsers[it->second]);
    }

    // Adding contained parent kinds
    for (std::vector<std::pair<Db::Identifier, Db::Identifier> >::iterator it = containable[kindName].begin(); it != containable[kindName].end(); ++it) {
        filtersParser->addNestedKindExpressionsParser(it->second, filterExpressionsParsers[it->second]);
    }

    // Adding referring kinds
    for (std::vector<std::pair<Db::Identifier, Db::Identifier> >::iterator it = refersTo[kindName].begin(); it != refersTo[kindName].end(); ++it) {
        filtersParser->addNestedKindExpressionsParser(it->second, filterExpressionsParsers[it->second]);
    }

    // Adding referred kinds
    for (std::vector<Db::Identifier>::iterator it = referredBy[kindName].begin(); it != referredBy[kindName].end(); ++it) {
        filtersParser->addNestedKindExpressionsParser(*it, filterExpressionsParsers[*it]);
    }

    // Adding templating kinds
    for (std::vector<Db::Identifier>::iterator it = templateFor[kindName].begin(); it != templateFor[kindName].end(); ++it) {
        filtersParser->addNestedKindExpressionsParser(*it, filterExpressionsParsers[*it]);
    }

    // Adding templated kinds
    for (std::vector<std::pair<Db::Identifier, Db::Identifier> >::iterator it = templatized[kindName].begin(); it != templatized[kindName].end(); ++it) {
        filtersParser->addNestedKindExpressionsParser(it->second, filterExpressionsParsers[it->second]);
    }
}



template <typename Iterator>
bool ParserImpl<Iterator>::parseLineImpl(const std::string &line)
{
#ifdef PARSER_DEBUG
    std::cout << "Parse line: \"" << line << "\"" << std::endl;
    std::cout << "Start context stack: " << contextStackToString(contextStack) << std::endl;
#endif

    Iterator iter = line.begin();
    Iterator end = line.end(); 

    parseErrors.clear();

    parsingMode = PARSING_MODE_STANDARD;
    bool parsingSucceeded = true;
    parsingSucceededActions = true;
    singleKind = false;
    int parsingIterations = 0;
    bool functionWordParsed = false;
    bool nonexistantObject = false;
    ContextStack::size_type previousContextStackSize = contextStack.size();

    // Check if there are any function words at the beginning of the line.
    functionWordParsed = phrase_parse(iter, end, *functionWordsParser, ascii::space);

    // Function word parsed alone
    if ((iter == end) && (functionWordParsed)) {
        switch (parsingMode) {
            case PARSING_MODE_SHOW:
                // Function show does not require any parameters -> emit signals
#ifdef PARSER_DEBUG
                std::cout << "Action Show" << std::endl;
                std::cout << "End context stack: " << contextStackToString(contextStack) << std::endl;
#endif
                if (!dryRun)
                    m_parser->functionShow();
                return true;
                break;
            case PARSING_MODE_CREATE:
            case PARSING_MODE_DELETE:
            case PARSING_MODE_RENAME:
                // Function delete and rename require parameter -> report error
                if (contextStack.empty()) {
                    addParseError(ParseError<Iterator>(line.begin(), end, iter, "", allKinds,
                                  PARSE_ERROR_TYPE_OBJECT_DEFINITION_NOT_FOUND));
                    parsingSucceeded = false;
                } else {
                    addParseError(ParseError<Iterator>(line.begin(), end, iter,
                                  contextStack.back().kind, parserKindsEmbedsRecursively(contextStack.back().kind),
                                  PARSE_ERROR_TYPE_OBJECT_DEFINITION_NOT_FOUND));
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
            parsingSucceeded = phrase_parse(iter, end, *topLevelParser, ascii::space);
        } else {
            // Context -> parse attributes or nested kinds
            switch (parsingMode) {
                case PARSING_MODE_STANDARD:
                    parsingSucceeded = phrase_parse(iter, end, *(wholeKindParsers[contextStack.back().kind]),
                                                    ascii::space);
                    break;
                case PARSING_MODE_CREATE:
                case PARSING_MODE_DELETE:
                case PARSING_MODE_SHOW:
                case PARSING_MODE_RENAME:
                    parsingSucceeded = phrase_parse(iter, end, *(kindsParsers[contextStack.back().kind]),
                                                    ascii::space);
                    break;
                default:
                    throw std::domain_error("Invalid value of parsingMode");
            }        
        }

        // Check for existence of parsed kind instance and add parse error based on parsing mode.
        if (!contextStack.empty() && parsingSucceeded && parsingSucceededActions) {
            std::vector<Db::Identifier> instances;
            switch (parsingMode) {
                case PARSING_MODE_STANDARD:
                case PARSING_MODE_CREATE:
                    break;
                case PARSING_MODE_DELETE:
                case PARSING_MODE_SHOW:
                case PARSING_MODE_RENAME:
                    // Modes SHOW, DELETE and RENAME requires existing kind instances.
                    if (!contextStack.back().filter) {
                        instances = m_parser->m_dbApi->kindInstances(contextStack.back().kind,
                            Db::Filter(Db::AttributeExpression(Db::FILTER_COLUMN_EQ, contextStack.back().kind,
                                "name", Db::Value(contextStackToPath(contextStack)))));
                        if (instances.empty()) {
                            addParseError(ParseError<Iterator>(line.begin(), end, iter - contextStack.back().name.size() - 1,
                                                               contextStack.back().kind, contextStack.back().name,
                                                               PARSE_ERROR_TYPE_OBJECT_NOT_FOUND));
                            parsingSucceeded = false;
                            nonexistantObject = true;
                        }
                    }
                    break;
                default:
                    throw std::domain_error("Invalid value of parsingMode");
            }
        }

        if (!(parsingSucceeded && parsingSucceededActions)) {
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
    if ((parsingMode == PARSING_MODE_RENAME) && (contextStack.size() > previousContextStackSize) &&
        parsingSucceededActions) {
        if ((parsingIterations > 0) && (!nonexistantObject)) {
            // Function word not parsed alone -> error was not reported yet
            if (iter == end) {
                // Missing new name
                parsingSucceeded = false;
                addParseError(ParseError<Iterator>(line.begin(), end, iter, PARSE_ERROR_TYPE_IDENTIFIER_NOT_FOUND));
            } else if (contextStack.empty()) {
                // Missing object to rename
                parsingSucceeded = false;
                addParseError(ParseError<Iterator>(line.begin(), end, iter, "", allKinds,
                              PARSE_ERROR_TYPE_OBJECT_DEFINITION_NOT_FOUND));
            } else {
                // We are ready to parse new name
                parsingSucceeded = phrase_parse(iter, end, predefinedRules->getObjectIdentifier(),
                                                ascii::space, newName);
                if (iter != end)
                    parsingSucceeded = false;
                if (!parsingSucceeded)
                    addParseError(ParseError<Iterator>(line.begin(), end, iter, PARSE_ERROR_TYPE_IDENTIFIER_NOT_FOUND));
            }
        }
    }

    if ((!dryRun) && (parsingSucceeded && parsingSucceededActions)) {
        // Emit signals, when there is some function word used.
        switch (parsingMode) {
            case PARSING_MODE_STANDARD:
                // No special signal to be triggered inthis case.
                break;
            case PARSING_MODE_CREATE:
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

    if (!(parsingSucceeded && parsingSucceededActions)) {
         // Some bad input
        if (!dryRun)
            reportParseError(line);
    }

    // Invoke categoryLeft signals when parsing in-line definitions. Do not invoke categoryLeft
    // when in dryRun for purposes of generating tab completions.
    if (((parsingMode == PARSING_MODE_STANDARD) && singleKind) | (dryRun)) {
        // Definition of kind found stand-alone in standard mode on one line -> nest permanently
    } else {
        int depthDiff = contextStack.size() - previousContextStackSize;
        if (depthDiff > 0) {
            for (int i = 0; i < depthDiff; ++i) {
                if (parsingSucceeded)
                    categoryLeft();
                else
                    contextStack.pop_back();
            }
        }
    }
#ifdef PARSER_DEBUG
    std::cout << "End context stack: " << contextStackToString(contextStack) << std::endl;
#endif
    return (parsingSucceeded && parsingSucceededActions);
}



template <typename Iterator>
void ParserImpl<Iterator>::reportParseError(const std::string& line)
{
    // If there is no error, it means, that we entered some bad input fot the kind, that has no attributes and no
    // nested kinds, so there was no grammar to generate the error.
    if (parseErrors.empty()) {
        m_parser->parseError(NoAttributesOrKindsDefined("No attributes or nested kind names expected here.",
                                                        line, line.begin()));
        return;
    }

    typename std::vector<ParseError<Iterator> >::iterator it;

    // Error in the attribute value
    it = std::find_if(parseErrors.begin(), parseErrors.end(), phoenix::bind(&ParseError<Iterator>::errorType,
                      phoenix::arg_names::_1) == PARSE_ERROR_TYPE_VALUE_TYPE);
    if (it != parseErrors.end()) {
#ifdef PARSER_DEBUG
        std::cout << it->toString() << std::endl;
#endif
        m_parser->parseError(InvalidAttributeDataTypeError(it->toString(), line, it->errorPosition()));
        return;
    }

    // Pair kind name - object name expected, but not found
    it = std::find_if(parseErrors.begin(), parseErrors.end(), phoenix::bind(&ParseError<Iterator>::errorType,
                      phoenix::arg_names::_1) == PARSE_ERROR_TYPE_OBJECT_DEFINITION_NOT_FOUND);
    if (it != parseErrors.end()) {
#ifdef PARSER_DEBUG
        std::cout << it->toString() << std::endl;
#endif
        m_parser->parseError(ObjectDefinitionNotFound(it->toString(), line, it->errorPosition()));
        return;
    }

    // Error in object name
    it = std::find_if(parseErrors.begin(), parseErrors.end(), phoenix::bind(&ParseError<Iterator>::errorType,
                      phoenix::arg_names::_1) == PARSE_ERROR_TYPE_IDENTIFIER_NOT_FOUND);
    if (it != parseErrors.end()) {
#ifdef PARSER_DEBUG
        std::cout << it->toString() << std::endl;
#endif
        m_parser->parseError(MalformedIdentifier(it->toString(), line, it->errorPosition()));
        return;
    }

    // Existing object required, but entered object does not exist
    it = std::find_if(parseErrors.begin(), parseErrors.end(), phoenix::bind(&ParseError<Iterator>::errorType,
                      phoenix::arg_names::_1) == PARSE_ERROR_TYPE_OBJECT_NOT_FOUND);
    if (it != parseErrors.end()) {
#ifdef PARSER_DEBUG
        std::cout << it->toString() << std::endl;
#endif
        m_parser->parseError(ObjectNotFound(it->toString(), line, it->errorPosition()));
        return;
    }

    // Error in identifier (bad number of "->")
    it = std::find_if(parseErrors.begin(), parseErrors.end(), phoenix::bind(&ParseError<Iterator>::errorType,
                      phoenix::arg_names::_1) == PARSE_ERROR_TYPE_KIND_NESTING);
    if (it != parseErrors.end()) {
#ifdef PARSER_DEBUG
        std::cout << it->toString() << std::endl;
#endif
        m_parser->parseError(MalformedIdentifier(it->toString(), line, line.begin()));
        return;
    }

    // Error in kind name when creating a new object using "new"
    it = std::find_if(parseErrors.begin(), parseErrors.end(), phoenix::bind(&ParseError<Iterator>::errorType,
                      phoenix::arg_names::_1) == PARSE_ERROR_TYPE_KINDS_CONSTRUCT);
    if (it != parseErrors.end()) {
#ifdef PARSER_DEBUG
        std::cout << it->toString() << std::endl;
#endif
        m_parser->parseError(InvalidObjectKind(it->toString(), line, it->errorPosition()));
        return;
    }

    // Error in identifiers set name
    it = std::find_if(parseErrors.begin(), parseErrors.end(), phoenix::bind(&ParseError<Iterator>::errorType,
                      phoenix::arg_names::_1) == PARSE_ERROR_TYPE_IDENTIFIERS_SET);
    if (it != parseErrors.end()) {
#ifdef PARSER_DEBUG
        std::cout << it->toString() << std::endl;
#endif
        m_parser->parseError(UndefinedAttributeError(it->toString(), line, it->errorPosition()));
        return;
    }

    // Error in attribute name in removal
    it = std::find_if(parseErrors.begin(), parseErrors.end(), phoenix::bind(&ParseError<Iterator>::errorType,
                      phoenix::arg_names::_1) == PARSE_ERROR_TYPE_ATTRIBUTE_REMOVAL);
    if (it != parseErrors.end()) {
#ifdef PARSER_DEBUG
        std::cout << it->toString() << std::endl;
#endif
        m_parser->parseError(UndefinedAttributeError(it->toString(), line, it->errorPosition()));
        return;
    }

    // Error in attribute name
    it = std::find_if(parseErrors.begin(), parseErrors.end(), phoenix::bind(&ParseError<Iterator>::errorType,
                      phoenix::arg_names::_1) == PARSE_ERROR_TYPE_ATTRIBUTE);
    if (it != parseErrors.end()) {
        // Error in attribute name or nested kind name -> report them together
        typename std::vector<ParseError<Iterator> >::iterator itk =
            std::find_if(parseErrors.begin(), parseErrors.end(), phoenix::bind(&ParseError<Iterator>::errorType,
                         phoenix::arg_names::_1) == PARSE_ERROR_TYPE_KIND);
        if (itk != parseErrors.end()) {
#ifdef PARSER_DEBUG
            std::cout << it->toCombinedString(*itk) << std::endl;
#endif
            m_parser->parseError(UndefinedAttributeError(it->toCombinedString(*itk), line, it->errorPosition()));
            return;
        }
#ifdef PARSER_DEBUG
        std::cout << it->toString() << std::endl;
#endif
        m_parser->parseError(UndefinedAttributeError(it->toString(), line, it->errorPosition()));
        return;
    }

    // Error in kind nesting
    it = std::find_if(parseErrors.begin(), parseErrors.end(), phoenix::bind(&ParseError<Iterator>::errorType,
                      phoenix::arg_names::_1) == PARSE_ERROR_TYPE_NESTING);
    if (it != parseErrors.end()) {
#ifdef PARSER_DEBUG
        std::cout << it->toString() << std::endl;
#endif
        m_parser->parseError(InvalidObjectKind(it->toString(), line, it->errorPosition()));
        return;
    }

    // Error in object name
    it = std::find_if(parseErrors.begin(), parseErrors.end(), phoenix::bind(&ParseError<Iterator>::errorType,
                      phoenix::arg_names::_1) == PARSE_ERROR_TYPE_OBJECT_NAME);
    if (it != parseErrors.end()) {
#ifdef PARSER_DEBUG
        std::cout << it->toString() << std::endl;
#endif
        m_parser->parseError(MalformedIdentifier(it->toString(), line, it->errorPosition()));
        return;
    }

    // Error in kind name in special filter
    it = std::find_if(parseErrors.begin(), parseErrors.end(), phoenix::bind(&ParseError<Iterator>::errorType,
                      phoenix::arg_names::_1) == PARSE_ERROR_TYPE_KIND_SPECIAL_FILTER);
    if (it != parseErrors.end()) {
#ifdef PARSER_DEBUG
        std::cout << it->toString() << std::endl;
#endif
        m_parser->parseError(InvalidObjectKind(it->toString(), line, it->errorPosition()));
        return;
    }

    // Error in kind name
    it = std::find_if(parseErrors.begin(), parseErrors.end(), phoenix::bind(&ParseError<Iterator>::errorType,
                      phoenix::arg_names::_1) == PARSE_ERROR_TYPE_KIND);
    if (it != parseErrors.end()) {
#ifdef PARSER_DEBUG
        std::cout << it->toString() << std::endl;
#endif
        m_parser->parseError(InvalidObjectKind(it->toString(), line, it->errorPosition()));
        return;
    }

    // Error in kind name in filter
    it = std::find_if(parseErrors.begin(), parseErrors.end(), phoenix::bind(&ParseError<Iterator>::errorType,
                      phoenix::arg_names::_1) == PARSE_ERROR_TYPE_KIND_FILTER);
    if (it != parseErrors.end()) {
#ifdef PARSER_DEBUG
        std::cout << it->toString() << std::endl;
#endif
        m_parser->parseError(InvalidObjectKind(it->toString(), line, it->errorPosition()));
        return;
    }

    throw std::out_of_range("Parse error reporting: No error reported.");
}



template <typename Iterator>
void ParserImpl<Iterator>::insertTabPossibilitiesOfCurrentContext(const std::string &line,
                                                                  std::vector<std::string> &possibilities)
{
    if (line.empty())
        possibilities.push_back("show");
    if (contextStack.empty()) {
        // No context -> add names of kinds
        for (std::vector<Db::Identifier>::iterator it = allKinds.begin(); it != allKinds.end(); ++it) {
            possibilities.push_back(line + *it);
        }
        if ((!allKinds.empty()) && (line.empty())) {
            possibilities.push_back(line + "create");
            possibilities.push_back(line + "delete");
            possibilities.push_back(line + "rename");
            possibilities.push_back(line + "all");
        }
    } else {
        // Do not add completions of attributes when in non-standard mode.
        if (parsingMode == PARSING_MODE_STANDARD) {
            // Add names of attributes of current kind
            std::vector<Db::KindAttributeDataType> attributes = m_parser->m_dbApi->kindAttributes(contextStack.back().kind);
            for (std::vector<Db::KindAttributeDataType>::iterator it = attributes.begin(); it != attributes.end(); ++it) {
                possibilities.push_back(line + it->name);
            }
            for (std::vector<std::pair<Db::Identifier, Db::Identifier> >::iterator itm = 
                contains[contextStack.back().kind].begin(); itm != contains[contextStack.back().kind].end(); ++itm) {
                std::vector<Db::KindAttributeDataType> attributes = m_parser->m_dbApi->kindAttributes(itm->second);
                for (std::vector<Db::KindAttributeDataType>::iterator it = 
                    attributes.begin(); it != attributes.end(); ++it) {
                    possibilities.push_back(line + it->name);
                }
            }
            if (!attributes.empty()) {
                possibilities.push_back(line + "no");
                if (containsIdentifiersSet(contextStack.back().kind)) {
                    possibilities.push_back(line + "add");
                    possibilities.push_back(line + "remove");
                }
            }
            possibilities.push_back(line + "end");
        }
        // Add names of nested kinds of current kind
        std::vector<Db::Identifier> embededKinds = parserKindsEmbedsRecursively(contextStack.back().kind);
        for (std::vector<Db::Identifier>::iterator it = embededKinds.begin(); it != embededKinds.end(); ++it) {
            possibilities.push_back(line + *it);
        }
        if (!embededKinds.empty()) {
            possibilities.push_back(line + "new");
            possibilities.push_back(line + "last");
            possibilities.push_back(line + "all");
            if (line.empty()) {
                possibilities.push_back(line + "create");
                possibilities.push_back(line + "delete");
                possibilities.push_back(line + "rename");
            }
        }
    }
}



template <typename Iterator>
void ParserImpl<Iterator>::insertTabPossibilitiesFromErrors(const std::string &line,
                                                            std::vector<std::string> &possibilities)
{
    std::string::const_iterator realEnd = line.end() - 1;
    while ((*realEnd != ' ') && (*realEnd != '\t') && (*realEnd != '\n') && (*realEnd != '(') && (*realEnd != ')')) {
        if (realEnd == line.begin())
            break;
        --realEnd;
    }
    // Step to our imaginary line end.
    ++realEnd;

    typename std::vector<ParseError<Iterator> >::iterator it;

    // At first, find out if the user wants to enter some value
    it = std::find_if(parseErrors.begin(), parseErrors.end(), phoenix::bind(&ParseError<Iterator>::errorType,
                      phoenix::arg_names::_1) == PARSE_ERROR_TYPE_OBJECT_NAME);
    if (it != parseErrors.end()) {
#ifdef PARSER_DEBUG
        std::cout << "Tab completion error: PARSE_ERROR_TYPE_OBJECT_NAME" << std::endl;
        std::cout << "Tab completion error offset: " << realEnd - it->errorPosition() << std::endl;
#endif
        // Error have to occur at the end of the line
        if ((realEnd - it->errorPosition()) == 0) {
            std::vector<std::string> expectations = it->expectedTypes();
            // Check if the user is supposed to enter some objects name, that we can complete
            if (!(it->context().empty())) {
                std::vector<Db::Identifier> objects = m_parser->m_dbApi->kindInstances(it->context());
                for (std::vector<Db::Identifier>::iterator iti = objects.begin(); iti != objects.end(); ++iti) {
                    std::vector<Db::Identifier> path = pathToVector(*iti);
                    BOOST_ASSERT(path.size() > contextStack.size());
                    possibilities.push_back(line + vectorToPath(std::vector<Db::Identifier>(
                        path.begin() + contextStack.size(), path.end())));
                }
            }
            possibilities.push_back(line + "where");
        }
    }

    // Find out, if the user wants to enter attribute name for value removal
    it = std::find_if(parseErrors.begin(), parseErrors.end(), phoenix::bind(&ParseError<Iterator>::errorType,
                      phoenix::arg_names::_1) == PARSE_ERROR_TYPE_ATTRIBUTE_REMOVAL);
    if (it != parseErrors.end()) {
#ifdef PARSER_DEBUG
        std::cout << "Tab completion error: PARSE_ERROR_TYPE_ATTRIBUTE_REMOVAL" << std::endl;
        std::cout << "Tab completion error offset: " << realEnd - it->errorPosition() << std::endl;
#endif
        // Error have to occur at the end of the line
        // Because of parsing the pair no <attribute name> using sequence parser with space skipper error occures
        // right after no keyword. That means it is one character before end of the line.
        if ((realEnd - it->errorPosition() - 1) == 0) {
            std::vector<std::string> expectations = it->expectedKeywords();
            for (std::vector<std::string>::iterator iti = expectations.begin(); iti != expectations.end(); ++iti) {
                possibilities.push_back(line + *iti);
            }
        }
    }

    // Find out, if the user wants to enter kind name and object name for renaming or deleting
    it = std::find_if(parseErrors.begin(), parseErrors.end(), phoenix::bind(&ParseError<Iterator>::errorType,
                      phoenix::arg_names::_1) == PARSE_ERROR_TYPE_OBJECT_DEFINITION_NOT_FOUND);
    if (it != parseErrors.end()) {
#ifdef PARSER_DEBUG
        std::cout << "Tab completion error: PARSE_ERROR_TYPE_OBJECT_DEFINITION_NOT_FOUND" << std::endl;
        std::cout << "Tab completion error offset: " << realEnd - it->errorPosition() << std::endl;
#endif
        // Error have to occur at the end of the line
        if ((realEnd - it->errorPosition()) == 0) {
            std::vector<std::string> expectations = it->expectedKeywords();
            for (std::vector<std::string>::iterator iti = expectations.begin(); iti != expectations.end(); ++iti) {
                possibilities.push_back(line + *iti);
            }
        }
    }

    // Find out, if the user wants to enter kind name for filter
    it = std::find_if(parseErrors.begin(), parseErrors.end(), phoenix::bind(&ParseError<Iterator>::errorType,
                      phoenix::arg_names::_1) == PARSE_ERROR_TYPE_KIND_SPECIAL_FILTER);
    if (it != parseErrors.end()) {
#ifdef PARSER_DEBUG
        std::cout << "Tab completion error: PARSE_ERROR_TYPE_KIND_SPECIAL_FILTER" << std::endl;
        std::cout << "Tab completion error offset: " << realEnd - it->errorPosition() << std::endl;
#endif
        // Error have to occur at the end of the line
        // Because of parsing the pair no <attribute name> using sequence parser with space skipper error occures
        // right after no keyword. That means it is one character before end of the line.
        if ((realEnd - it->errorPosition() - 1) == 0) {
            std::vector<std::string> expectations = it->expectedKeywords();
            for (std::vector<std::string>::iterator iti = expectations.begin(); iti != expectations.end(); ++iti) {
                possibilities.push_back(line + *iti);
            }
        }
    }

    // Find out, if the user wants to enter kind name
    it = std::find_if(parseErrors.begin(), parseErrors.end(), phoenix::bind(&ParseError<Iterator>::errorType,
                      phoenix::arg_names::_1) == PARSE_ERROR_TYPE_KIND);
    if (it != parseErrors.end()) {
#ifdef PARSER_DEBUG
        std::cout << "Tab completion error: PARSE_ERROR_TYPE_KIND" << std::endl;
        std::cout << "Tab completion error offset: " << realEnd - it->errorPosition() << std::endl;
#endif
        // Error have to occur at the end of the line
        if ((realEnd - it->errorPosition()) == 0) {
            std::vector<std::string> expectations = it->expectedKeywords();
            for (std::vector<std::string>::iterator iti = expectations.begin(); iti != expectations.end(); ++iti) {
                possibilities.push_back(line + *iti);
            }
        }
    }

    // Find out, if the user wants to enter kind name
    it = std::find_if(parseErrors.begin(), parseErrors.end(), phoenix::bind(&ParseError<Iterator>::errorType,
                      phoenix::arg_names::_1) == PARSE_ERROR_TYPE_KINDS_CONSTRUCT);
    if (it != parseErrors.end()) {
#ifdef PARSER_DEBUG
        std::cout << "Tab completion error: PARSE_ERROR_TYPE_KINDS_CONSTRUCT" << std::endl;
        std::cout << "Tab completion error offset: " << realEnd - it->errorPosition() << std::endl;
#endif
        // Error have to occur at the end of the line
        if ((realEnd - it->errorPosition() - 1) == 0) {
            std::vector<std::string> expectations = it->expectedKeywords();
            for (std::vector<std::string>::iterator iti = expectations.begin(); iti != expectations.end(); ++iti) {
                possibilities.push_back(line + *iti);
            }
        }
    }

    // Find out, if the user wants to enter attribute name
    it = std::find_if(parseErrors.begin(), parseErrors.end(), phoenix::bind(&ParseError<Iterator>::errorType,
                      phoenix::arg_names::_1) == PARSE_ERROR_TYPE_ATTRIBUTE);
    if (it != parseErrors.end()) {
#ifdef PARSER_DEBUG
        std::cout << "Tab completion error: PARSE_ERROR_TYPE_ATTRIBUTE" << std::endl;
        std::cout << "Tab completion error offset: " << realEnd - it->errorPosition() << std::endl;
#endif
        // Error have to occur at the end of the line
        if ((realEnd - it->errorPosition()) == 0) {
            std::vector<std::string> expectations = it->expectedKeywords();
            for (std::vector<std::string>::iterator iti = expectations.begin(); iti != expectations.end(); ++iti) {
                possibilities.push_back(line + *iti);
            }
        }
    }

    // Find out, if the user wants to enter attribute name
    it = std::find_if(parseErrors.begin(), parseErrors.end(), phoenix::bind(&ParseError<Iterator>::errorType,
                      phoenix::arg_names::_1) == PARSE_ERROR_TYPE_IDENTIFIERS_SET);
    if (it != parseErrors.end()) {
#ifdef PARSER_DEBUG
        std::cout << "Tab completion error: PARSE_ERROR_TYPE_IDENTIFIERS_SET" << std::endl;
        std::cout << "Tab completion error offset: " << realEnd - it->errorPosition() << std::endl;
#endif
        // Error have to occur at the end of the line
        if ((realEnd - it->errorPosition() - 1) == 0) {
            std::vector<std::string> expectations = it->expectedKeywords();
            for (std::vector<std::string>::iterator iti = expectations.begin(); iti != expectations.end(); ++iti) {
                possibilities.push_back(line + *iti);
            }
        }
    }

    // Find out, if the user wants to kind and attribute name of some referred kind for filter
    it = std::find_if(parseErrors.begin(), parseErrors.end(), phoenix::bind(&ParseError<Iterator>::errorType,
                      phoenix::arg_names::_1) == PARSE_ERROR_TYPE_KIND_FILTER);
    typename std::vector<ParseError<Iterator> >::iterator itobj =
        std::find_if(parseErrors.begin(), parseErrors.end(), phoenix::bind(&ParseError<Iterator>::errorType,
            phoenix::arg_names::_1) == PARSE_ERROR_TYPE_OBJECT_NAME);
    // This error could occur also while parsing object name, but we do not want to suggest completions here
    if ((it != parseErrors.end()) && !((itobj != parseErrors.end()) && ((realEnd - itobj->errorPosition()) == 0))) {
#ifdef PARSER_DEBUG
        std::cout << "Tab completion error: PARSE_ERROR_TYPE_KIND_FILTER" << std::endl;
        std::cout << "Tab completion error offset: " << realEnd - it->errorPosition() << std::endl;
#endif
        // Error have to occur at the end of the line
        if ((realEnd - it->errorPosition()) == 0) {
            std::vector<std::string> expectations = it->expectedKeywords();
            for (std::vector<std::string>::iterator iti = expectations.begin(); iti != expectations.end(); ++iti) {
                std::vector<Db::KindAttributeDataType> attributes = m_parser->m_dbApi->kindAttributes(*iti);
                for (std::vector<Db::KindAttributeDataType>::iterator itat = attributes.begin();
                     itat != attributes.end(); ++itat) {
                    possibilities.push_back(line + *iti + "." + itat->name);
                }
                possibilities.push_back(line + *iti + ".name");
            }
        }
    }

    // Find out, if the user wants to enter object name
    it = std::find_if(parseErrors.begin(), parseErrors.end(), phoenix::bind(&ParseError<Iterator>::errorType,
                      phoenix::arg_names::_1) == PARSE_ERROR_TYPE_VALUE_TYPE);
    if (it != parseErrors.end()) {
#ifdef PARSER_DEBUG
        std::cout << "Tab completion error: PARSE_ERROR_TYPE_VALUE_TYPE" << std::endl;
        std::cout << "Tab completion error offset: " << realEnd - it->errorPosition() << std::endl;
#endif
        std::vector<std::string> expectedTypes = it->expectedTypes();
        if ((expectedTypes.size() == 1) && (expectedTypes.front() == "identifier (alphanumerical letters and _)")) {
            // Error have to occur at the end of the line
            if (((realEnd - it->errorPosition()) == 0) && (!contextStack.empty())) {
                //Check whether the attribute reffers to some kind
                std::vector<std::pair<Db::Identifier, Db::Identifier> >::iterator itr;
                for (itr = refersTo[contextStack.back().kind].begin();
                     itr != refersTo[contextStack.back().kind].end(); ++itr)
                         if (itr->first == it->context())
                             break;
                if (itr != refersTo[contextStack.back().kind].end()) {
                    std::vector<Db::Identifier> objects = m_parser->m_dbApi->kindInstances(itr->second);
                    for (std::vector<Db::Identifier>::iterator iti = objects.begin(); iti != objects.end(); ++iti) {
                        possibilities.push_back(line + *iti);
                    }
                }

                std::map<Db::Identifier, std::pair<Db::Identifier, Db::Identifier> >::const_iterator i = embeddedInto.find(contextStack.back().kind);
                if ((i != embeddedInto.end()) && (i->second.first == it->context())) {
                    std::vector<Db::Identifier> objects = m_parser->m_dbApi->kindInstances(i->second.second);
                    for (std::vector<Db::Identifier>::iterator iti = objects.begin(); iti != objects.end(); ++iti) {
                        possibilities.push_back(line + *iti);
                    }
                }

                for (itr = contains[contextStack.back().kind].begin();
                     itr != contains[contextStack.back().kind].end(); ++itr)
                         if (itr->first == it->context())
                             break;
                if (itr != contains[contextStack.back().kind].end()) {
                    std::vector<Db::Identifier> objects = m_parser->m_dbApi->kindInstances(itr->second);
                    for (std::vector<Db::Identifier>::iterator iti = objects.begin(); iti != objects.end(); ++iti) {
                        possibilities.push_back(line + *iti);
                    }
                }

                for (itr = containable[contextStack.back().kind].begin();
                     itr != containable[contextStack.back().kind].end(); ++itr)
                         if (itr->first == it->context())
                             break;
                if (itr != containable[contextStack.back().kind].end()) {
                    std::vector<Db::Identifier> objects = m_parser->m_dbApi->kindInstances(itr->second);
                    for (std::vector<Db::Identifier>::iterator iti = objects.begin(); iti != objects.end(); ++iti) {
                        possibilities.push_back(line + *iti);
                    }
                }

                for (itr = templatized[contextStack.back().kind].begin();
                     itr != templatized[contextStack.back().kind].end(); ++itr)
                         if (itr->first == it->context())
                             break;
                if (itr != templatized[contextStack.back().kind].end()) {
                    std::vector<Db::Identifier> objects = m_parser->m_dbApi->kindInstances(itr->second);
                    for (std::vector<Db::Identifier>::iterator iti = objects.begin(); iti != objects.end(); ++iti) {
                        possibilities.push_back(line + *iti);
                    }
                }
            }
        }
    }
}



template <typename Iterator>
std::vector<Db::Identifier> ParserImpl<Iterator>::parserKindsEmbedsRecursively(const Db::Identifier &kindName)
{
    std::vector<Db::Identifier> embedsRecursivelyTotal = parserKindsEmbeds(kindName);
    for (std::vector<Db::Identifier>::iterator it = embedsRecursivelyTotal.begin();
         it != embedsRecursivelyTotal.end(); ++it) {
        std::vector<Db::Identifier> embedsRecursively = parserKindsEmbedsRecursively(*it);
        embedsRecursivelyTotal.insert(embedsRecursivelyTotal.end(), embedsRecursively.begin(), embedsRecursively.end());
    }

    return embedsRecursivelyTotal;
}



template <typename Iterator>
bool ParserImpl<Iterator>::containsIdentifiersSet(const Db::Identifier &kindName)
{
    std::vector<Db::KindAttributeDataType> attributes = m_parser->m_dbApi->kindAttributes(kindName);
    for (std::vector<Db::KindAttributeDataType>::iterator it = attributes.begin(); it != attributes.end(); ++it) {
        if (it->type == Db::TYPE_IDENTIFIER_SET)
            return true;
    }
    for (std::vector<std::pair<Db::Identifier, Db::Identifier> >::iterator itm = contains[kindName].begin(); itm != contains[kindName].end(); ++itm) {
        std::vector<Db::KindAttributeDataType> attributes = m_parser->m_dbApi->kindAttributes(itm->second);
        for (std::vector<Db::KindAttributeDataType>::iterator it = attributes.begin(); it != attributes.end(); ++it) {
            if (it->type == Db::TYPE_IDENTIFIER_SET)
                return true;
        }
    }
    return false;
}



/////////////////////////Template instances for linker//////////////////////////

template ParserImpl<iterator_type>::ParserImpl(Parser *parent);

template ParserImpl<iterator_type>::~ParserImpl();

template std::map<std::string, std::string> ParserImpl<iterator_type>::parserKeywordsUsage();

template std::vector<Db::Identifier> ParserImpl<iterator_type>::parserKindsEmbeds(const Db::Identifier &kindName);

template std::vector<std::pair<Db::Identifier, std::string> > ParserImpl<iterator_type>::parserKindsAttributes(const Db::Identifier &kindName);

template void ParserImpl<iterator_type>::parseLine(const std::string &line);

template bool ParserImpl<iterator_type>::isNestedInContext() const;

template ContextStack ParserImpl<iterator_type>::currentContextStack() const;

template std::vector<std::string> ParserImpl<iterator_type>::tabCompletionPossibilities(const std::string &line);

template void ParserImpl<iterator_type>::newObject(const Db::Identifier &kind);

template void ParserImpl<iterator_type>::categoryEntered(const Db::Identifier &kind, const Db::Identifier &name);

template void ParserImpl<iterator_type>::categoryLeft();

template void ParserImpl<iterator_type>::attributeSet(const Db::Identifier &kindName, const Db::Identifier &name, const Db::Value &value);

template void ParserImpl<iterator_type>::attributeSetInsert(const Db::Identifier &kindName, const Db::Identifier &name, const Db::Identifier &value);

template void ParserImpl<iterator_type>::attributeSetRemove(const Db::Identifier &kindName, const Db::Identifier &name, const Db::Identifier &value);

template void ParserImpl<iterator_type>::attributeRemove(const Db::Identifier &kindName, const Db::Identifier &name);

template void ParserImpl<iterator_type>::objectsFilter(const Db::Identifier &kind, const boost::optional<Db::Filter> &filter);

template void ParserImpl<iterator_type>::parsedSingleKind();

template void ParserImpl<iterator_type>::addParseError(const ParseError<iterator_type> &error);

template void ParserImpl<iterator_type>::setParsingMode(ParsingMode mode);

template ParsingMode ParserImpl<iterator_type>::getParsingMode();

template std::vector<Db::Identifier> ParserImpl<iterator_type>::getKindNames();

template void ParserImpl<iterator_type>::setContextStack(const ContextStack &stack);

template void ParserImpl<iterator_type>::clearContextStack();

template void ParserImpl<iterator_type>::addKindAttributes(const Db::Identifier &kindName, AttributesSettingParser<iterator_type> *attributesSettingParser, AttributeRemovalsParser<iterator_type> *attributeRemovalsParser, IdentifiersSetsParser<iterator_type> *identifiersSetsParser, FilterExpressionsParser<iterator_type> *filtersParser);

template void ParserImpl<iterator_type>::addNestedKinds(const Db::Identifier &kindName, KindsOnlyParser<iterator_type> *kindsOnlyParser, KindsFiltersParser<iterator_type> *kindsFiltersParser, KindsConstructParser<iterator_type> *kindsConstructParser);

template void ParserImpl<iterator_type>::addNestedKinds(const Db::Identifier &kindName, FiltersParser<iterator_type> *filtersParser);

template bool ParserImpl<iterator_type>::parseLineImpl(const std::string &line);

template void ParserImpl<iterator_type>::reportParseError(const std::string& line);

template void ParserImpl<iterator_type>::insertTabPossibilitiesOfCurrentContext(const std::string &line, std::vector<std::string> &possibilities);

template void ParserImpl<iterator_type>::insertTabPossibilitiesFromErrors(const std::string &line, std::vector<std::string> &possibilities);

template std::vector<Db::Identifier> ParserImpl<iterator_type>::parserKindsEmbedsRecursively(const Db::Identifier &kindName);

template bool ParserImpl<iterator_type>::containsIdentifiersSet(const Db::Identifier &kindName);

}
}
