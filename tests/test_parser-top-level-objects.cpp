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

#define BOOST_TEST_MODULE example
#include <boost/test/unit_test.hpp>
#include "deska/cli/Parser.h"
#include "ParserTestFixture.h"


/** @short Verify that we don't fail when leaving a context immediately we've entered it */
BOOST_FIXTURE_TEST_CASE( parsing_top_level_object_on_two_lines, F )
{
    // Start a new context with nothing inside
    parser->parseLine("hardware hpv2\r\n");
    expectCategoryEntered("hardware", "hpv2");
    expectNothingElse();
    verifyStackOneLevel("hardware", "hpv2");

    // ...and leave it immediately
    parser->parseLine("end\r\n");
    expectCategoryLeft();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Assign a simple value to an object using verbose syntax */
BOOST_FIXTURE_TEST_CASE( parsing_trivial_argument, F )
{
    // Start a new context
    parser->parseLine("hardware hpv2\r\n");
    expectCategoryEntered("hardware", "hpv2");
    expectNothingElse();

    // Verify stack nesting
    verifyStackOneLevel("hardware", "hpv2");

    // Set the attribute
    parser->parseLine("name \"foo bar baz\"\r\n");
    expectSetAttr("name", "foo bar baz");
    expectNothingElse();
    verifyStackOneLevel("hardware", "hpv2");

    // And terminate the input
    parser->parseLine("end\r\n");
    expectCategoryLeft();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Assing a simple value to an object using the inline syntax*/
BOOST_FIXTURE_TEST_CASE( parsing_trivial_argument_inline, F )
{
    // Start a new context
    parser->parseLine("hardware hpv2 name \"foo bar baz\"\r\n");
    expectCategoryEntered("hardware", "hpv2");
    expectSetAttr("name", "foo bar baz");
    expectCategoryLeft();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Set two attributes of an object using the multiline variant of the syntax */
BOOST_FIXTURE_TEST_CASE( parsing_two_arguments, F )
{
    // Start a new context
    parser->parseLine("hardware hpv2\r\n");
    expectCategoryEntered("hardware", "hpv2");
    expectNothingElse();
    verifyStackOneLevel("hardware", "hpv2");

    // Set the second one
    parser->parseLine("price 666\r\n");
    expectSetAttr("price", 666.0);
    expectNothingElse();
    verifyStackOneLevel("hardware", "hpv2");

    // Set the first attribute
    parser->parseLine("name \"foo bar baz\"\r\n");
    expectSetAttr("name", "foo bar baz");
    expectNothingElse();
    verifyStackOneLevel("hardware", "hpv2");

    // And terminate the input
    parser->parseLine("end\r\n");
    expectCategoryLeft();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Set two attributes of an object inline */
BOOST_FIXTURE_TEST_CASE( parsing_two_arguments_inline, F )
{
    // Start a new context
    parser->parseLine("hardware hpv2 price 666 name \"foo bar baz\"\r\n");
    expectCategoryEntered("hardware", "hpv2");
    expectSetAttr("price", 666.0);
    expectSetAttr("name", "foo bar baz");
    expectCategoryLeft();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Make sure we can indeed handle multiple top-level objects */
BOOST_FIXTURE_TEST_CASE( parsing_two_toplevel, F )
{
    // create hpv2
    parser->parseLine("hardware hpv2\r\n");
    expectCategoryEntered("hardware", "hpv2");
    expectNothingElse();
    verifyStackOneLevel("hardware", "hpv2");

    // terminate hpv2
    parser->parseLine("end\r\n");
    expectCategoryLeft();
    expectNothingElse();
    verifyEmptyStack();

    // create second object
    parser->parseLine("host hpv2\r\n");
    expectCategoryEntered("host", "hpv2");
    expectNothingElse();
    verifyStackOneLevel("host", "hpv2");

    // terminate the host
    parser->parseLine("end\r\n");
    expectCategoryLeft();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short test correct parsing of multiple arguments, all passed inline */
BOOST_FIXTURE_TEST_CASE(parsing_multiple_arguments_inline, F)
{
    parser->parseLine("hardware abcde id 1243 name \"jmeno\" price 1234.5");
    expectCategoryEntered("hardware", "abcde");
    expectSetAttr("id", 1243);
    expectSetAttr("name", "jmeno");
    expectSetAttr("price", 1234.5);
    expectCategoryLeft();
    expectNothingElse();
    verifyEmptyStack();
}
