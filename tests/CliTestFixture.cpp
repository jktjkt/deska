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
#include "deska/cli/CliConfig.h"

// At first, define a few macros which specify how the code shall be generated

#define FORWARD_1(FUNC, EFUNC, TYPE_1) \
void TestUserInterfaceIO::FUNC(boost::call_traits<TYPE_1>::param_type arg1) { tester->expectHelper(MockCliEvent::FUNC(arg1)); } \
void CliTestFixture::expect##EFUNC(boost::call_traits<TYPE_1>::param_type arg1) { cliEvents.push(MockCliEvent::FUNC(arg1)); }

#define FORWARD_2_RAW_ARGS(FUNC, EFUNC, TYPE_1, TYPE_2) \
void TestUserInterfaceIO::FUNC(TYPE_1 arg1, TYPE_2 arg2) \
{ tester->expectHelper(MockCliEvent::FUNC(arg1, arg2)); } \
void CliTestFixture::expect##EFUNC(TYPE_1 arg1, TYPE_2 arg2) \
{ cliEvents.push(MockCliEvent::FUNC(arg1, arg2)); }

#define FORWARD_2(FUNC, EFUNC, TYPE_1, TYPE_2) \
    FORWARD_2_RAW_ARGS(FUNC, EFUNC, boost::call_traits<TYPE_1>::param_type, boost::call_traits<TYPE_2>::param_type)

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

#define FORWARD_2_RETURN(FUNC, EFUNC, RET_TYPE, RET_VAR, TYPE_1, TYPE_2) \
RET_TYPE TestUserInterfaceIO::FUNC(boost::call_traits<TYPE_1>::param_type arg1, boost::call_traits<TYPE_2>::param_type arg2) { \
    MockCliEvent event = MockCliEvent::FUNC(arg1, arg2); \
    tester->expectHelper(event); \
    MockCliEvent ret = tester->returnHelper(event); \
    return ret.RET_VAR; \
    } \
void CliTestFixture::expect##EFUNC(boost::call_traits<TYPE_1>::param_type arg1, boost::call_traits<TYPE_2>::param_type arg2) { cliEvents.push(MockCliEvent::FUNC(arg1, arg2)); } \
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

// Now include the function definitions, which will instantiate the required methods for us
#include "CliFunctionDefinitions.h"


TestUserInterfaceIO::TestUserInterfaceIO(CliTestFixture *cliTester)
{
    tester = cliTester;
}

TestUserInterfaceIO::~TestUserInterfaceIO()
{
}

CliTestFixture::CliTestFixture():
    conf(0), conn(0), parser(0), db(0), io(0), ui(0), sh(0), testStarted(false)
{
}

CliTestFixture::~CliTestFixture()
{
    delete sh;
    delete ui;
    delete io;
    delete db;
    delete parser;
    delete conn;
    delete conf;
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
    int argc = 0;
    char **argv = 0;
    testStarted = true;
    conf = new Deska::Cli::CliConfig("deska.ini", argc, argv);
    std::vector<std::string> args;
    args.push_back(std::string(CMAKE_CURRENT_SOURCE_DIR) + "/src/deska/server/app/deska-server");
    conn = new Deska::Db::Connection(args);
    parser = new Deska::Cli::Parser(conn);
    db = new Deska::Cli::DbInteraction(conn);
    io = new TestUserInterfaceIO(this);
    ui = new Deska::Cli::UserInterface(db, parser, io, conf);
    sh = new Deska::Cli::SignalsHandler(parser, ui);
    ui->run();
}



void CliTestFixture::expectCliInit()
{
    expectAddCommandCompletion("abort");
    expectAddCommandCompletion("backup %file");
    expectAddCommandCompletion("batch %file");
    expectAddCommandCompletion("commit");
    expectAddCommandCompletion("configdiff regenerate");
    expectAddCommandCompletion("context objects");
    expectAddCommandCompletion("detach");
    expectAddCommandCompletion("diff %file");
    expectAddCommandCompletion("dump %file");
    expectAddCommandCompletion("exit");
    expectAddCommandCompletion("quit");
    expectAddCommandCompletion("help kinds");
    expectAddCommandCompletion("help help");
    expectAddCommandCompletion("help abort");
    expectAddCommandCompletion("help backup");
    expectAddCommandCompletion("help batch");
    expectAddCommandCompletion("help commit");
    expectAddCommandCompletion("help configdiff");
    expectAddCommandCompletion("help context");
    expectAddCommandCompletion("help detach");
    expectAddCommandCompletion("help diff");
    expectAddCommandCompletion("help dump");
    expectAddCommandCompletion("help exit");
    expectAddCommandCompletion("help log");
    expectAddCommandCompletion("help quit");
    expectAddCommandCompletion("help rebase");
    expectAddCommandCompletion("help restore");
    expectAddCommandCompletion("help resume");
    expectAddCommandCompletion("help start");
    expectAddCommandCompletion("help status");
    expectAddCommandCompletion("help hardware");
    expectAddCommandCompletion("help hardware_template");
    expectAddCommandCompletion("help host");
    expectAddCommandCompletion("help host_template");
    expectAddCommandCompletion("help interface");
    expectAddCommandCompletion("help interface_template");
    expectAddCommandCompletion("help service");
    expectAddCommandCompletion("help vendor");
    expectAddCommandCompletion("help virtual_hardware");
    expectAddCommandCompletion("help add");
    expectAddCommandCompletion("help all");
    expectAddCommandCompletion("help create");
    expectAddCommandCompletion("help delete");
    expectAddCommandCompletion("help end");
    expectAddCommandCompletion("help last");
    expectAddCommandCompletion("help new");
    expectAddCommandCompletion("help no");
    expectAddCommandCompletion("help remove");
    expectAddCommandCompletion("help show");
    expectAddCommandCompletion("log");
    expectAddCommandCompletion("exit");
    expectAddCommandCompletion("quit");
    expectAddCommandCompletion("rebase");
    expectAddCommandCompletion("restore %file");
    expectAddCommandCompletion("resume");
    expectAddCommandCompletion("start");
    expectAddCommandCompletion("status");
    expectPrintMessage("Deska CLI started. For usage info try typing \"help\".");
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
