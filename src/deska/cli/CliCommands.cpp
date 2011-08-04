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
#include <boost/algorithm/string/trim.hpp>
#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>

#include "CliCommands.h"
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
LogFilterParser<Iterator>::LogFilterParser(): LogFilterParser<Iterator>::base_type(start)
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


    /*
    phoenix::function<AttributeErrorHandler<Iterator> > attributeErrorHandler = AttributeErrorHandler<Iterator>();
    phoenix::function<ValueErrorHandler<Iterator> > valueErrorHandler = ValueErrorHandler<Iterator>();
    on_error<fail>(kindExpr, attributeErrorHandler(_1, _2, _3, _4, phoenix::ref(kinds)));
    on_error<fail>(kindDispatch, valueErrorHandler(_1, _2, _3, _4, phoenix::ref(currentKindName)));
    */

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

    

Command::Command(UserInterface *userInterface): ui(userInterface)
{
}



Command::~Command()
{
}



std::vector<std::string> Command::completionPatterns()
{
    return complPatterns;
}



std::string Command::name()
{
    return cmdName;
}



std::string Command::usage()
{
    return cmdUsage;
}



std::vector<std::string> Command::extractParams(const std::string &params)
{
    std::vector<std::string> paramsList;
    boost::char_separator<char> separators(" \t");
    boost::tokenizer<boost::char_separator<char> > tokenizer(params, separators);
    std::string token;
    
    for (boost::tokenizer<boost::char_separator<char> >::const_iterator it = tokenizer.begin();
         it != tokenizer.end(); ++it) {
        token = *it;
        boost::algorithm::trim(token);
        paramsList.push_back(token);
    }

    return paramsList;
}



Start::Start(UserInterface *userInterface): Command(userInterface)
{
    cmdName = "start";
    cmdUsage = "Starts new changeset.";
    complPatterns.push_back("start");
}



Start::~Start()
{
}



void Start::operator()(const std::string &params)
{
    if (!params.empty()) {
        ui->io->reportError("Error: No parameters expected for command " + cmdName + ".");
        return;
    }
    if (ui->currentChangeset) {
        std::ostringstream ostr;
        ostr << "Error: You are already in the changeset " << *(ui->currentChangeset) << "!";
        ui->io->reportError(ostr.str());
        return;
    }
    ui->currentChangeset = ui->m_dbInteraction->createNewChangeset();
    std::ostringstream ostr;
    ostr << "Changeset " << *(ui->currentChangeset) << " started.";
    ui->io->printMessage(ostr.str());
}



Resume::Resume(UserInterface *userInterface): Command(userInterface)
{
    cmdName = "resume";
    cmdUsage = "Displays list of pending changesets with ability to connect to one.";
    complPatterns.push_back("resume");
}



Resume::~Resume()
{
}



void Resume::operator()(const std::string &params)
{
    if (!params.empty()) {
        ui->io->reportError("Error: No parameters expected for command " + cmdName + ".");
        return;
    }
    if (ui->currentChangeset) {
        std::ostringstream ostr;
        ostr << "Error: You are already in the changeset " << *(ui->currentChangeset) << "!";
        ui->io->reportError(ostr.str());
        return;
    }
    try {
        // Print list of pending changesets, so user can choose one
        std::vector<Db::PendingChangeset> pendingChangesets = ui->m_dbInteraction->allPendingChangesets();
        int choice = ui->io->chooseChangeset(pendingChangesets);

        if (choice >= 0) {
            // Some changeset was choosen
            ui->m_dbInteraction->resumeChangeset(pendingChangesets[choice].revision);
            ui->currentChangeset = pendingChangesets[choice].revision;
            std::ostringstream ostr;
            ostr << "Changeset " << *(ui->currentChangeset) << " resumed.";
            ui->io->printMessage(ostr.str());
        }
        
    } catch (Deska::Db::NotFoundError &e) {
        ui->io->reportError("Server reports an error:\nObject not found:\n\n" + e.whatWithBacktrace() + "\n");
    } catch (Deska::Db::NoChangesetError &e) {
        ui->io->reportError("Server reports an error:\nYou aren't associated to a changeset:\n\n" + e.whatWithBacktrace() + "\n");
    } catch (Deska::Db::ChangesetAlreadyOpenError &e) {
        ui->io->reportError("Server reports an error:\nChangeset is already open:\n\n" + e.whatWithBacktrace() + "\n");
    } catch (Deska::Db::SqlError &e) {
        ui->io->reportError("Server reports an error:\nError in executing an SQL statement:\n\n" + e.whatWithBacktrace() + "\n");
    } catch (Deska::Db::ServerError &e) {
        ui->io->reportError("Server reports an error:\nInternal server error:\n\n" + e.whatWithBacktrace() + "\n");
    } catch (Deska::Db::JsonSyntaxError &e) {
        ui->io->reportError("Cannot parse JSON data.\n " + e.whatWithBacktrace() + "\n");
    } catch (Deska::Db::JsonStructureError &e) {
        ui->io->reportError("Received malformed JSON data:\n " + e.whatWithBacktrace() + "\n");
    }
}



