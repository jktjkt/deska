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


#ifndef DESKA_TEST_CLITESTFIXTURE_H
#define DESKA_TEST_CLITESTFIXTURE_H

#include <queue>
#include "deska/db/Connection.h"
#include "deska/cli/DbInteraction.h"
#include "deska/cli/ParserSignals.h"
#include "deska/cli/UserInterface.h"
#include "deska/cli/UserInterfaceIOBase.h"
#include "deska/cli/Parser.h"
#include "MockCliEvent.h"


namespace Deska
{

namespace Db
{
class Api;
}

namespace Cli
{
class Parser;
}

}

#define FORWARD_0_RETURN(FUNC, EFUNC, RET_TYPE, RET_VAR) RET_TYPE FUNC();
#define FORWARD_1(FUNC, EFUNC, TYPE_1) virtual void FUNC(boost::call_traits<TYPE_1>::param_type);
#define FORWARD_1_RETURN(FUNC, EFUNC, RET_TYPE, RET_VAR, TYPE_1) RET_TYPE FUNC(boost::call_traits<TYPE_1>::param_type);
#define FORWARD_2(FUNC, EFUNC, TYPE_1, TYPE_2) virtual void FUNC(boost::call_traits<TYPE_1>::param_type, boost::call_traits<TYPE_2>::param_type);
#define FORWARD_2_RETURN(FUNC, EFUNC, RET_TYPE, RET_VAR, TYPE_1, TYPE_2) RET_TYPE FUNC(boost::call_traits<TYPE_1>::param_type, boost::call_traits<TYPE_2>::param_type);
#define FORWARD_2_RAW_ARGS(FUNC, EFUNC, TYPE_1, TYPE_2) virtual void FUNC(TYPE_1, TYPE_2);
#define FORWARD_5(FUNC, EFUNC, TYPE_1, TYPE_2, TYPE_3, TYPE_4, TYPE_5) virtual void FUNC(boost::call_traits<TYPE_1>::param_type, boost::call_traits<TYPE_2>::param_type, boost::call_traits<TYPE_3>::param_type, boost::call_traits<TYPE_4>::param_type, boost::call_traits<TYPE_5>::param_type);
#define FORWARD_3_OSTREAM(FUNC, EFUNC, TYPE_1, TYPE_2) virtual void FUNC(boost::call_traits<TYPE_1>::param_type, boost::call_traits<TYPE_2>::param_type, std::ostream&);
#define FORWARD_4_OSTREAM(FUNC, EFUNC, TYPE_1, TYPE_2, TYPE_3) virtual void FUNC(boost::call_traits<TYPE_1>::param_type, boost::call_traits<TYPE_2>::param_type, boost::call_traits<TYPE_3>::param_type, std::ostream&);

class CliTestFixture;
class CliConfigTest;

/** @short Class for testing IO operations needed in a command line user interface with a standard iostream implementation.
*
*   All methods checks the events queue.
*
*   @see UserInterfaceIO
*/
class TestUserInterfaceIO: public Deska::Cli::UserInterfaceIOBase
{
public:
    TestUserInterfaceIO(CliTestFixture *cliTester);
    
    virtual ~TestUserInterfaceIO();
    
#include "CliFunctionDefinitions.h"
    
private:
    CliTestFixture *tester;
};

#define FORWARD_0_RETURN(FUNC, EFUNC, RET_TYPE, RET_VAR) \
    void expect##EFUNC(); \
    void return##EFUNC(boost::call_traits<RET_TYPE>::param_type);
#define FORWARD_1(FUNC, EFUNC, TYPE_1) void expect##EFUNC(boost::call_traits<TYPE_1>::param_type);
#define FORWARD_1_RETURN(FUNC, EFUNC, RET_TYPE, RET_VAR, TYPE_1) \
    void expect##EFUNC(boost::call_traits<TYPE_1>::param_type); \
    void return##EFUNC(boost::call_traits<RET_TYPE>::param_type);
#define FORWARD_2(FUNC, EFUNC, TYPE_1, TYPE_2) void expect##EFUNC(boost::call_traits<TYPE_1>::param_type, boost::call_traits<TYPE_2>::param_type);
#define FORWARD_2_RETURN(FUNC, EFUNC, RET_TYPE, RET_VAR, TYPE_1, TYPE_2) \
    void expect##EFUNC(boost::call_traits<TYPE_1>::param_type, boost::call_traits<TYPE_2>::param_type); \
    void return##EFUNC(boost::call_traits<RET_TYPE>::param_type);
#define FORWARD_2_RAW_ARGS(FUNC, EFUNC, TYPE_1, TYPE_2) void expect##EFUNC(TYPE_1, TYPE_2);
#define FORWARD_5(FUNC, EFUNC, TYPE_1, TYPE_2, TYPE_3, TYPE_4, TYPE_5) void expect##EFUNC(boost::call_traits<TYPE_1>::param_type, boost::call_traits<TYPE_2>::param_type, boost::call_traits<TYPE_3>::param_type, boost::call_traits<TYPE_4>::param_type, boost::call_traits<TYPE_5>::param_type);
#define FORWARD_3_OSTREAM(FUNC, EFUNC, TYPE_1, TYPE_2) void expect##EFUNC(boost::call_traits<TYPE_1>::param_type, boost::call_traits<TYPE_2>::param_type, std::ostream&);
#define FORWARD_4_OSTREAM(FUNC, EFUNC, TYPE_1, TYPE_2, TYPE_3) void expect##EFUNC(boost::call_traits<TYPE_1>::param_type, boost::call_traits<TYPE_2>::param_type, boost::call_traits<TYPE_3>::param_type, std::ostream&);

struct CliTestFixture
{
    CliTestFixture();

    ~CliTestFixture();

#include "CliFunctionDefinitions.h"
    
    void verifyEnd();
    
    /** @short Initializes the testing CLI and starts test. Events queue have to be filled befor calling this function. */
    void startTest();

    void expectCliInit();
    
    void expectHelper(const MockCliEvent &e);
    
    MockCliEvent returnHelper(const MockCliEvent &e);
    

    CliConfigTest *conf;
    Deska::Db::Connection *conn;
    Deska::Cli::Parser *parser;
    Deska::Cli::DbInteraction *db;
    Deska::Cli::UserInterfaceIOBase *io;
    Deska::Cli::UserInterface *ui;
    Deska::Cli::SignalsHandler *sh;
    
    std::queue<MockCliEvent> cliEvents;
    
    bool testStarted;
};

#endif // DESKA_TEST_CLITESTFIXTURE_H
