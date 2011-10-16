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
#include <boost/date_time/posix_time/posix_time.hpp>
#include "fix_boost_test_sigchld.h"
#include "CliTestFixture.h"


/** @short Check CLI start and quit */
BOOST_FIXTURE_TEST_CASE(start_quit, CliTestFixture)
{
    expectAddCommandCompletion("abort");
    expectAddCommandCompletion("commit");
    expectAddCommandCompletion("configdiff regenerate");
    expectAddCommandCompletion("context objects");
    expectAddCommandCompletion("detach");
    expectAddCommandCompletion("diff");
    expectAddCommandCompletion("dump %file");
    expectAddCommandCompletion("exit");
    expectAddCommandCompletion("quit");
    expectAddCommandCompletion("help kinds");
    expectAddCommandCompletion("help help");
    expectAddCommandCompletion("help abort");
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
    expectAddCommandCompletion("help add");
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
    expectReadLine("");
    returnReadLine(std::make_pair<std::string, bool>("quit", false));
    startTest();
    verifyEnd();
}



/** @short Check CLI start and creating new changeset */
BOOST_FIXTURE_TEST_CASE(changesets, CliTestFixture)
{
    expectAddCommandCompletion("abort");
    expectAddCommandCompletion("commit");
    expectAddCommandCompletion("configdiff regenerate");
    expectAddCommandCompletion("context objects");
    expectAddCommandCompletion("detach");
    expectAddCommandCompletion("diff");
    expectAddCommandCompletion("dump %file");
    expectAddCommandCompletion("exit");
    expectAddCommandCompletion("quit");
    expectAddCommandCompletion("help kinds");
    expectAddCommandCompletion("help help");
    expectAddCommandCompletion("help abort");
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
    expectAddCommandCompletion("help add");
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
    expectReadLine("");
    returnReadLine(std::make_pair<std::string, bool>("start", false));
    expectPrintMessage("Changeset tmp1 started.");
    expectReadLine("");
    returnReadLine(std::make_pair<std::string, bool>("start", false));
    expectReportError("Error: You are already in the changeset tmp1!");
    expectReadLine("");
    returnReadLine(std::make_pair<std::string, bool>("abort", false));
    expectPrintMessage("Changeset tmp1 aborted.");
    expectReadLine("");
    returnReadLine(std::make_pair<std::string, bool>("start", false));
    expectPrintMessage("Changeset tmp2 started.");
    expectReadLine("");
    returnReadLine(std::make_pair<std::string, bool>("detach", false));
    expectAskForDetachMessage();
    returnAskForDetachMessage("Testing message");
    expectPrintMessage("Changeset tmp2 detached.");
    expectReadLine("");
    returnReadLine(std::make_pair<std::string, bool>("resume", false));
    std::vector<Deska::Db::PendingChangeset> changesets;
    changesets.push_back(Deska::Db::PendingChangeset(Deska::Db::TemporaryChangesetId(2), "foobar", 
        boost::posix_time::time_from_string("2011-Sep-17 12:34:56.789101"), Deska::Db::RevisionId(1), "Testing message",
        Deska::Db::PendingChangeset::ATTACH_DETACHED, boost::optional<std::string>()));
    expectChooseChangeset(changesets);
    returnChooseChangeset(0);
    expectPrintMessage("Changeset tmp2 resumed.");
    expectReadLine("");
    returnReadLine(std::make_pair<std::string, bool>("quit", false));
    startTest();
    verifyEnd();
}

