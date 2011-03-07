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
    parser->parseLine("hardware abcde id 1243 name \"jmeno\" price 1234.5\n");
    expectCategoryEntered("hardware", "abcde");
    expectSetAttr("id", 1243);
    expectSetAttr("name", "jmeno");
    expectSetAttr("price", 1234.5);
    expectCategoryLeft();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Syntax error in the data type of the first attribute

A single-line input which enters a new category and immediately after that encounters an exception.
The idea here is that the stack should not roll back after the exception.
*/
BOOST_FIXTURE_TEST_CASE(error_in_datatype_of_first_inline, F)
{
    const std::string line = "hardware abcde id xx name \"jmeno\" price 1234.5\n";
    const std::string::const_iterator it = line.begin() + line.find("xx");
    parser->parseLine(line);
    expectCategoryEntered("hardware", "abcde");
    expectParseError(Deska::CLI::InvalidAttributeDataTypeError("Expecting integer as a data type for the \"id\" argument.", line, it));
    expectNothingElse();
    verifyStackOneLevel("hardware", "abcde");
}

/** @short Syntax error in the name of the first attribute

Similar to error_in_datatype_of_first_inline, but the mistake is not in the value, but rather in the attribute identifier.

@see error_in_datatype_of_first_inline

*/
BOOST_FIXTURE_TEST_CASE(error_in_first_attr_name_inline, F)
{
    const std::string line = "hardware abcde isd 123 name \"jmeno\" price 1234.5\n";
    const std::string::const_iterator it = line.begin() + line.find("isd");
    parser->parseLine(line);
    expectCategoryEntered("hardware", "abcde");
    expectParseError(Deska::CLI::UndefinedAttributeError("Attribute \"isd\" is not recognized for an object of type \"hardware\".", line, it));
    expectNothingElse();
    verifyStackOneLevel("hardware", "abcde");
}

/** @short Syntax error in the kind of a top-level object */
BOOST_FIXTURE_TEST_CASE(error_toplevel_name, F)
{
    const std::string line = "haware abcde id 123 name \"jmeno\" price 1234.5\n";
    const std::string::const_iterator it = line.begin();
    parser->parseLine(line);
    expectParseError(Deska::CLI::InvalidObjectKind("Object \"haware\" not recognized.", line, it));
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Test parsing of an object nested into the parent one */
BOOST_FIXTURE_TEST_CASE(nested_interface, F)
{
    parser->parseLine("host abcde\n");
    expectCategoryEntered("host", "abcde");
    expectNothingElse();

    verifyStackOneLevel("host", "abcde");
    parser->parseLine("name \"as123\"\n");
    expectSetAttr("name", "as123");
    expectNothingElse();
    verifyStackOneLevel("host", "abcde");

    parser->parseLine("interface eth0\n");
    expectCategoryEntered("interface", "eth0");
    expectNothingElse();
    verifyStackTwoLevels("host", "abcde", "interface", "eth0");

    parser->parseLine("mac \"nejakamac\"\n");
    expectSetAttr("mac", "nejakamac");
    expectNothingElse();
    verifyStackTwoLevels("host", "abcde", "interface", "eth0");
}

BOOST_AUTO_TEST_CASE_EXPECTED_FAILURES(nested_interface_inline_with_attr_for_parent, 6);
/** @short An attribute for parent is listed inline after an embedded object -> fail */
BOOST_FIXTURE_TEST_CASE(nested_interface_inline_with_attr_for_parent, F)
{
    const std::string line ="host abcde hardware_id 123 name \"jmeno\" interface eth0 mac \"nejakamac\" price 1234.5";
    const std::string::const_iterator it = line.begin() + line.find("price");
    parser->parseLine(line);
    expectCategoryEntered("host", "abcde");
    expectSetAttr("hardware_id", 123);
    expectSetAttr("name", "jmeno");
    expectCategoryEntered("interface", "eth0");
    expectSetAttr("mac", "nejakamac");
    expectParseError(Deska::CLI::UndefinedAttributeError("Attribute \"price\" not defined for object of type \"interface\".", line, it));
    expectNothingElse();
    verifyStackTwoLevels("host", "abcde", "interface", "eth0");
}

BOOST_AUTO_TEST_CASE_EXPECTED_FAILURES(nested_interface_immediately_inline, 2);
/** @short Inline definition of an embedded object given immediately after the parent */
BOOST_FIXTURE_TEST_CASE(nested_interface_immediately_inline, F)
{
    parser->parseLine("host abcde interface eth0 mac \"nejakamac\"\n");
    expectCategoryEntered("host", "abcde");
    expectCategoryEntered("interface", "eth0");
    expectSetAttr("mac", "nejakamac");
    expectCategoryLeft();
    expectCategoryLeft();
    expectNothingElse();
}


BOOST_AUTO_TEST_CASE_EXPECTED_FAILURES(nested_interface_after_parent_attr_inline, 9);
/** @short Inline definition of an embedded object after a paren't attr */
BOOST_FIXTURE_TEST_CASE(nested_interface_after_parent_attr_inline, F)
{
    parser->parseLine("host abcde hardware_id 1 interface eth0 mac \"nejakamac\"\n");
    expectCategoryEntered("host", "abcde");
    expectSetAttr("hardware_id", 1);
    expectCategoryEntered("interface", "eth0");
    expectSetAttr("mac", "nejakamac");
    expectCategoryLeft();
    expectCategoryLeft();
    expectNothingElse();
}

/** @short Embedding incompatible types after a paren't attribute */
BOOST_FIXTURE_TEST_CASE(embed_incompatible_types_with_attr_inline, F)
{
    const std::string line = "hardware abcde id 123 interface eth0";
    const std::string::const_iterator it = line.begin() + line.find("interface");
    parser->parseLine(line);
    expectCategoryEntered("hardware", "abcde");
    expectSetAttr("id", 123);
    expectParseError(Deska::CLI::NestingError("Can't embed object of type \"interface\" into \"hardware\".", line, it));
    expectNothingElse();
    verifyStackOneLevel("hardware", "abcde");
}

/** @short Embedding incompatible types immediately after paren't definition */
BOOST_FIXTURE_TEST_CASE(embed_incompatible_immediately_inline, F)
{
    const std::string line = "hardware abcde interface eth0";
    const std::string::const_iterator it = line.begin() + line.find("interface");
    parser->parseLine(line);
    expectCategoryEntered("hardware", "abcde");
    expectParseError(Deska::CLI::NestingError("Can't embed object of type \"interface\" into \"hardware\".", line, it));
    expectNothingElse();
    verifyStackOneLevel("hardware", "abcde");
}