Commit::Commit(UserInterface *userInterface): Command(userInterface)
{
    cmdName = "commit";
    cmdUsage = "Displays promt for commit message and commits current changeset.";
    complPatterns.push_back("commit");
}



Commit::~Commit()
{
}



void Commit::operator()(const std::string &params)
{
    if (!(ui->currentChangeset)) {
        ui->io->reportError("Error: You are not in any changeset!");
        return;
    }
    std::string commitMessage;
    if (!params.empty()) {
        commitMessage = params;
    } else {
        commitMessage = ui->io->askForCommitMessage();
    }
    try {
        ui->m_dbInteraction->commitChangeset(commitMessage);
        std::ostringstream ostr;
        ostr << "Changeset " << *(ui->currentChangeset) << " commited.";
        ui->io->printMessage(ostr.str());
        ui->currentChangeset = boost::optional<Db::TemporaryChangesetId>();
        ui->m_parser->clearContextStack();
    } catch (Deska::Db::RemoteDbError &e) {
        // FIXME: quick & durty "fix" for the demo
        std::ostringstream ss;
        ss << "Error: commit failed: " << e.what();
        ui->io->reportError(ss.str());
    }
}



Detach::Detach(UserInterface *userInterface): Command(userInterface)
{
    cmdName = "detach";
    cmdUsage = "Displays promt for detach message and detaches from current changeset.";
    complPatterns.push_back("detach");
}



Detach::~Detach()
{
}



void Detach::operator()(const std::string &params)
{
    if (!(ui->currentChangeset)) {
        ui->io->reportError("Error: You are not in any changeset!");
        return;
    }
    std::string detachMessage;
    if (!params.empty()) {
        detachMessage = params;
    } else {
        detachMessage = ui->io->askForDetachMessage();
    }
    ui->m_dbInteraction->detachFromChangeset(detachMessage);
    std::ostringstream ostr;
    ostr << "Changeset " << *(ui->currentChangeset) << " detached.";
    ui->io->printMessage(ostr.str());
    ui->currentChangeset = boost::optional<Db::TemporaryChangesetId>();
    ui->m_parser->clearContextStack();
}



Abort::Abort(UserInterface *userInterface): Command(userInterface)
{
    cmdName = "abort";
    cmdUsage = "Aborts current changeset.";
    complPatterns.push_back("abort");
}



Abort::~Abort()
{
}



void Abort::operator()(const std::string &params)
{
    if (!params.empty()) {
        ui->io->reportError("Error: No parameters expected for command " + cmdName + ".");
        return;
    }
    if (!(ui->currentChangeset)) {
        ui->io->reportError("Error: You are not in any changeset!");
        return;
    }
    ui->m_dbInteraction->abortChangeset();
    std::ostringstream ostr;
    ostr << "Changeset " << *(ui->currentChangeset) << " aborted.";
    ui->io->printMessage(ostr.str());
    ui->currentChangeset = boost::optional<Db::TemporaryChangesetId>();
    ui->m_parser->clearContextStack();
}



Rebase::Rebase(UserInterface *userInterface): Command(userInterface)
{
    cmdName = "rebase";
    cmdUsage = "Rebases current changeset.";
    complPatterns.push_back("rebase");
}



Rebase::~Rebase()
{
}



void Rebase::operator()(const std::string &params)
{
    if (!params.empty()) {
        ui->io->reportError("Error: No parameters expected for command " + cmdName + ".");
        return;
    }
    if (!(ui->currentChangeset)) {
        ui->io->reportError("Error: You are not in any changeset!");
        return;
    }
    Db::RevisionId parentRevision = ui->m_dbInteraction->changesetParent(*(ui->currentChangeset));
    Db::RevisionId headRevision = ui->m_dbInteraction->allRevisions().back().revision;
    if (headRevision == parentRevision) {
        ui->io->printMessage("No rebase needed.");
        return;
    }
}