/** @short Objects creating */
BOOST_FIXTURE_TEST_CASE(creating_objects, CliTestFixture)
{
    expectAddCommandCompletion("abort");
    expectAddCommandCompletion("commit");
    expectAddCommandCompletion("configdiff regenerate");
    expectAddCommandCompletion("context objects");
    expectAddCommandCompletion("detach");
    expectAddCommandCompletion("diff");
    expectAddCommandCompletion("dump %file");
    expectAddCommandCompletion("exit");
    expectAddCommandCompletion("quit");
    expectAddCommandCompletion("help kinds");
    expectAddCommandCompletion("help help");
    expectAddCommandCompletion("help abort");
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
    expectAddCommandCompletion("help add");
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
    expectReadLine("");
    returnReadLine(std::make_pair<std::string, bool>("resume", false));
    std::vector<Deska::Db::PendingChangeset> changesets;
    changesets.push_back(Deska::Db::PendingChangeset(Deska::Db::TemporaryChangesetId(2), "foobar", 
        boost::posix_time::time_from_string("2011-Sep-17 12:34:56.789101"), Deska::Db::RevisionId(1), "Testing message",
        Deska::Db::PendingChangeset::ATTACH_IN_PROGRESS, boost::optional<std::string>()));
    expectChooseChangeset(changesets);
    returnChooseChangeset(0);
    expectPrintMessage("Changeset tmp2 resumed.");
    expectReadLine("");
    returnReadLine(std::make_pair<std::string, bool>("hardware hpv2", false));
    expectConfirmCreation(Deska::Cli::ObjectDefinition("hardware", "hpv2"));
    returnConfirmCreation(true);
    expectReadLine("hardware hpv2");
    returnReadLine(std::make_pair<std::string, bool>("end", false));
    expectReadLine("");
    returnReadLine(std::make_pair<std::string, bool>("hardware hpv2", false));
    expectReadLine("hardware hpv2");
    returnReadLine(std::make_pair<std::string, bool>("show", false));
    std::vector<std::pair<Deska::Cli::AttributeDefinition, Deska::Db::Identifier> > attrs;
    attrs.push_back(std::make_pair<Deska::Cli::AttributeDefinition, Deska::Db::Identifier>(
        Deska::Cli::AttributeDefinition("cpu_ht", Deska::Db::Value()), Deska::Db::Identifier()));
    attrs.push_back(std::make_pair<Deska::Cli::AttributeDefinition, Deska::Db::Identifier>(
        Deska::Cli::AttributeDefinition("cpu_num", Deska::Db::Value()), Deska::Db::Identifier()));
    attrs.push_back(std::make_pair<Deska::Cli::AttributeDefinition, Deska::Db::Identifier>(
        Deska::Cli::AttributeDefinition("host", Deska::Db::Value()), Deska::Db::Identifier()));
    attrs.push_back(std::make_pair<Deska::Cli::AttributeDefinition, Deska::Db::Identifier>(
        Deska::Cli::AttributeDefinition("note_hardware", Deska::Db::Value()), Deska::Db::Identifier()));
    attrs.push_back(std::make_pair<Deska::Cli::AttributeDefinition, Deska::Db::Identifier>(
        Deska::Cli::AttributeDefinition("purchase", Deska::Db::Value()), Deska::Db::Identifier()));
    attrs.push_back(std::make_pair<Deska::Cli::AttributeDefinition, Deska::Db::Identifier>(
        Deska::Cli::AttributeDefinition("ram", Deska::Db::Value()), Deska::Db::Identifier()));
    attrs.push_back(std::make_pair<Deska::Cli::AttributeDefinition, Deska::Db::Identifier>(
        Deska::Cli::AttributeDefinition("template_hardware", Deska::Db::Value()), Deska::Db::Identifier()));
    attrs.push_back(std::make_pair<Deska::Cli::AttributeDefinition, Deska::Db::Identifier>(
        Deska::Cli::AttributeDefinition("vendor", Deska::Db::Value()), Deska::Db::Identifier()));
    attrs.push_back(std::make_pair<Deska::Cli::AttributeDefinition, Deska::Db::Identifier>(
        Deska::Cli::AttributeDefinition("warranty", Deska::Db::Value()), Deska::Db::Identifier()));
    expectPrintAttributesWithOrigin(attrs, 0, std::cout);
    expectReadLine("hardware hpv2");
    returnReadLine(std::make_pair<std::string, bool>("quit", false));
    startTest();
    verifyEnd();
}

