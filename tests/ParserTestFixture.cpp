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
    using boost::phoenix::arg_names::_3;

    FakeApi *fake = new FakeApi();
    fake->attrs["hardware"].push_back( KindAttributeDataType( "id", TYPE_INT ) );
    fake->attrs["hardware"].push_back( KindAttributeDataType( "hardware_name", TYPE_STRING ) );
    fake->attrs["hardware"].push_back( KindAttributeDataType( "price", TYPE_DOUBLE ) );
    fake->attrs["interface"].push_back( KindAttributeDataType( "ip", TYPE_IPV4_ADDRESS ) );
    fake->attrs["interface"].push_back( KindAttributeDataType( "mac", TYPE_MAC_ADDRESS ) );
    fake->attrs["host"].push_back( KindAttributeDataType( "hardware_id", TYPE_IDENTIFIER ) );
    fake->attrs["host"].push_back( KindAttributeDataType( "host_name", TYPE_STRING ) );
    fake->attrs["host"].push_back( KindAttributeDataType( "role", TYPE_IDENTIFIER_SET ) );

    fake->relations["interface"].push_back( ObjectRelation::embedInto("host", "pocitac") );
    fake->relations["host"].push_back( ObjectRelation::contains("hardware", "hardware") );
    fake->relations["hardware"].push_back( ObjectRelation::containable("host", "host") );
    db = fake;

    parser = new Deska::Cli::Parser(db);
    parser->createObject.connect(bind(&ParserTestFixture::slotParserCreateObject, this, _1, _2));
    parser->categoryEntered.connect(bind(&ParserTestFixture::slotParserCategoryEntered, this, _1, _2));
    parser->categoryLeft.connect(bind(&ParserTestFixture::slotParserCategoryLeft, this));
    // this one has to be fully qualified to prevent ambiguity...
    parser->attributeSet.connect(boost::phoenix::bind(&ParserTestFixture::slotParserSetAttr, this, _1, _2, _3));  
    attrSetCheckContextConnection = parser->attributeSet.connect(bind(&ParserTestFixture::slotParserSetAttrCheckContext, this));
    parser->attributeSetInsert.connect(boost::phoenix::bind(&ParserTestFixture::slotParserSetAttrInsert, this, _1, _2, _3));
    attrSetInsertCheckContextConnection = parser->attributeSetInsert.connect(bind(&ParserTestFixture::slotParserSetAttrInsertCheckContext, this));
    parser->attributeSetRemove.connect(boost::phoenix::bind(&ParserTestFixture::slotParserSetAttrRemove, this, _1, _2, _3));
    attrSetRemoveCheckContextConnection = parser->attributeSetRemove.connect(bind(&ParserTestFixture::slotParserSetAttrRemoveCheckContext, this));
    parser->attributeRemove.connect(boost::phoenix::bind(&ParserTestFixture::slotParserRemoveAttr, this, _1, _2));
    attrRemoveCheckContextConnection = parser->attributeRemove.connect(bind(&ParserTestFixture::slotParserRemoveAttrCheckContext, this));
    parser->functionShow.connect(bind(&ParserTestFixture::slotParserFunctionShow, this));
    parser->functionDelete.connect(bind(&ParserTestFixture::slotParserFunctionDelete, this));
    deleteCheckContextConnection = parser->functionDelete.connect(bind(&ParserTestFixture::slotParserDeleteCheckContext, this));
    parser->functionRename.connect(boost::phoenix::bind(&ParserTestFixture::slotParserFunctionRename, this, _1));
    renameCheckContextConnection = parser->functionRename.connect(bind(&ParserTestFixture::slotParserRenameCheckContext, this));
    parser->objectsFilter.connect(boost::phoenix::bind(&ParserTestFixture::slotParserObjectsFilter, this, _1, _2));
    parser->parseError.connect(bind(&ParserTestFixture::slotParserParseError, this, _1));
    parser->parsingFinished.connect(bind(&ParserTestFixture::slotParserParsingFinished, this));
    parser->parsingStarted.connect(bind(&ParserTestFixture::slotParserParsingStarted, this));
}

ParserTestFixture::~ParserTestFixture()
{
    delete parser;
    delete db;
}

void ParserTestFixture::slotParserCreateObject(const Deska::Db::Identifier &kind, const Deska::Db::Identifier &name)
{
    parserEvents.push(MockParserEvent::createObject(kind, name));
}

