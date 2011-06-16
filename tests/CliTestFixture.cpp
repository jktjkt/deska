/* Copyright (C) 2011 Jan Kundrát <kundratj@fzu.cz>
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


#include <boost/test/test_tools.hpp>
#include <boost/assert.hpp>


#include "CliTestFixture.h"



TestUserInterfaceIO::TestUserInterfaceIO(CliTestFixture *cliTester)
{
    tester = cliTester;
}



TestUserInterfaceIO::~TestUserInterfaceIO()
{
}



void TestUserInterfaceIO::reportError(const std::string &errorMessage)
{
    tester->expectHelper(MockCliEvent::reportError(errorMessage));
}



void TestUserInterfaceIO::printMessage(const std::string &message)
{
    tester->expectHelper(MockCliEvent::printMessage(message));
}



bool TestUserInterfaceIO::confirmDeletion(const Deska::Db::ObjectDefinition &object)
{
    MockCliEvent event = MockCliEvent::confirmDeletion(object);
    tester->expectHelper(event);
    MockCliEvent ret = tester->returnHelper(event);
    return ret.boolean;
}



bool TestUserInterfaceIO::confirmCreation(const Deska::Db::ObjectDefinition &object)
{
    MockCliEvent event = MockCliEvent::confirmCreation(object);
    tester->expectHelper(event);
    MockCliEvent ret = tester->returnHelper(event);
    return ret.boolean;
}



bool TestUserInterfaceIO::confirmRestoration(const Deska::Db::ObjectDefinition &object)
{
    MockCliEvent event = MockCliEvent::confirmRestoration(object);
    tester->expectHelper(event);
    MockCliEvent ret = tester->returnHelper(event);
    return ret.boolean;
}



std::string TestUserInterfaceIO::askForCommitMessage()
{
    MockCliEvent event = MockCliEvent::askForCommitMessage();
    tester->expectHelper(event);
    MockCliEvent ret = tester->returnHelper(event);
    return ret.str1;
}



std::string TestUserInterfaceIO::askForDetachMessage()
{
    MockCliEvent event = MockCliEvent::askForDetachMessage();
    tester->expectHelper(event);
    MockCliEvent ret = tester->returnHelper(event);
    return ret.str1;
}



void TestUserInterfaceIO::printHelp(const std::map<std::string, std::string> &cliCommands,
                                const std::map<std::string, std::string> &parserKeywords)
{
    tester->expectHelper(MockCliEvent::printHelp(cliCommands, parserKeywords));
}



void TestUserInterfaceIO::printHelpCommand(const std::string &cmdName, const std::string &cmdDscr)
{
    tester->expectHelper(MockCliEvent::printHelpCommand(cmdName, cmdDscr));
}



void TestUserInterfaceIO::printHelpKeyword(const std::string &keywordName, const std::string &keywordDscr)
{
    tester->expectHelper(MockCliEvent::printHelpKeyword(keywordName, keywordDscr));
}



void TestUserInterfaceIO::printHelpKind(const std::string &kindName,
                                    const std::vector<std::pair<std::string, std::string> > &kindAttrs,
                                    const std::vector<std::string> &nestedKinds)
{
    tester->expectHelper(MockCliEvent::printHelpKind(kindName, kindAttrs, nestedKinds));
}



void TestUserInterfaceIO::printHelpShowKinds(const std::vector<std::string> &kinds)
{
    tester->expectHelper(MockCliEvent::printHelpShowKinds(kinds));
}



int TestUserInterfaceIO::chooseChangeset(const std::vector<Deska::Db::PendingChangeset> &pendingChangesets)
{
    MockCliEvent event = MockCliEvent::chooseChangeset(pendingChangesets);
    tester->expectHelper(event);
    MockCliEvent ret = tester->returnHelper(event);
    return ret.integer;
}



std::string TestUserInterfaceIO::readLine(const std::string &prompt)
{
    MockCliEvent event = MockCliEvent::readLine(prompt);
    tester->expectHelper(event);
    MockCliEvent ret = tester->returnHelper(event);
    return ret.str1;
}



void TestUserInterfaceIO::printAttributes(const std::vector<Deska::Db::AttributeDefinition> &attributes, int indentLevel,
                                      std::ostream &out)
{
    tester->expectHelper(MockCliEvent::printAttributes(attributes, indentLevel));
}



void TestUserInterfaceIO::printObjects(const std::vector<Deska::Db::ObjectDefinition> &objects, int indentLevel,
                                   bool fullName, std::ostream &out)
{
    tester->expectHelper(MockCliEvent::printObjects(objects, indentLevel, fullName));
}



void TestUserInterfaceIO::printAttribute(const Deska::Db::AttributeDefinition &attribute, int indentLevel, std::ostream &out)
{
    tester->expectHelper(MockCliEvent::printAttribute(attribute, indentLevel));
}



void TestUserInterfaceIO::printObject(const Deska::Db::ObjectDefinition &object, int indentLevel, bool fullName, std::ostream &out)
{
    tester->expectHelper(MockCliEvent::printObject(object, indentLevel, fullName));
}



void TestUserInterfaceIO::printEnd(int indentLevel, std::ostream &out)
{
    tester->expectHelper(MockCliEvent::printEnd(indentLevel));
}



void TestUserInterfaceIO::addCommandCompletion(const std::string &completion)
{
    tester->expectHelper(MockCliEvent::addCommandCompletion(completion));
}



CliTestFixture::CliTestFixture()
{
    conn = 0;
    parser = 0;
    db = 0;
    io = 0;
    ui = 0;
    sh = 0;
    testStarted = false;
}



CliTestFixture::~CliTestFixture()
{
    if (sh != 0)
        delete sh;
    if (ui != 0)
        delete ui;
    if (io != 0)
        delete io;
    if (db != 0)
        delete db;
    if (parser != 0)
        delete parser;
    if (conn != 0)
        delete conn;
}



void CliTestFixture::expectReportError(const std::string &errorMessage)
{
    cliEvents.push(MockCliEvent::reportError(errorMessage));
}



void CliTestFixture::expectPrintMessage(const std::string &message)
{
    cliEvents.push(MockCliEvent::printMessage(message));
}



void CliTestFixture::expectConfirmDeletion(const Deska::Db::ObjectDefinition &object)
{
    cliEvents.push(MockCliEvent::confirmDeletion(object));
}



void CliTestFixture::returnConfirmDeletion(bool confirm)
{
    cliEvents.push(MockCliEvent::returnConfirmDeletion(confirm));
}



void CliTestFixture::expectConfirmCreation(const Deska::Db::ObjectDefinition &object)
{
    cliEvents.push(MockCliEvent::confirmCreation(object));
}



void CliTestFixture::returnConfirmCreation(bool confirm)
{
    cliEvents.push(MockCliEvent::returnConfirmCreation(confirm));
}



void CliTestFixture::expectConfirmRestoration(const Deska::Db::ObjectDefinition &object)
{
    cliEvents.push(MockCliEvent::confirmRestoration(object));
}



void CliTestFixture::returnConfirmRestoration(bool confirm)
{
    cliEvents.push(MockCliEvent::returnConfirmRestoration(confirm));
}



void CliTestFixture::expectAskForCommitMessage()
{
    cliEvents.push(MockCliEvent::askForCommitMessage());
}



void CliTestFixture::returnAskForCommitMessage(const std::string &message)
{
    cliEvents.push(MockCliEvent::returnAskForCommitMessage(message));
}



void CliTestFixture::expectAskForDetachMessage()
{
    cliEvents.push(MockCliEvent::askForDetachMessage());
}



void CliTestFixture::returnAskForDetachMessage(const std::string &message)
{
    cliEvents.push(MockCliEvent::returnAskForDetachMessage(message));
}



void CliTestFixture::expectPrintHelp(const std::map<std::string, std::string> &cliCommands,
                                   const std::map<std::string, std::string> &parserKeywords)
{
    cliEvents.push(MockCliEvent::printHelp(cliCommands, parserKeywords));
}



void CliTestFixture::expectPrintHelpCommand(const std::string &cmdName, const std::string &cmdDscr)
{
    cliEvents.push(MockCliEvent::printHelpCommand(cmdName, cmdDscr));
}



void CliTestFixture::expectPrintHelpKeyword(const std::string &keywordName, const std::string &keywordDscr)
{
    cliEvents.push(MockCliEvent::printHelpKeyword(keywordName, keywordDscr));
}



void CliTestFixture::expectPrintHelpKind(const std::string &kindName,
                                  const std::vector<std::pair<std::string, std::string> > &kindAttrs,
                                  const std::vector<std::string> &nestedKinds)
{
    cliEvents.push(MockCliEvent::printHelpKind(kindName, kindAttrs, nestedKinds));
}



void CliTestFixture::expectPrintHelpShowKinds(const std::vector<std::string> &kinds)
{
    cliEvents.push(MockCliEvent::printHelpShowKinds(kinds));
}



void CliTestFixture::expectChooseChangeset(const std::vector<Deska::Db::PendingChangeset> &pendingChangesets)
{
    cliEvents.push(MockCliEvent::chooseChangeset(pendingChangesets));
}



void CliTestFixture::returnChooseChangeset(int changeset)
{
    cliEvents.push(MockCliEvent::returnChooseChangeset(changeset));
}



void CliTestFixture::expectReadLine(const std::string &prompt)
{
    cliEvents.push(MockCliEvent::readLine(prompt));
}



void CliTestFixture::returnReadLine(const std::string &line)
{
    cliEvents.push(MockCliEvent::returnReadLine(line));
}



void CliTestFixture::expectPrintAttributes(const std::vector<Deska::Db::AttributeDefinition> &attributes, int indentLevel,
                                           std::ostream &out)
{
    cliEvents.push(MockCliEvent::printAttributes(attributes, indentLevel));
}



void CliTestFixture::expectPrintAttribute(const Deska::Db::AttributeDefinition &attribute, int indentLevel,
                                          std::ostream &out)
{
    cliEvents.push(MockCliEvent::printAttribute(attribute, indentLevel));
}                                  



void CliTestFixture::expectPrintObjects(const std::vector<Deska::Db::ObjectDefinition> &objects, int indentLevel,
                                        bool fullName, std::ostream &out)
{
    cliEvents.push(MockCliEvent::printObjects(objects, indentLevel, fullName));
}



void CliTestFixture::expectPrintObject(const Deska::Db::ObjectDefinition &object, int indentLevel, bool fullName,
                                       std::ostream &out)
{
    cliEvents.push(MockCliEvent::printObject(object, indentLevel, fullName));
}



void CliTestFixture::expectPrintEnd(int indentLevel, std::ostream &out)
{
    cliEvents.push(MockCliEvent::printEnd(indentLevel));
}



void CliTestFixture::expectAddCommandCompletion(const std::string &completion)
{
    cliEvents.push(MockCliEvent::addCommandCompletion(completion));
}



void CliTestFixture::verifyEnd()
{
    BOOST_CHECK_MESSAGE(cliEvents.empty(), "Expected no more events");
    if (!cliEvents.empty()) {
        // show the first queued event here
        BOOST_CHECK_EQUAL(MockCliEvent::invalid(), cliEvents.front());
    }
}



void CliTestFixture::startTest()
{
    BOOST_ASSERT(!testStarted);
    testStarted = true;
    conn = new Deska::Db::Connection();
    parser = new Deska::Cli::Parser(conn);
    db = new Deska::Cli::DbInteraction(conn);
    io = new TestUserInterfaceIO(this);
    ui = new Deska::Cli::UserInterface(db, parser, io);
    sh = new Deska::Cli::SignalsHandler(parser, ui);
    ui->run();
}



void CliTestFixture::expectHelper(const MockCliEvent &e)
{
    BOOST_CHECK(!cliEvents.empty());
    bool shouldPop = !cliEvents.empty();
    MockCliEvent other = shouldPop ? cliEvents.front() : MockCliEvent::invalid();
    BOOST_CHECK_EQUAL(e, other);
    if (shouldPop)
        cliEvents.pop();
}



MockCliEvent CliTestFixture::returnHelper(const MockCliEvent &e)
{
    BOOST_CHECK(!cliEvents.empty());
    bool shouldPop = !cliEvents.empty();
    MockCliEvent returner = shouldPop ? cliEvents.front() : MockCliEvent::invalid();
    BOOST_CHECK_MESSAGE(e.myReturn(returner), "No return value found for current event");
    if (shouldPop)
        cliEvents.pop();
    return returner;
}
