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

#ifndef DESKA_TEST_PARSERTESTFIXTURE_H
#define DESKA_TEST_PARSERTESTFIXTURE_H

#include <queue>
#include <boost/signals2/connection.hpp>
#include <boost/signals2/trackable.hpp>
#include "deska/db/Objects.h"
#include "MockParserEvent.h"

namespace Deska {
namespace Db {
class Api;
}
namespace Cli {
class Parser;
}
}

struct ParserTestFixture: public boost::signals2::trackable
{
    ParserTestFixture();

    ~ParserTestFixture();

    /** @short Handler for Parser's categoryEntered signal */
    void slotParserCategoryEntered(const Deska::Db::Identifier &kind, const Deska::Db::Identifier &name);

    /** @short Handler for Parser's categoryLeft() */
    void slotParserCategoryLeft();

    /** @short Handler for Parser's setAttr() signal */
    void slotParserSetAttr(const Deska::Db::Identifier &name, const Deska::Db::Value &val);
    
    /** @short Handler for Parser's removeAttr() signal */
    void slotParserRemoveAttr(const Deska::Db::Identifier &name);

    /** @short Handler for Parser's functionShow() */
    void slotParserFunctionShow();
    
    /** @short Handler for Parser's functionDelete() */
    void slotParserFunctionDelete();

    /** @short Handler for Parser's functionRename() */
    void slotParserFunctionRename(const Deska::Db::Identifier &newName);

    /** @short Handler for Parser's parserError() signal */
    void slotParserParseError(const Deska::Cli::ParserException &exception);
    
    /** @short Handler for Parser's parsingFinished() */
    void slotParserParsingFinished();

    /** @short Handler for Parser's parsingStarted() */
    void slotParserParsingStarted();

    /** @short Call this function to verify that no more events were logged */
    void expectNothingElse();

    /** @short Verify that the first signal which wasn't checked yet was the categoryEntered and that its argument match */
    void expectCategoryEntered(const Deska::Db::Identifier &kind, const Deska::Db::Identifier &name);

    /** @short Check for categoryLeft

    @see expectCategoryEntered()
    */
    void expectCategoryLeft();

    /** @short Check that the first signal which was not checked yet was the setAttr()

    @see expectCategoryEntered();
    */
    void expectSetAttr(const Deska::Db::Identifier &name, const Deska::Db::Value &val);
    
    /** @short Check that the first signal which was not checked yet was the removeAttr()

    @see expectCategoryEntered();
    */
    void expectRemoveAttr(const Deska::Db::Identifier &name);

    /** @short Check for functionShow

    @see expectCategoryEntered()
    */
    void expectFunctionShow();
    
    /** @short Check for functionDelete

    @see expectCategoryEntered()
    */
    void expectFunctionDelete();

    /** @short Check for functionRename

    @see expectCategoryEntered()
    */
    void expectFunctionRename(const Deska::Db::Identifier &newName);
    
    /** @short Check that the first signal which was not checked yet was the parseError, with corresponding arguments

    @see expectCategoryEntered();
    */
    void expectParseError(const Deska::Cli::ParserException &exception);
    
    /** @short Check for parsingFinished

    @see expectCategoryEntered()
    */
    void expectParsingFinished();

    /** @short Check for parsingStarted

    @see expectCategoryEntered()
    */
    void expectParsingStarted();

    /** @short Helper for various expect* functions */
    void expectHelper(const MockParserEvent &e);

    void verifyStackOneLevel(const Deska::Db::Identifier &kind, const Deska::Db::Identifier &name);

    void verifyStackTwoLevels(const Deska::Db::Identifier &kind1, const Deska::Db::Identifier &name1,
                              const Deska::Db::Identifier &kind2, const Deska::Db::Identifier &name2);

    void verifyEmptyStack();

    void slotParserSetAttrCheckContext();

    void slotParserRemoveAttrCheckContext();

    void slotParserDeleteCheckContext();

    void slotParserRenameCheckContext();

    Deska::Db::Api *db;
    Deska::Cli::Parser *parser; // we have to use a pointer because it has to be initialized at construction time :(
    std::queue<MockParserEvent> parserEvents;
    boost::signals2::connection attrCheckContextConnection;
    boost::signals2::connection attrRemoveCheckContextConnection;
    boost::signals2::connection deleteCheckContextConnection;
    boost::signals2::connection renameCheckContextConnection;
};

#endif // DESKA_TEST_PARSERTESTFIXTURE_H