void ParserTestFixture::slotParserCategoryEntered(const Deska::Db::Identifier &kind, const Deska::Db::Identifier &name)
{
    parserEvents.push(MockParserEvent::categoryEntered(kind, name));
}

void ParserTestFixture::slotParserCategoryLeft()
{
    parserEvents.push(MockParserEvent::categoryLeft());
}

void ParserTestFixture::slotParserSetAttr(const Deska::Db::Identifier &kind, const Deska::Db::Identifier &name, const Deska::Db::Value &val)
{
    parserEvents.push(MockParserEvent::setAttr(kind, name, val));
}

void ParserTestFixture::slotParserSetAttrInsert(const Deska::Db::Identifier &kind, const Deska::Db::Identifier &name, const Deska::Db::Identifier &val)
{
    parserEvents.push(MockParserEvent::setAttrInsert(kind, name, val));
}

void ParserTestFixture::slotParserSetAttrRemove(const Deska::Db::Identifier &kind, const Deska::Db::Identifier &name, const Deska::Db::Identifier &val)
{
    parserEvents.push(MockParserEvent::setAttrRemove(kind, name, val));
}

void ParserTestFixture::slotParserRemoveAttr(const Deska::Db::Identifier &kind, const Deska::Db::Identifier &name)
{
    parserEvents.push(MockParserEvent::removeAttr(kind, name));
}

void ParserTestFixture::slotParserFunctionShow()
{
    parserEvents.push(MockParserEvent::functionShow());
}

void ParserTestFixture::slotParserFunctionDelete()
{
    parserEvents.push(MockParserEvent::functionDelete());
}

void ParserTestFixture::slotParserFunctionRename(const Deska::Db::Identifier &newName)
{
    parserEvents.push(MockParserEvent::functionRename(newName));
}

void ParserTestFixture::slotParserObjectsFilter(const Deska::Db::Identifier &kind, const boost::optional<Deska::Db::Filter> &filter)
{
    parserEvents.push(MockParserEvent::objectsFilter(kind, filter));
}

void ParserTestFixture::slotParserParseError(const Deska::Cli::ParserException &exception)
{
    parserEvents.push(MockParserEvent::parserError(exception));
}

void ParserTestFixture::slotParserParsingFinished()
{
    parserEvents.push(MockParserEvent::parsingFinished());
}

void ParserTestFixture::slotParserParsingStarted()
{
    parserEvents.push(MockParserEvent::parsingStarted());
}

void ParserTestFixture::expectNothingElse()
{
    BOOST_CHECK_MESSAGE(parserEvents.empty(), "Expected no more emitted signals");
    if (!parserEvents.empty()) {
        // show the first queued event here
        BOOST_CHECK_EQUAL(MockParserEvent::invalid(), parserEvents.front());
    }
}

void ParserTestFixture::expectCreateObject(const Deska::Db::Identifier &kind, const Deska::Db::Identifier &name)
{
    expectHelper(MockParserEvent::createObject(kind, name));
}

void ParserTestFixture::expectCategoryEntered(const Deska::Db::Identifier &kind, const Deska::Db::Identifier &name)
{
    expectHelper(MockParserEvent::categoryEntered(kind, name));
}

void ParserTestFixture::expectCategoryLeft()
{
    expectHelper(MockParserEvent::categoryLeft());
}

void ParserTestFixture::expectSetAttr(const Deska::Db::Identifier &kind, const Deska::Db::Identifier &name, const Deska::Db::Value &val)
{
    expectHelper(MockParserEvent::setAttr(kind, name, val));
}

void ParserTestFixture::expectSetAttrInsert(const Deska::Db::Identifier &kind, const Deska::Db::Identifier &name, const Deska::Db::Identifier &val)
{
    expectHelper(MockParserEvent::setAttrInsert(kind, name, val));
}

void ParserTestFixture::expectSetAttrRemove(const Deska::Db::Identifier &kind, const Deska::Db::Identifier &name, const Deska::Db::Identifier &val)
{
    expectHelper(MockParserEvent::setAttrRemove(kind, name, val));
}

void ParserTestFixture::expectRemoveAttr(const Deska::Db::Identifier &kind, const Deska::Db::Identifier &name)
{
    expectHelper(MockParserEvent::removeAttr(kind, name));
}

void ParserTestFixture::expectFunctionShow()
{
    expectHelper(MockParserEvent::functionShow());
}

void ParserTestFixture::expectFunctionDelete()
{
    expectHelper(MockParserEvent::functionDelete());
}

