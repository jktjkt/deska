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

#define FORWARD_1(FUNC, EFUNC, TYPE_1) \
void TestUserInterfaceIO::FUNC(boost::call_traits<TYPE_1>::param_type arg1) { tester->expectHelper(MockCliEvent::FUNC(arg1)); } \
void CliTestFixture::expect##EFUNC(boost::call_traits<TYPE_1>::param_type arg1) { cliEvents.push(MockCliEvent::FUNC(arg1)); }

#define FORWARD_2(FUNC, EFUNC, TYPE_1, TYPE_2) \
void TestUserInterfaceIO::FUNC(boost::call_traits<TYPE_1>::param_type arg1, boost::call_traits<TYPE_2>::param_type arg2) { tester->expectHelper(MockCliEvent::FUNC(arg1, arg2)); } \
void CliTestFixture::expect##EFUNC(boost::call_traits<TYPE_1>::param_type arg1, boost::call_traits<TYPE_2>::param_type arg2) { cliEvents.push(MockCliEvent::FUNC(arg1, arg2)); }

#define FORWARD_3_RAW_ARGS(FUNC, EFUNC, TYPE_1, TYPE_2, TYPE_3) \
void TestUserInterfaceIO::FUNC(TYPE_1 arg1, TYPE_2 arg2, TYPE_3 arg3) \
{ tester->expectHelper(MockCliEvent::FUNC(arg1, arg2, arg3)); } \
void CliTestFixture::expect##EFUNC(TYPE_1 arg1, TYPE_2 arg2, TYPE_3 arg3) \
{ cliEvents.push(MockCliEvent::FUNC(arg1, arg2, arg3)); }

#define FORWARD_3(FUNC, EFUNC, TYPE_1, TYPE_2, TYPE_3) \
    FORWARD_3_RAW_ARGS(FUNC, EFUNC, boost::call_traits<TYPE_1>::param_type, boost::call_traits<TYPE_2>::param_type, boost::call_traits<TYPE_3>::param_type)

#define FORWARD_3_OSTREAM(FUNC, EFUNC, TYPE_1, TYPE_2) \
    FORWARD_3_RAW_ARGS(FUNC, EFUNC, boost::call_traits<TYPE_1>::param_type, boost::call_traits<TYPE_2>::param_type, std::ostream &)

#define FORWARD_4_OSTREAM(FUNC, EFUNC, TYPE_1, TYPE_2, TYPE_3) \
void TestUserInterfaceIO::FUNC(boost::call_traits<TYPE_1>::param_type arg1, boost::call_traits<TYPE_2>::param_type arg2, boost::call_traits<TYPE_3>::param_type arg3, std::ostream &stream) \
    { tester->expectHelper(MockCliEvent::FUNC(arg1, arg2, arg3, stream)); } \
void CliTestFixture::expect##EFUNC(boost::call_traits<TYPE_1>::param_type arg1, boost::call_traits<TYPE_2>::param_type arg2, boost::call_traits<TYPE_3>::param_type arg3, std::ostream &stream) \
    { cliEvents.push(MockCliEvent::FUNC(arg1, arg2, arg3, stream)); }

#define FORWARD_1_RETURN(FUNC, EFUNC, RET_TYPE, RET_VAR, TYPE_1) \
RET_TYPE TestUserInterfaceIO::FUNC(boost::call_traits<TYPE_1>::param_type arg1) { \
    MockCliEvent event = MockCliEvent::FUNC(arg1); \
    tester->expectHelper(event); \
    MockCliEvent ret = tester->returnHelper(event); \
    return ret.RET_VAR; \
    } \
void CliTestFixture::expect##EFUNC(boost::call_traits<TYPE_1>::param_type arg1) { cliEvents.push(MockCliEvent::FUNC(arg1)); } \
void CliTestFixture::return##EFUNC(boost::call_traits<RET_TYPE>::param_type res) { cliEvents.push(MockCliEvent::return##EFUNC(res)); }

#define FORWARD_0_RETURN(FUNC, EFUNC, RET_TYPE, RET_VAR) \
RET_TYPE TestUserInterfaceIO::FUNC() { \
    MockCliEvent event = MockCliEvent::FUNC(); \
    tester->expectHelper(event); \
    MockCliEvent ret = tester->returnHelper(event); \
    return ret.RET_VAR; \
    } \
void CliTestFixture::expect##EFUNC() { cliEvents.push(MockCliEvent::FUNC()); } \
void CliTestFixture::return##EFUNC(boost::call_traits<RET_TYPE>::param_type res) { cliEvents.push(MockCliEvent::return##EFUNC(res)); }


typedef std::map<std::string, std::string> map_string_string;
typedef std::vector<std::pair<std::string, std::string> > vect_pair_str_str;

FORWARD_1(reportError, ReportError, std::string);
FORWARD_1(printMessage, PrintMessage, std::string);
FORWARD_2(printHelp, PrintHelp, map_string_string, map_string_string);
FORWARD_2(printHelpCommand, PrintHelpCommand, std::string, std::string);
FORWARD_2(printHelpKeyword, PrintHelpKeyword, std::string, std::string);
FORWARD_1(printHelpShowKinds, PrintHelpShowKinds, std::vector<std::string>);
FORWARD_1_RETURN(confirmDeletion, ConfirmDeletion, bool, boolean, Deska::Db::ObjectDefinition);
FORWARD_1_RETURN(confirmCreation, ConfirmCreation, bool, boolean, Deska::Db::ObjectDefinition);
FORWARD_1_RETURN(confirmRestoration, ConfirmRestoration, bool, boolean, Deska::Db::ObjectDefinition);
FORWARD_1_RETURN(chooseChangeset, ChooseChangeset, int, integer, std::vector<Deska::Db::PendingChangeset>);
FORWARD_1_RETURN(readLine, ReadLine, std::string, str1, std::string);
FORWARD_3(printHelpKind, PrintHelpKind, std::string, vect_pair_str_str, std::vector<std::string>);
FORWARD_0_RETURN(askForCommitMessage, AskForCommitMessage, std::string, str1);
FORWARD_0_RETURN(askForDetachMessage, AskForDetachMessage, std::string, str1);
FORWARD_3_OSTREAM(printAttributes, PrintAttributes, std::vector<Deska::Db::AttributeDefinition>, int);
FORWARD_4_OSTREAM(printObjects, PrintObjects, std::vector<Deska::Db::ObjectDefinition>, int, bool);


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


void CliTestFixture::expectPrintAttribute(const Deska::Db::AttributeDefinition &attribute, int indentLevel,
                                          std::ostream &out)
{
    cliEvents.push(MockCliEvent::printAttribute(attribute, indentLevel));
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
    if (!shouldPop)
        BOOST_REQUIRE_EQUAL(returner, e);
    bool match = e.myReturn(returner);
    BOOST_CHECK_MESSAGE(match, "Returning event in queue does not match current event.");
    if (!match) {
        BOOST_CHECK_EQUAL(MockCliEvent::invalid(), e);
        BOOST_REQUIRE_EQUAL(MockCliEvent::invalid(), returner);
    }
    if (shouldPop)
        cliEvents.pop();
    return returner;
}
