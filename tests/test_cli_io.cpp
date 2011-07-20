/*
* Copyright (C) 2011 Tomáš Hubík <hubik.tomas@gmail.com>
* Copyright (C) 2011 Jan Kundrát <kundratj@fzu.cz>
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

#define BOOST_TEST_MODULE cli_io
#include "fix_boost_test_sigchld.h"
#include "CliTestFixture.h"


/** @short Check CLI start and quit */
BOOST_FIXTURE_TEST_CASE( start_quit, CliTestFixture )
{
    expectAddCommandCompletion("abort");
    expectAddCommandCompletion("commit");
    expectAddCommandCompletion("detach");
    expectAddCommandCompletion("diff");
    expectAddCommandCompletion("dump %file");
    expectAddCommandCompletion("exit");
    expectAddCommandCompletion("quit");
    expectAddCommandCompletion("help kinds");
    expectAddCommandCompletion("help help");
    expectAddCommandCompletion("help abort");
    expectAddCommandCompletion("help commit");
    expectAddCommandCompletion("help detach");
    expectAddCommandCompletion("help diff");
    expectAddCommandCompletion("help dump");
    expectAddCommandCompletion("help exit");
    expectAddCommandCompletion("help log");
    expectAddCommandCompletion("help quit");
    expectAddCommandCompletion("help restore");
    expectAddCommandCompletion("help resume");
    expectAddCommandCompletion("help start");
    expectAddCommandCompletion("help status");
    expectAddCommandCompletion("help hardware");
    expectAddCommandCompletion("help hardware_template");
    expectAddCommandCompletion("help host");
    expectAddCommandCompletion("help interface");
    expectAddCommandCompletion("help interface_template");
    expectAddCommandCompletion("help vendor");
    expectAddCommandCompletion("help delete");
    expectAddCommandCompletion("help end");
    expectAddCommandCompletion("help no");
    expectAddCommandCompletion("help show");
    expectAddCommandCompletion("log");
    expectAddCommandCompletion("exit");
    expectAddCommandCompletion("quit");
    expectAddCommandCompletion("restore %file");
    expectAddCommandCompletion("resume");
    expectAddCommandCompletion("start");
    expectAddCommandCompletion("status");
    expectPrintMessage("Deska CLI started. For usage info try typing \"help\".");
    expectReadLine("");
    returnReadLine("quit");
    startTest();
    verifyEnd();
}
