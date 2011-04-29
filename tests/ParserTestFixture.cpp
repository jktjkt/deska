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

#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/test/test_tools.hpp>

#include "ParserTestFixture.h"
#include "deska/cli/Parser.h"
#include "deska/db/FakeApi.h"

ParserTestFixture::ParserTestFixture()
{
    using namespace Deska::Db;
    using boost::phoenix::bind;
    using boost::phoenix::arg_names::_1;
    using boost::phoenix::arg_names::_2;

    FakeApi *fake = new FakeApi();
    fake->attrs["hardware"].push_back( KindAttributeDataType( "id", TYPE_INT ) );
    fake->attrs["hardware"].push_back( KindAttributeDataType( "name", TYPE_STRING ) );
    fake->attrs["hardware"].push_back( KindAttributeDataType( "price", TYPE_DOUBLE ) );
    fake->attrs["interface"].push_back( KindAttributeDataType( "ip", TYPE_STRING ) );
    fake->attrs["interface"].push_back( KindAttributeDataType( "mac", TYPE_STRING ) );
    fake->attrs["host"].push_back( KindAttributeDataType( "hardware_id", TYPE_IDENTIFIER ) );
    fake->attrs["host"].push_back( KindAttributeDataType( "name", TYPE_STRING ) );

    fake->relations["interface"].push_back( ObjectRelation::embedInto("host") );
    db = fake;

    parser = new Deska::Cli::Parser(db);
    parser->categoryEntered.connect(bind(&ParserTestFixture::slotParserCategoryEntered, this, _1, _2));
    parser->categoryLeft.connect(bind(&ParserTestFixture::slotParserCategoryLeft, this));
    // this one has to be fully qualified to prevent ambiguity...
    parser->attributeSet.connect(boost::phoenix::bind(&ParserTestFixture::slotParserSetAttr, this, _1, _2));
    attrCheckContextConnection = parser->attributeSet.connect(bind(&ParserTestFixture::slotParserSetAttrCheckContext, this));
    parser->functionShow.connect(bind(&ParserTestFixture::slotParserFunctionShow, this));
    parser->functionDelete.connect(bind(&ParserTestFixture::slotParserFunctionDelete, this));
    parser->parseError.connect(bind(&ParserTestFixture::slotParserParseError, this, _1));
    parser->parsingFinished.connect(bind(&ParserTestFixture::slotParserParsingFinished, this));
}

ParserTestFixture::~ParserTestFixture()
{
    delete parser;
    delete db;
}

void ParserTestFixture::slotParserCategoryEntered(const Deska::Db::Identifier &kind, const Deska::Db::Identifier &name)
{
    parserEvents.push(MockParserEvent::categoryEntered(kind, name));
}

void ParserTestFixture::slotParserCategoryLeft()
{
    parserEvents.push(MockParserEvent::categoryLeft());
}

void ParserTestFixture::slotParserSetAttr(const Deska::Db::Identifier &name, const Deska::Db::Value &val)
{
    parserEvents.push(MockParserEvent::setAttr(name, val));
}

void ParserTestFixture::slotParserFunctionShow()
{
    parserEvents.push(MockParserEvent::functionShow());
}

void ParserTestFixture::slotParserFunctionDelete()
{
    parserEvents.push(MockParserEvent::functionDelete());
}

void ParserTestFixture::slotParserParseError(const Deska::Cli::ParserException &exception)
{
    parserEvents.push(MockParserEvent::parserError(exception));
}

void ParserTestFixture::slotParserParsingFinished()
{
    parserEvents.push(MockParserEvent::parsingFinished());
}

void ParserTestFixture::expectNothingElse()
{
    BOOST_CHECK_MESSAGE(parserEvents.empty(), "Expected no more emitted signals");
    if (!parserEvents.empty()) {
        // show the first queued event here
        BOOST_CHECK_EQUAL(MockParserEvent::invalid(), parserEvents.front());
    }
}

void ParserTestFixture::expectCategoryEntered(const Deska::Db::Identifier &kind, const Deska::Db::Identifier &name)
{
    expectHelper(MockParserEvent::categoryEntered(kind, name));
}

void ParserTestFixture::expectCategoryLeft()
{
    expectHelper(MockParserEvent::categoryLeft());
}

void ParserTestFixture::expectSetAttr(const Deska::Db::Identifier &name, const Deska::Db::Value &val)
{
    expectHelper(MockParserEvent::setAttr(name, val));
}

void ParserTestFixture::expectFunctionShow()
{
    expectHelper(MockParserEvent::functionShow());
}

void ParserTestFixture::expectFunctionDelete()
{
    expectHelper(MockParserEvent::functionDelete());
}

void ParserTestFixture::expectParseError(const Deska::Cli::ParserException &exception)
{
    expectHelper(MockParserEvent::parserError(exception));
}

void ParserTestFixture::expectParsingFinished()
{
    expectHelper(MockParserEvent::parsingFinished());
}

void ParserTestFixture::expectHelper(const MockParserEvent &e)
{
    BOOST_CHECK( ! parserEvents.empty() );
    bool shouldPop = ! parserEvents.empty();
    MockParserEvent other = shouldPop ? parserEvents.front() : MockParserEvent::invalid();
    BOOST_CHECK_EQUAL(e, other);
    if ( shouldPop )
        parserEvents.pop();
}

void ParserTestFixture::verifyStackOneLevel(const Deska::Db::Identifier &kind, const Deska::Db::Identifier &name)
{
    const std::vector<Deska::Cli::ContextStackItem> &stack = parser->currentContextStack();
    std::vector<Deska::Cli::ContextStackItem> specimen;
    specimen.push_back(Deska::Cli::ContextStackItem(kind, name));
    BOOST_CHECK(parser->isNestedInContext());
    BOOST_CHECK_EQUAL_COLLECTIONS(stack.begin(), stack.end(), specimen.begin(), specimen.end());
}

void ParserTestFixture::verifyStackTwoLevels(const Deska::Db::Identifier &kind1, const Deska::Db::Identifier &name1,
                             const Deska::Db::Identifier &kind2, const Deska::Db::Identifier &name2)
{
    const std::vector<Deska::Cli::ContextStackItem> &stack = parser->currentContextStack();
    std::vector<Deska::Cli::ContextStackItem> specimen;
    specimen.push_back(Deska::Cli::ContextStackItem(kind1, name1));
    specimen.push_back(Deska::Cli::ContextStackItem(kind2, name2));
    BOOST_CHECK(parser->isNestedInContext());
    BOOST_CHECK_EQUAL_COLLECTIONS(stack.begin(), stack.end(), specimen.begin(), specimen.end());
}

void ParserTestFixture::verifyEmptyStack()
{
    BOOST_CHECK( ! parser->isNestedInContext() );
    BOOST_CHECK( parser->currentContextStack().empty() );
}

void ParserTestFixture::slotParserSetAttrCheckContext()
{
    BOOST_CHECK_MESSAGE(parser->isNestedInContext(), "Parser has to be nested in a context in order to set attributes");
}