Status::Status(UserInterface *userInterface): Command(userInterface)
{
    cmdName = "status";
    cmdUsage = "Shows if you are connected to any changeset or not.";
    complPatterns.push_back("status");
}



Status::~Status()
{
}



void Status::operator()(const std::string &params)
{
    if (!params.empty()) {
        ui->io->reportError("Error: No parameters expected for command " + cmdName + ".");
        return;
    }
    if (ui->currentChangeset) {
        std::ostringstream ostr;
        ostr << "You are connected to changeset " << *(ui->currentChangeset) << ".";
        ui->io->printMessage(ostr.str());
    } else {
        ui->io->printMessage("You are not connected to any changeset.");
    }
}



Log::Log(UserInterface *userInterface): Command(userInterface)
{
    cmdName = "log";
    cmdUsage = "Command for operations with revisions and history. Without parameter shows list of revisions.";
    complPatterns.push_back("log");

    filterParser = new LogFilterParser<iterator_type>();
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
    bool r = boost::spirit::qi::phrase_parse(iter, end, *filterParser, boost::spirit::ascii::space, filter);
    if (!r || (iter != end)) {
        ui->io->reportError("Invalid revisions filter entered!");
        return;
    }

    std::vector<Db::RevisionMetadata> revisions = ui->m_dbInteraction->filteredRevisions(filter);
    ui->io->printRevisions(revisions);
}



Diff::Diff(UserInterface *userInterface): Command(userInterface)
{
    cmdName = "diff";
    cmdUsage = "Command for showing difference between revisions. Without parameters shows diff of current changeset with its parent. With two parameters, revisions IDs shows diff between these revisions.";
    complPatterns.push_back("diff");
}



Diff::~Diff()
{
}



void Diff::operator()(const std::string &params)
{
    if (params.empty()) {
        if (!ui->currentChangeset) {
            ui->io->reportError("Error: Wou have to be connected to a changeset to perform diff with its parent. Use commands \"start\" or \"resume\". Use \"help\" for more info.");
            return;
        }
        std::vector<Db::ObjectModification> modifications = ui->m_dbInteraction->revisionsDifferenceChangeset(
            *(ui->currentChangeset));
        ui->io->printDiff(modifications);
        return;
    }

    std::vector<std::string> paramsList = extractParams(params);
    if (paramsList.size() != 2) {
        ui->io->reportError("Invalid number of parameters entered!");
        return;
    }

    try {
        Db::RevisionId revA = stringToRevision(paramsList[0]);
        Db::RevisionId revB = stringToRevision(paramsList[1]);
        std::vector<Db::ObjectModification> modifications = ui->m_dbInteraction->revisionsDifference(
            stringToRevision(paramsList[0]), stringToRevision(paramsList[1]));
        ui->io->printDiff(modifications);
    } catch (std::invalid_argument &e) {
        ui->io->reportError("Invalid parameters entered!");
        return;
    }
}



Db::RevisionId Diff::stringToRevision(const std::string &rev)
{
    if ((rev.size() < 2) || (rev[0] != 'r'))
        throw std::invalid_argument("Deska::Cli::Log::stringToRevision: Error while converting string to revision ID.");
    std::string revStr(rev.begin() + 1, rev.end());
    unsigned int revInt;
    std::istringstream iss(revStr);
    iss >> revInt;
    if (iss.fail())
        throw std::invalid_argument("Deska::Cli::Log::stringToRevision: Error while converting string to revision ID.");
    return Db::RevisionId(revInt);
}



Configdiff::Configdiff(UserInterface *userInterface): Command(userInterface)
{
    cmdName = "configdiff";
    cmdUsage = "Command for showing diff of current configuration and configuration generated with changes in current changeset using configuration generators. With parameter \"regenerate\" forces regeneration of the configuration.";
    complPatterns.push_back("configdiff regenerate");
}



Configdiff::~Configdiff()
{
}



void Configdiff::operator()(const std::string &params)
{
    if (!ui->currentChangeset) {
        ui->io->reportError("Error: Wou have to be connected to a changeset to perform diff of configuration. Use commands \"start\" or \"resume\". Use \"help\" for more info.");
        return;
    }

    bool forceRegen = false;
    if (params.empty()) {
        // nothing special
    } else if (params == "regenerate") {
        forceRegen = true;
    } else {
        ui->io->reportError("Error: Invalid parameter entered. Use \"help\" for more info.");
        return;
    }

    std::string diff = ui->m_dbInteraction->configDiff(forceRegen);
    if (diff.empty())
        ui->io->printMessage("No difference.");
    else
        ui->io->printMessage(diff);
    return;
}



