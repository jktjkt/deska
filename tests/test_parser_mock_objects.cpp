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

#define BOOST_TEST_MODULE parser_mock_objects
#include <boost/signals2/shared_connection_block.hpp>
#include <boost/test/unit_test.hpp>
#include "ParserTestFixture.h"


/** @short Verify that the mock object setup works properly

In this test case, we check our mock facility for obvious errors.
*/
BOOST_FIXTURE_TEST_CASE( test_mock_objects, ParserTestFixture )
{
    // There's no parser, so we have to skip testing of the proper nesting
    boost::signals2::shared_connection_block(attrCheckContextConnection);

    // At first, nothing should be present in there
    expectNothingElse();

    // Simulate slots triggered by the Parser
    slotParserParsingStarted();
    slotParserCategoryEntered("a", "b");
    slotParserSetAttr("a", "foo", Deska::Db::Value(std::string("bar")));
    slotParserRemoveAttr("a", "rab");
    slotParserSetAttrInsert("a", "seta", "xx");
    slotParserSetAttrRemove("a", "setb", "yy");
    slotParserCategoryLeft();
    slotParserParsingFinished();

    // ...and verify that we indeed received them
    expectParsingStarted();
    expectCategoryEntered("a", "b");
    expectSetAttr("a", "foo", Deska::Db::Value(std::string("bar")));
    expectRemoveAttr("a", "rab");
    expectSetAttrInsert("a", "seta", "xx");
    expectSetAttrRemove("a", "setb", "yy");
    expectCategoryLeft();
    expectParsingFinished();

    // The shouldn't be anything else at this point
    expectNothingElse();
    verifyEmptyStack();
    
    // Simulate function slots triggered by the Parser
    slotParserParsingStarted();
    slotParserCategoryEntered("a", "b");
    slotParserFunctionShow();
    slotParserFunctionDelete();
    slotParserFunctionRename("c");
    slotParserCategoryLeft();
    slotParserParsingFinished();
    
    // ...and verify that we indeed received them
    expectParsingStarted();
    expectCategoryEntered("a", "b");
    expectFunctionShow();
    expectFunctionDelete();
    expectFunctionRename("c");
    expectCategoryLeft();
    expectParsingFinished();

    // The shouldn't be anything else at this point
    expectNothingElse();
    verifyEmptyStack();

    // Simulate function slots triggered by the Parser
    slotParserParsingStarted();
    slotParserCreateObject("a", "b");
    slotParserParsingFinished();

    // ...and verify that we indeed received them
    expectParsingStarted();
    expectCreateObject("a", "b");
    expectParsingFinished();

    // The shouldn't be anything else at this point
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Test the implementation of equality test on caught exceptions */
BOOST_FIXTURE_TEST_CASE(test_mock_exceptions, ParserTestFixture) {
    // Verify that basic exception handling works well
    slotParserParsingStarted();
    slotParserParseError(Deska::Cli::InvalidAttributeDataTypeError("foo bar"));
    
    expectParsingStarted();
    expectParseError(Deska::Cli::InvalidAttributeDataTypeError("foo bar"));
    expectNothingElse();


    // Test exception parameters
    std::string s1 = "this is a sample input";
    std::string::const_iterator it1 = s1.begin() + s1.size() / 2;
    slotParserParsingStarted();
    slotParserParseError(Deska::Cli::UndefinedAttributeError("some message", s1, it1));
    std::string s2 = "this is a sample input";
    std::string::const_iterator it2 = s2.begin() + s2.size() / 2;

    expectParsingStarted();
    expectParseError(Deska::Cli::UndefinedAttributeError("some message", s2, it2));
    expectNothingElse();
}

/** @short Helper for testing inequality of logged exceptions */
void verifyExceptionDiffers(const Deska::Cli::ParserException &e, ParserTestFixture &f)
{
    BOOST_REQUIRE( ! f.parserEvents.empty() );
    BOOST_CHECK_NE(f.parserEvents.front(), MockParserEvent::parserError(e));
    f.parserEvents.pop();
}

/** @short Test that we see a difference between two exceptions which are not equal */
BOOST_FIXTURE_TEST_CASE(test_mock_exceptions_not_equal, ParserTestFixture)
{
    // Test for difference in the description
    slotParserParseError(Deska::Cli::InvalidAttributeDataTypeError("foo bor"));
    verifyExceptionDiffers(Deska::Cli::InvalidAttributeDataTypeError("foo bar"), *this);
    expectNothingElse();

    // Compare different classes
    slotParserParseError(Deska::Cli::NestingError("foo bar"));
    verifyExceptionDiffers(Deska::Cli::InvalidAttributeDataTypeError("foo bar"), *this);
    expectNothingElse();

    // Different user-supplied text
    std::string s1 = "this is a sample input";
    std::string::const_iterator it1 = s1.begin() + s1.size() / 2;
    slotParserParseError(Deska::Cli::UndefinedAttributeError("some message", s1, it1));
    std::string s2 = "this is A sample input";
    std::string::const_iterator it2 = s2.begin() + s2.size() / 2;
    verifyExceptionDiffers(Deska::Cli::UndefinedAttributeError("some message", s2, it2), *this);
    expectNothingElse();

    // Same text, different position of the iterator
    s2 = s1;
    it2 = it1 + 1;
    slotParserParseError(Deska::Cli::UndefinedAttributeError("some message", s1, it1));
    verifyExceptionDiffers(Deska::Cli::UndefinedAttributeError("some message", s2, it2), *this);
    expectNothingElse();
}
