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
#include <boost/bind.hpp>

#include "ParserTestFixture.h"
#include "deska/cli/Parser.h"
#include "deska/db/FakeApi.h"

F::F()
{
    using namespace Deska;
    Deska::FakeApi *fake = new FakeApi();
    fake->attrs["hardware"].push_back( KindAttributeDataType( "id", TYPE_INT ) );
    fake->attrs["hardware"].push_back( KindAttributeDataType( "name", TYPE_STRING ) );
    fake->attrs["hardware"].push_back( KindAttributeDataType( "price", TYPE_DOUBLE ) );
    fake->attrs["interface"].push_back( KindAttributeDataType( "ip", TYPE_STRING ) );
    fake->attrs["interface"].push_back( KindAttributeDataType( "mac", TYPE_STRING ) );
    fake->attrs["host"].push_back( KindAttributeDataType( "hardware_id", TYPE_IDENTIFIER ) );
    fake->attrs["host"].push_back( KindAttributeDataType( "name", TYPE_STRING ) );

    fake->relations["interface"].push_back( ObjectRelation::embedInto("host") );
    db = fake;

    parser = new Deska::CLI::Parser(db);
    parser->categoryEntered.connect(boost::bind(&F::slotParserCategoryEntered, this, _1, _2));
    parser->categoryLeft.connect(boost::bind(&F::slotParserCategoryLeft, this));
    parser->attributeSet.connect(boost::bind(&F::slotParserSetAttr, this, _1, _2));
    attrCheckContextConnection = parser->attributeSet.connect(boost::bind(&F::slotParserSetAttrCheckContext, this));
}

F::~F()
{
    delete parser;
    delete db;
}

void F::slotParserCategoryEntered(const Deska::Identifier &kind, const Deska::Identifier &name)
{
    parserEvents.push(MockParserEvent::categoryEntered(kind, name));
}

void F::slotParserCategoryLeft()
{
    parserEvents.push(MockParserEvent::categoryLeft());
}

void F::slotParserSetAttr(const Deska::Identifier &name, const Deska::Value &val)
{
    parserEvents.push(MockParserEvent::setAttr(name, val));
}

void F::expectNothingElse()
{
    BOOST_CHECK_MESSAGE(parserEvents.empty(), "Expected no more emitted signals");
}

void F::expectCategoryEntered(const Deska::Identifier &kind, const Deska::Identifier &name)
{
    expectHelper(MockParserEvent::categoryEntered(kind, name));
}

void F::expectCategoryLeft()
{
    expectHelper(MockParserEvent::categoryLeft());
}

void F::expectSetAttr(const Deska::Identifier &name, const Deska::Value &val)
{
    expectHelper(MockParserEvent::setAttr(name, val));
}

void F::expectHelper(const MockParserEvent &e)
{
    // We would like to continue with the test suite after hitting the first error, and
    // BOOST_REQUIRE doesn't allow masking via BOOST_AUTO_TEST_CASE_EXPECTED_FAILURES...
    BOOST_CHECK( ! parserEvents.empty() );
    bool shouldPop = ! parserEvents.empty();
    MockParserEvent other = shouldPop ? parserEvents.front() : MockParserEvent::invalid();
    BOOST_CHECK_EQUAL(e, other);
    if ( shouldPop )
        parserEvents.pop();
}

void F::verifyStackOneLevel(const Deska::Identifier &kind, const Deska::Identifier &name)
{
    const std::vector<Deska::CLI::ContextStackItem> &stack = parser->currentContextStack();
    std::vector<Deska::CLI::ContextStackItem> specimen;
    specimen.push_back(Deska::CLI::ContextStackItem(kind, name));
    BOOST_REQUIRE_EQUAL(stack.size(), specimen.size());
    BOOST_CHECK(parser->isNestedInContext());
    BOOST_CHECK_EQUAL_COLLECTIONS(stack.begin(), stack.end(), specimen.begin(), specimen.end());
}

void F::verifyEmptyStack()
{
    BOOST_CHECK( ! parser->isNestedInContext() );
    BOOST_CHECK( parser->currentContextStack().empty() );
}

void F::slotParserSetAttrCheckContext()
{
    BOOST_CHECK_MESSAGE(parser->isNestedInContext(), "Parser has to be nested in a context in order to set attributes");
}