/** @short Attributes setting */
BOOST_FIXTURE_TEST_CASE(setting_attributes, CliTestFixture)
{
    expectAddCommandCompletion("abort");
    expectAddCommandCompletion("commit");
    expectAddCommandCompletion("configdiff regenerate");
    expectAddCommandCompletion("context objects");
    expectAddCommandCompletion("detach");
    expectAddCommandCompletion("diff");
    expectAddCommandCompletion("dump %file");
    expectAddCommandCompletion("exit");
    expectAddCommandCompletion("quit");
    expectAddCommandCompletion("help kinds");
    expectAddCommandCompletion("help help");
    expectAddCommandCompletion("help abort");
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
    expectAddCommandCompletion("help add");
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
    expectReadLine("");
    returnReadLine(std::make_pair<std::string, bool>("resume", false));
    std::vector<Deska::Db::PendingChangeset> changesets;
    changesets.push_back(Deska::Db::PendingChangeset(Deska::Db::TemporaryChangesetId(2), "foobar", 
        boost::posix_time::time_from_string("2011-Sep-17 12:34:56.789101"), Deska::Db::RevisionId(1), "Testing message",
        Deska::Db::PendingChangeset::ATTACH_IN_PROGRESS, boost::optional<std::string>()));
    expectChooseChangeset(changesets);
    returnChooseChangeset(0);
    expectPrintMessage("Changeset tmp2 resumed.");
    expectReadLine("");
    returnReadLine(std::make_pair<std::string, bool>("hardware hpv2 cpu_ht true cpu_num 2 note_hardware \"Some note\"", false));
    expectReadLine("");
    returnReadLine(std::make_pair<std::string, bool>("show hardware hpv2", false));
    std::vector<std::pair<Deska::Cli::AttributeDefinition, Deska::Db::Identifier> > attrs;
    attrs.push_back(std::make_pair<Deska::Cli::AttributeDefinition, Deska::Db::Identifier>(
        Deska::Cli::AttributeDefinition("cpu_ht", Deska::Db::Value(true)), Deska::Db::Identifier()));
    attrs.push_back(std::make_pair<Deska::Cli::AttributeDefinition, Deska::Db::Identifier>(
        Deska::Cli::AttributeDefinition("cpu_num", Deska::Db::Value(2)), Deska::Db::Identifier()));
    attrs.push_back(std::make_pair<Deska::Cli::AttributeDefinition, Deska::Db::Identifier>(
        Deska::Cli::AttributeDefinition("host", Deska::Db::Value()), Deska::Db::Identifier()));
    attrs.push_back(std::make_pair<Deska::Cli::AttributeDefinition, Deska::Db::Identifier>(
        Deska::Cli::AttributeDefinition("note_hardware", Deska::Db::Value(std::string("Some note"))), Deska::Db::Identifier()));
    attrs.push_back(std::make_pair<Deska::Cli::AttributeDefinition, Deska::Db::Identifier>(
        Deska::Cli::AttributeDefinition("purchase", Deska::Db::Value()), Deska::Db::Identifier()));
    attrs.push_back(std::make_pair<Deska::Cli::AttributeDefinition, Deska::Db::Identifier>(
        Deska::Cli::AttributeDefinition("ram", Deska::Db::Value()), Deska::Db::Identifier()));
    attrs.push_back(std::make_pair<Deska::Cli::AttributeDefinition, Deska::Db::Identifier>(
        Deska::Cli::AttributeDefinition("template_hardware", Deska::Db::Value()), Deska::Db::Identifier()));
    attrs.push_back(std::make_pair<Deska::Cli::AttributeDefinition, Deska::Db::Identifier>(
        Deska::Cli::AttributeDefinition("vendor", Deska::Db::Value()), Deska::Db::Identifier()));
    attrs.push_back(std::make_pair<Deska::Cli::AttributeDefinition, Deska::Db::Identifier>(
        Deska::Cli::AttributeDefinition("warranty", Deska::Db::Value()), Deska::Db::Identifier()));
    expectPrintAttributesWithOrigin(attrs, 0, std::cout);
    expectReadLine("");
    returnReadLine(std::make_pair<std::string, bool>("quit", false));
    startTest();
    verifyEnd();
}

/** @short Connecting objects */
BOOST_FIXTURE_TEST_CASE(objects_connecting, CliTestFixture)
{
    expectAddCommandCompletion("abort");
    expectAddCommandCompletion("commit");
    expectAddCommandCompletion("configdiff regenerate");
    expectAddCommandCompletion("context objects");
    expectAddCommandCompletion("detach");
    expectAddCommandCompletion("diff");
    expectAddCommandCompletion("dump %file");
    expectAddCommandCompletion("exit");
    expectAddCommandCompletion("quit");
    expectAddCommandCompletion("help kinds");
    expectAddCommandCompletion("help help");
    expectAddCommandCompletion("help abort");
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
    expectAddCommandCompletion("help add");
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
    expectReadLine("");
    returnReadLine(std::make_pair<std::string, bool>("resume", false));
    std::vector<Deska::Db::PendingChangeset> changesets;
    changesets.push_back(Deska::Db::PendingChangeset(Deska::Db::TemporaryChangesetId(2), "foobar", 
        boost::posix_time::time_from_string("2011-Sep-17 12:34:56.789101"), Deska::Db::RevisionId(1), "Testing message",
        Deska::Db::PendingChangeset::ATTACH_IN_PROGRESS, boost::optional<std::string>()));
    expectChooseChangeset(changesets);
    returnChooseChangeset(0);
    expectPrintMessage("Changeset tmp2 resumed.");
    expectReadLine("");
    returnReadLine(std::make_pair<std::string, bool>("host hpv2", false));
    std::vector<Deska::Cli::ObjectDefinition> merged;
    merged.push_back(Deska::Cli::ObjectDefinition("hardware", "hpv2"));
    expectConfirmCreationConnection2(Deska::Cli::ObjectDefinition("host", "hpv2"), merged);
    returnConfirmCreationConnection2(true);
    expectReadLine("host hpv2");
    returnReadLine(std::make_pair<std::string, bool>("show hardware hpv2", false));
    returnReadLine(std::make_pair<std::string, bool>("quit", false));
    startTest();
    verifyEnd();
}