void ParserTestFixture::expectFunctionRename(const Deska::Db::Identifier &newName)
{
    expectHelper(MockParserEvent::functionRename(newName));
}

void ParserTestFixture::expectObjectsFilter(const Deska::Db::Identifier &kind, const boost::optional<Deska::Db::Filter> &filter)
{
    expectHelper(MockParserEvent::objectsFilter(kind, filter));
}

void ParserTestFixture::expectParseError(const Deska::Cli::ParserException &exception)
{
    expectHelper(MockParserEvent::parserError(exception));
}

void ParserTestFixture::expectParsingFinished()
{
    expectHelper(MockParserEvent::parsingFinished());
}

void ParserTestFixture::expectParsingStarted()
{
    expectHelper(MockParserEvent::parsingStarted());
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

void ParserTestFixture::verifyStackOneLevel(const Deska::Db::Identifier &kind, const boost::optional<Deska::Db::Filter> &filter)
{
    const std::vector<Deska::Cli::ContextStackItem> &stack = parser->currentContextStack();
    std::vector<Deska::Cli::ContextStackItem> specimen;
    specimen.push_back(Deska::Cli::ContextStackItem(kind, filter));
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

void ParserTestFixture::verifyStackTwoLevels(const Deska::Db::Identifier &kind1, const boost::optional<Deska::Db::Filter> &filter1,
                             const Deska::Db::Identifier &kind2, const Deska::Db::Identifier &name2)
{
    const std::vector<Deska::Cli::ContextStackItem> &stack = parser->currentContextStack();
    std::vector<Deska::Cli::ContextStackItem> specimen;
    specimen.push_back(Deska::Cli::ContextStackItem(kind1, filter1));
    specimen.push_back(Deska::Cli::ContextStackItem(kind2, name2));
    BOOST_CHECK(parser->isNestedInContext());
    BOOST_CHECK_EQUAL_COLLECTIONS(stack.begin(), stack.end(), specimen.begin(), specimen.end());
}

void ParserTestFixture::verifyStackTwoLevels(const Deska::Db::Identifier &kind1, const Deska::Db::Identifier &name1,
                             const Deska::Db::Identifier &kind2, const boost::optional<Deska::Db::Filter> &filter2)
{
    const std::vector<Deska::Cli::ContextStackItem> &stack = parser->currentContextStack();
    std::vector<Deska::Cli::ContextStackItem> specimen;
    specimen.push_back(Deska::Cli::ContextStackItem(kind1, name1));
    specimen.push_back(Deska::Cli::ContextStackItem(kind2, filter2));
    BOOST_CHECK(parser->isNestedInContext());
    BOOST_CHECK_EQUAL_COLLECTIONS(stack.begin(), stack.end(), specimen.begin(), specimen.end());
}

void ParserTestFixture::verifyStackTwoLevels(const Deska::Db::Identifier &kind1, const boost::optional<Deska::Db::Filter> &filter1,
                             const Deska::Db::Identifier &kind2, const boost::optional<Deska::Db::Filter> &filter2)
{
    const std::vector<Deska::Cli::ContextStackItem> &stack = parser->currentContextStack();
    std::vector<Deska::Cli::ContextStackItem> specimen;
    specimen.push_back(Deska::Cli::ContextStackItem(kind1, filter1));
    specimen.push_back(Deska::Cli::ContextStackItem(kind2, filter2));
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

void ParserTestFixture::slotParserSetAttrInsertCheckContext()
{
    BOOST_CHECK_MESSAGE(parser->isNestedInContext(), "Parser has to be nested in a context in order to insert identifier in a set");
}

void ParserTestFixture::slotParserSetAttrRemoveCheckContext()
{
    BOOST_CHECK_MESSAGE(parser->isNestedInContext(), "Parser has to be nested in a context in order to remove identifier form a set");
}

void ParserTestFixture::slotParserRemoveAttrCheckContext()
{
    BOOST_CHECK_MESSAGE(parser->isNestedInContext(), "Parser has to be nested in a context in order to remove attributes");
}

void ParserTestFixture::slotParserDeleteCheckContext()
{
    BOOST_CHECK_MESSAGE(parser->isNestedInContext(), "Parser has to be nested in a context in order to delete object");
}

void ParserTestFixture::slotParserRenameCheckContext()
{
    BOOST_CHECK_MESSAGE(parser->isNestedInContext(), "Parser has to be nested in a context in order to rename object");
}
