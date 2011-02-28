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
#include "deska/db/Api.h"
#include "MockParserEvent.h"

namespace Deska {
namespace CLI {
class Parser;
}
}

struct F: public boost::signals2::trackable
{
    F();

    ~F();

    /** @short Handler for Parser's categoryEntered signal */
    void slotParserCategoryEntered(const Deska::Identifier &kind, const Deska::Identifier &name);

    /** @short Handler for Parser's categoryLeft() */
    void slotParserCategoryLeft();

    /** @short Handler for Parser's setAttr() signal */
    void slotParserSetAttr(const Deska::Identifier &name, const Deska::Value &val);

    /** @short Call this function to verify that no more events were logged */
    void expectNothingElse();

    /** @short Verify that the first signal which wasn't checked yet was the categoryEntered and that its argument match */
    void expectCategoryEntered(const Deska::Identifier &kind, const Deska::Identifier &name);

    /** @short Check for categoryLeft

    @see expectCategoryEntered()
    */
    void expectCategoryLeft();

    /** @short Check that the first signal which was not checked yet was the setAttr()

    @see expectCategoryEntered();
    */
    void expectSetAttr(const Deska::Identifier &name, const Deska::Value &val);

    /** @short Helper for various expect* functions */
    void expectHelper(const MockParserEvent &e);

    void verifyStackOneLevel(const Deska::Identifier &kind, const Deska::Identifier &name);

    void verifyEmptyStack();

    void slotParserSetAttrCheckContext();

    Deska::Api *db;
    Deska::CLI::Parser *parser; // we have to use a pointer because it has to be initialized at construction time :(
    std::queue<MockParserEvent> parserEvents;
    boost::signals2::connection attrCheckContextConnection;
};

#endif // DESKA_TEST_PARSERTESTFIXTURE_H