Exit::Exit(UserInterface *userInterface): Command(userInterface)
{
    cmdName = "exit";
    cmdUsage = "Exits the CLI.";
    complPatterns.push_back("exit");
    complPatterns.push_back("quit");
}



Exit::~Exit()
{
}



void Exit::operator()(const std::string &params)
{
    if (!params.empty()) {
        ui->io->reportError("Error: No parameters expected for command " + cmdName + ".");
        return;
    }
    ui->exitLoop = true;
}



Dump::Dump(UserInterface *userInterface): Command(userInterface)
{
    cmdName = "dump";
    cmdUsage = "Prints everything in the DB. Optional parameter is filename, where to save the dump.";
    complPatterns.push_back("dump %file");
}



Dump::~Dump()
{
}



void Dump::operator()(const std::string &params)
{
    if (params.empty()) {
        // FIXME: Dump recursively
        //BOOST_FOREACH(const Deska::Db::Identifier &kindName, ui->m_dbInteraction->topLevelKinds()) {
        BOOST_FOREACH(const Deska::Db::Identifier &kindName, ui->m_dbInteraction->kindNames()) {
            BOOST_FOREACH(const Deska::Cli::ObjectDefinition &object, ui->m_dbInteraction->kindInstances(kindName)) {
                //ui->io->printObject(object, 0, false);
                ui->io->printObject(object, 0, true);
                dumpObjectRecursive(object, 1);
            }
        }
    } else {
        std::ofstream ofs(params.c_str());
        if (!ofs) {
            ui->io->reportError("Error while dumping DB to file \"" + params + "\".");
            return;
        }
        // FIXME: Dump recursively
        //BOOST_FOREACH(const Deska::Db::Identifier &kindName, ui->m_dbInteraction->topLevelKinds()) {
        BOOST_FOREACH(const Deska::Db::Identifier &kindName, ui->m_dbInteraction->kindNames()) {
            BOOST_FOREACH(const Deska::Cli::ObjectDefinition &object, ui->m_dbInteraction->kindInstances(kindName)) {
                //ui->io->printObject(object, 0, false, ofs);
                ui->io->printObject(object, 0, true, ofs);
                dumpObjectRecursive(object, 1, ofs);
            }
        }
        ui->io->printMessage("DB successfully dumped into file \"" + params + "\".");
    }  
}



void Dump::dumpObjectRecursive(const ObjectDefinition &object, unsigned int depth, std::ostream &out)
{
    std::vector<AttributeDefinition> attributes = ui->m_dbInteraction->allAttributes(object);
    ui->io->printAttributes(attributes, depth, out);
    std::vector<ObjectDefinition> nestedObjs = ui->m_dbInteraction->allNestedObjects(object);
    for (std::vector<ObjectDefinition>::iterator it = nestedObjs.begin(); it != nestedObjs.end(); ++it) {
        ui->io->printObject(*it, depth, false, out);
        dumpObjectRecursive(*it, depth + 1, out);
    }
    if (depth > 0)
        ui->io->printEnd(depth - 1, out);
}



Context::Context(UserInterface *userInterface): Command(userInterface)
{
    cmdName = "context";
    cmdUsage = "Shows current context with detail of filters. With parameter \"objects\" prints list of objects matched by current context with its filters.";
    complPatterns.push_back("context objects");
}



Context::~Context()
{
}



void Context::operator()(const std::string &params)
{
    if (params.empty()) {
        std::string contextDump = dumpContextStack(ui->m_parser->currentContextStack());
        if (contextDump.empty())
            ui->io->printMessage("No context.");
        else
            ui->io->printMessage(contextDump);
        return;
    }

    if (params != "objects") {
        ui->io->reportError("Error: Unknown parameter. Function context supports parameter \"objects\". Try \"help context\".");
        return;
    }
    
    std::vector<ObjectDefinition> objects =
        ui->m_dbInteraction->expandContextStack(ui->m_parser->currentContextStack());
    if (objects.empty())
        ui->io->printMessage("No objects matched by current context stack.");
    else
        ui->io->printObjects(objects, 0, true);
}



Restore::Restore(UserInterface *userInterface): Command(userInterface)
{
    cmdName = "restore";
    cmdUsage = "Executes commands from a file. Can be used for restoring the DB from a dump. Requires file name with commands as a parameter. Lines with # at the beginning are comments and will not be parsed.";
    complPatterns.push_back("restore %file");
}



