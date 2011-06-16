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

class CliTestFixture;

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
    
    virtual void reportError(const std::string &errorMessage);
    
    virtual void printMessage(const std::string &message);
    
    virtual bool confirmDeletion(const Deska::Db::ObjectDefinition &object);
    
    virtual bool confirmCreation(const Deska::Db::ObjectDefinition &object);
    
    virtual bool confirmRestoration(const Deska::Db::ObjectDefinition &object);
    
    virtual std::string askForCommitMessage();
    
    virtual std::string askForDetachMessage();
    
    virtual void printHelp(const std::map<std::string, std::string> &cliCommands,
                           const std::map<std::string, std::string> &parserKeywords);
                   
    virtual void printHelpCommand(const std::string &cmdName, const std::string &cmdDscr);
    
    virtual void printHelpKeyword(const std::string &keywordName, const std::string &keywordDscr);
    
    virtual void printHelpKind(const std::string &kindName,
                               const std::vector<std::pair<std::string, std::string> > &kindAttrs,
                               const std::vector<std::string> &nestedKinds);
                       
    virtual void printHelpShowKinds(const std::vector<std::string> &kinds);
    
    virtual int chooseChangeset(const std::vector<Deska::Db::PendingChangeset> &pendingChangesets);
    
    virtual std::string readLine(const std::string &prompt);
    
    virtual void printAttributes(const std::vector<Deska::Db::AttributeDefinition> &attributes, int indentLevel,
                                 std::ostream &out = std::cout);
                         
    virtual void printObjects(const std::vector<Deska::Db::ObjectDefinition> &objects, int indentLevel,
                              bool fullName, std::ostream &out = std::cout);
                      
    virtual void printAttribute(const Deska::Db::AttributeDefinition &attribute, int indentLevel,
                                std::ostream &out = std::cout);
                        
    virtual void printObject(const Deska::Db::ObjectDefinition &object, int indentLevel, bool fullName,
                             std::ostream &out = std::cout);
                     
    virtual void printEnd(int indentLevel, std::ostream &out = std::cout);
    
    virtual void addCommandCompletion(const std::string &completion);
    
private:
    CliTestFixture *tester;
};



struct CliTestFixture
{
    CliTestFixture();

    ~CliTestFixture();

    void expectReportError(const std::string &errorMessage);
    
    void expectPrintMessage(const std::string &message);
    
    void expectConfirmDeletion(const Deska::Db::ObjectDefinition &object);
    
    void returnConfirmDeletion(bool confirm);
    
    void expectConfirmCreation(const Deska::Db::ObjectDefinition &object);
    
    void returnConfirmCreation(bool confirm);
    
    void expectConfirmRestoration(const Deska::Db::ObjectDefinition &object);
    
    void returnConfirmRestoration(bool confirm);
    
    void expectAskForCommitMessage();
    
    void returnAskForCommitMessage(const std::string &message);
    
    void expectAskForDetachMessage();
    
    void returnAskForDetachMessage(const std::string &message);
    
    void expectPrintHelp(const std::map<std::string, std::string> &cliCommands,
                                       const std::map<std::string, std::string> &parserKeywords);
                                       
    void expectPrintHelpCommand(const std::string &cmdName, const std::string &cmdDscr);
    
    void expectPrintHelpKeyword(const std::string &keywordName, const std::string &keywordDscr);
    
    void expectPrintHelpKind(const std::string &kindName,
                                      const std::vector<std::pair<std::string, std::string> > &kindAttrs,
                                      const std::vector<std::string> &nestedKinds);
    
    void expectPrintHelpShowKinds(const std::vector<std::string> &kinds);
    
    void expectChooseChangeset(const std::vector<Deska::Db::PendingChangeset> &pendingChangesets);
    
    void returnChooseChangeset(int changeset);
    
    void expectReadLine(const std::string &prompt);
    
    void returnReadLine(const std::string &line);
    
    void expectPrintAttributes(const std::vector<Deska::Db::AttributeDefinition> &attributes, int indentLevel,
                                        std::ostream &out = std::cout);
    
    void expectPrintAttribute(const Deska::Db::AttributeDefinition &attribute, int indentLevel,
                                       std::ostream &out = std::cout);
    
    void expectPrintObjects(const std::vector<Deska::Db::ObjectDefinition> &objects, int indentLevel,
                                     bool fullName, std::ostream &out = std::cout);
    
    void expectPrintObject(const Deska::Db::ObjectDefinition &object, int indentLevel, bool fullName,
                                    std::ostream &out = std::cout);
    
    void expectPrintEnd(int indentLevel, std::ostream &out = std::cout);
    
    void expectAddCommandCompletion(const std::string &completion);
    
    void verifyEnd();
    
    /** @short Initializes the testing CLI and starts test. Events queue have to be filled befor calling this function. */
    void startTest();
    
    void expectHelper(const MockCliEvent &e);
    
    MockCliEvent returnHelper(const MockCliEvent &e);
    

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