Restore::~Restore()
{
}



void Restore::operator()(const std::string &params)
{
    if (params.empty()) {
        ui->io->reportError("Error: This command requires file name as a parameter.");
        return;
    }
    if (!ui->currentChangeset) {
        ui->io->reportError("Error: Wou have to be connected to a changeset to perform restoration. Use commands \"start\" or \"resume\". Use \"help\" for more info.");
        return;
    }
    std::ifstream ifs(params.c_str());
    if (!ifs) {
        ui->io->reportError("Error while opening commands file \"" + params + "\".");
        return;
    }
    ui->nonInteractiveMode = true;
    std::string line;
    ContextStack stackBackup = ui->m_parser->currentContextStack();
    ui->m_parser->clearContextStack();
    unsigned int lineNumber = 0;
    while (!getline(ifs, line).eof()) {
        ++lineNumber;
        if (!line.empty() && line[0] == '#')
            continue;
        ui->m_parser->parseLine(line);
        if (ui->parsingFailed)
            break;
    }
    ui->nonInteractiveMode = false;
    if (ui->parsingFailed) {
        std::ostringstream ostr;
        ostr << "Parsing of commands file failed on line " << lineNumber << ".";
        ui->io->reportError(ostr.str());
    } else {
        ui->io->printMessage("All commands successfully executed.");
    }
    ui->m_parser->setContextStack(stackBackup);
}



Help::Help(UserInterface *userInterface): Command(userInterface)
{
    cmdName = "help";
    cmdUsage = "Displays this list of commands with usages. Accepts parameter. For command name or parser keyword as parametr, usage is displayed, for kind name is displayed content of the kind and for word \"kinds\" are all defined kind names printed.";
    complPatterns.push_back("help kinds");
    // Help is not in commands map, because we are constructing it now.
    complPatterns.push_back("help help");
    for (UserInterface::CommandMap::iterator it = ui->commandsMap.begin(); it != ui->commandsMap.end(); ++it) {
        complPatterns.push_back("help " + it->first);
    }
    std::vector<std::string> kinds = ui->m_dbInteraction->kindNames();
    for (std::vector<std::string>::iterator it = kinds.begin(); it != kinds.end(); ++it) {
        complPatterns.push_back("help " + *it);
    }
    std::map<std::string, std::string> keywords = ui->m_parser->parserKeywordsUsage();
    for (std::map<std::string, std::string>::iterator it = keywords.begin(); it != keywords.end(); ++it) {
        complPatterns.push_back("help " + it->first);
    }
}



Help::~Help()
{
}



void Help::operator()(const std::string &params)
{
    if (!params.empty()) {
        if (params == "kinds") {
            ui->io->printHelpShowKinds(ui->m_dbInteraction->kindNames());
            return;
        }
        UserInterface::CommandMap::iterator itc = ui->commandsMap.find(params);
        if ( itc != ui->commandsMap.end()) {
            ui->io->printHelpCommand(params, itc->second->usage());
            return;
        }
        std::map<std::string, std::string> keywords = ui->m_parser->parserKeywordsUsage();
        std::map<std::string, std::string>::iterator itke = keywords.find(params);
        if ( itke != keywords.end()) {
            ui->io->printHelpKeyword(params, itke->second);
            return;
        }
        std::vector<std::string> kinds = ui->m_dbInteraction->kindNames();
        std::vector<std::string>::iterator itki = std::find(kinds.begin(), kinds.end(), params);
        if ( itki != kinds.end()) {
            ui->io->printHelpKind(params, ui->m_parser->parserKindsAttributes(params),
                                  ui->m_parser->parserKindsEmbeds(params));
            return;
        }
        ui->io->reportError("Error: No help entry for \"" + params + "\".");
        return;
    }
    std::map<std::string, std::string> cliCommands;
    for (UserInterface::CommandMap::iterator it = ui->commandsMap.begin(); it != ui->commandsMap.end(); ++it) {
        cliCommands[it->first] = it->second->usage();
    }
    ui->io->printHelp(cliCommands, ui->m_parser->parserKeywordsUsage());
}



/////////////////////////Template instances for linker//////////////////////////

template LogFilterParser<iterator_type>::LogFilterParser();

template LogFilterParser<iterator_type>::~LogFilterParser();

template void LogFilterParser<iterator_type>::addKind(const Db::Identifier &kindName);


}
}