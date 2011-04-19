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
BOOST_FIXTURE_TEST_CASE( parsing_top_level_object_on_two_lines, ParserTestFixture )
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
BOOST_FIXTURE_TEST_CASE( parsing_trivial_argument, ParserTestFixture )
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
BOOST_FIXTURE_TEST_CASE( parsing_trivial_argument_inline, ParserTestFixture )
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
BOOST_FIXTURE_TEST_CASE( parsing_two_arguments, ParserTestFixture )
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
BOOST_FIXTURE_TEST_CASE( parsing_two_arguments_inline, ParserTestFixture )
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
BOOST_FIXTURE_TEST_CASE( parsing_two_toplevel, ParserTestFixture )
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
BOOST_FIXTURE_TEST_CASE(parsing_multiple_arguments_inline, ParserTestFixture)
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
BOOST_FIXTURE_TEST_CASE(error_in_datatype_of_first_inline, ParserTestFixture)
{
    const std::string line = "hardware abcde id xx name \"jmeno\" price 1234.5\n";
    const std::string::const_iterator it = line.begin() + line.find("xx");
    parser->parseLine(line);
    expectCategoryEntered("hardware", "abcde");
    expectParseError(Deska::Cli::InvalidAttributeDataTypeError("Error while parsing argument value for id. Expected one of [ <integer> ].", line, it));
    expectCategoryLeft();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Syntax error in the name of the first attribute

Similar to error_in_datatype_of_first_inline, but the mistake is not in the value, but rather in the attribute identifier.

@see error_in_datatype_of_first_inline

*/
BOOST_FIXTURE_TEST_CASE(error_in_first_attr_name_inline, ParserTestFixture)
{
    const std::string line = "hardware abcde isd 123 name \"jmeno\" price 1234.5\n";
    const std::string::const_iterator it = line.begin() + line.find("isd");
    parser->parseLine(line);
    expectCategoryEntered("hardware", "abcde");
    expectParseError(Deska::Cli::UndefinedAttributeError("Error while parsing attribute name for hardware. Expected one of [ \"id\" \"name\" \"price\" ].", line, it));
    expectCategoryLeft();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Syntax error in the kind of a top-level object */
BOOST_FIXTURE_TEST_CASE(error_toplevel_name, ParserTestFixture)
{
    const std::string line = "haware abcde id 123 name \"jmeno\" price 1234.5\n";
    const std::string::const_iterator it = line.begin();
    parser->parseLine(line);
    expectParseError(Deska::Cli::InvalidObjectKind("Error while parsing kind name. Unknown top-level kind. Expected one of [ \"hardware\" \"host\" \"interface\" ].", line, it));
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Test parsing of an object nested into the parent one */
BOOST_FIXTURE_TEST_CASE(nested_interface, ParserTestFixture)
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

/** @short An attribute for parent is listed inline after an embedded object -> fail */
BOOST_FIXTURE_TEST_CASE(nested_interface_inline_with_attr_for_parent, ParserTestFixture)
{
    const std::string line ="host abcde hardware_id 123 name \"jmeno\" interface eth0 mac \"nejakamac\" price 1234.5";
    const std::string::const_iterator it = line.begin() + line.find("price");
    parser->parseLine(line);
    expectCategoryEntered("host", "abcde");
    // hardware_id is an identifier, not an int
    expectSetAttr("hardware_id", "123");
    expectSetAttr("name", "jmeno");
    expectCategoryEntered("interface", "eth0");
    expectSetAttr("mac", "nejakamac");
    expectParseError(Deska::Cli::UndefinedAttributeError("Error while parsing attribute name for interface. Expected one of [ \"ip\" \"mac\" ].", line, it));
    expectCategoryLeft();
    expectCategoryLeft();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Inline definition of an embedded object given immediately after the parent */
BOOST_FIXTURE_TEST_CASE(nested_interface_immediately_inline, ParserTestFixture)
{
    parser->parseLine("host abcde interface eth0 mac \"nejakamac\"\n");
    expectCategoryEntered("host", "abcde");
    expectCategoryEntered("interface", "eth0");
    expectSetAttr("mac", "nejakamac");
    expectCategoryLeft();
    expectCategoryLeft();
    expectNothingElse();
}


/** @short Inline definition of an embedded object after a paren't attr */
BOOST_FIXTURE_TEST_CASE(nested_interface_after_parent_attr_inline, ParserTestFixture)
{
    parser->parseLine("host abcde hardware_id 1 interface eth0 mac \"nejakamac\"\n");
    expectCategoryEntered("host", "abcde");
    expectSetAttr("hardware_id", "1"); // identifier, not an int
    expectCategoryEntered("interface", "eth0");
    expectSetAttr("mac", "nejakamac");
    expectCategoryLeft();
    expectCategoryLeft();
    expectNothingElse();
}

/** @short Embedding incompatible types after a paren't attribute */
BOOST_FIXTURE_TEST_CASE(embed_incompatible_types_with_attr_inline, ParserTestFixture)
{
    const std::string line = "hardware abcde id 123 interface eth0";
    const std::string::const_iterator it = line.begin() + line.find("interface");
    parser->parseLine(line);
    expectCategoryEntered("hardware", "abcde");
    expectSetAttr("id", 123);
    expectParseError(Deska::Cli::UndefinedAttributeError("Error while parsing attribute name for hardware. Expected one of [ \"id\" \"name\" \"price\" ].", line, it));
    expectCategoryLeft();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Embedding incompatible types immediately after paren't definition */
BOOST_FIXTURE_TEST_CASE(embed_incompatible_immediately_inline, ParserTestFixture)
{
    const std::string line = "hardware abcde interface eth0";
    const std::string::const_iterator it = line.begin() + line.find("interface");
    parser->parseLine(line);
    expectCategoryEntered("hardware", "abcde");
    expectParseError(Deska::Cli::UndefinedAttributeError("Error while parsing attribute name for hardware. Expected one of [ \"id\" \"name\" \"price\" ].", line, it));
    expectCategoryLeft();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short An embedded object in an inline form should not cause full rollback to empty state, but stay in the previous context */
BOOST_FIXTURE_TEST_CASE(multiline_with_error_in_inline_embed, ParserTestFixture)
{
    parser->parseLine("host abcde\r\n");
    expectCategoryEntered("host", "abcde");
    expectNothingElse();
    verifyStackOneLevel("host", "abcde");
    parser->parseLine("name \"jmeno\"\r\n");
    expectSetAttr("name", "jmeno");
    expectNothingElse();
    verifyStackOneLevel("host", "abcde");
    const std::string line = "interface eth0 mac \"foo\" bar baz\r\n";
    const std::string::const_iterator it = line.begin() + line.find("bar");
    parser->parseLine(line);
    expectCategoryEntered("interface", "eth0");
    expectSetAttr("mac", "foo");
    expectParseError(Deska::Cli::UndefinedAttributeError("Error while parsing attribute name for interface. Expected one of [ \"ip\" \"mac\" ].", line, it));
    expectCategoryLeft();
    expectNothingElse();
    verifyStackOneLevel("host", "abcde");
}

/** @short An embedded object in an inline form should then return to the previous context */
BOOST_FIXTURE_TEST_CASE(multiline_with_inline_embed, ParserTestFixture)
{
    parser->parseLine("host abcde\r\n");
    expectCategoryEntered("host", "abcde");
    expectNothingElse();
    verifyStackOneLevel("host", "abcde");
    parser->parseLine("name \"jmeno\"\r\n");
    expectSetAttr("name", "jmeno");
    expectNothingElse();
    verifyStackOneLevel("host", "abcde");
    parser->parseLine("interface eth0 mac \"foo\"\r\n");
    expectCategoryEntered("interface", "eth0");
    expectSetAttr("mac", "foo");
    expectCategoryLeft();
    expectNothingElse();
    verifyStackOneLevel("host", "abcde");
}

/** @short Generic test for multiline embed */
BOOST_FIXTURE_TEST_CASE(multiline_with_embed, ParserTestFixture)
{
    parser->parseLine("host abcde\r\n");
    expectCategoryEntered("host", "abcde");
    expectNothingElse();
    verifyStackOneLevel("host", "abcde");

    parser->parseLine("name \"jmeno\"\r\n");
    expectSetAttr("name", "jmeno");
    expectNothingElse();
    verifyStackOneLevel("host", "abcde");

    parser->parseLine("interface eth0\r\n");
    expectCategoryEntered("interface", "eth0");
    expectNothingElse();
    verifyStackTwoLevels("host", "abcde", "interface", "eth0");

    parser->parseLine("mac \"foo\"\r\n");
    expectSetAttr("mac", "foo");
    expectNothingElse();
    verifyStackTwoLevels("host", "abcde", "interface", "eth0");

    parser->parseLine("end\r\n");
    expectCategoryLeft();
    expectNothingElse();
    verifyStackOneLevel("host", "abcde");

    parser->parseLine("end\r\n");
    expectCategoryLeft();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short An error in multiline embed should not manipulate the context at all */
BOOST_FIXTURE_TEST_CASE(multiline_with_error_in_multiline_embed, ParserTestFixture)
{
    parser->parseLine("host abcde\r\n");
    expectCategoryEntered("host", "abcde");
    expectNothingElse();
    verifyStackOneLevel("host", "abcde");

    parser->parseLine("name \"jmeno\"\r\n");
    expectSetAttr("name", "jmeno");
    expectNothingElse();
    verifyStackOneLevel("host", "abcde");

    parser->parseLine("interface eth0\r\n");
    expectCategoryEntered("interface", "eth0");
    expectNothingElse();
    verifyStackTwoLevels("host", "abcde", "interface", "eth0");

    const std::string line = "maaaac \"foo\"\r\n";
    const std::string::const_iterator it = line.begin();
    parser->parseLine(line);
    expectParseError(Deska::Cli::UndefinedAttributeError("Error while parsing attribute name for interface. Expected one of [ \"ip\" \"mac\" ].", line, it));
    expectNothingElse();
    verifyStackTwoLevels("host", "abcde", "interface", "eth0");

    parser->parseLine("end\r\n");
    expectCategoryLeft();
    expectNothingElse();
    verifyStackOneLevel("host", "abcde");

    parser->parseLine("end\r\n");
    expectCategoryLeft();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short An error in kind name of embedded object */
BOOST_FIXTURE_TEST_CASE(invalid_kind_name_of_embed_object, ParserTestFixture)
{
    const std::string line = "host 123 int3rf4ce 456\n";
    const std::string::const_iterator it = line.begin() + line.find("int3rf4ce");
    parser->parseLine(line);
    expectCategoryEntered("host", "123");
    expectParseError(Deska::Cli::UndefinedAttributeError("Error while parsing attribute name or nested kind name for host. Expected one of [ \"hardware_id\" \"name\" \"interface\" ].", line, it));
    expectCategoryLeft();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short An error in kind identifier with single definition on line */
BOOST_FIXTURE_TEST_CASE(error_invalid_kind_name_single_definition, ParserTestFixture)
{
    const std::string line = "haware foo\n";
    const std::string::const_iterator it = line.begin();
    parser->parseLine(line);
    expectParseError(Deska::Cli::InvalidObjectKind("Error while parsing kind name. Unknown top-level kind. Expected one of [ \"hardware\" \"host\" \"interface\" ].", line, it));
    expectNothingElse();
    verifyEmptyStack();
}

/** @short end used when not nested */
BOOST_FIXTURE_TEST_CASE(error_end_no_context, ParserTestFixture)
{
    const std::string line = "end\n";
    const std::string::const_iterator it = line.begin();
    parser->parseLine(line);
    expectParseError(Deska::Cli::InvalidObjectKind("Error while parsing kind name. Unknown top-level kind. Expected one of [ \"hardware\" \"host\" \"interface\" ].", line, it));
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Bad identifier of top-level object */
BOOST_FIXTURE_TEST_CASE(error_invalid_object_identifier_toplevel, ParserTestFixture)
{
    const std::string line = "hardware foo*bar\n";
    const std::string::const_iterator it = line.begin() + line.find("*bar");
    parser->parseLine(line);
    expectCategoryEntered("hardware", "foo");
    expectParseError(Deska::Cli::UndefinedAttributeError("Error while parsing attribute name for hardware. Expected one of [ \"id\" \"name\" \"price\" ].", line, it));
    expectCategoryLeft();
    //expectParseError(Deska::Cli::InvalidAttributeDataTypeError("Error while parsing argument value for hardware. Expected one of [ <identifier (alphanumerical letters and _)> ].", line, it));
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Bad identifier of top-level object */
BOOST_FIXTURE_TEST_CASE(error_invalid_object_identifier_begin_toplevel, ParserTestFixture)
{
    const std::string line = "hardware *bar\n";
    const std::string::const_iterator it = line.begin() + line.find("*bar");
    parser->parseLine(line);
    expectParseError(Deska::Cli::InvalidAttributeDataTypeError("Error while parsing argument value for hardware. Expected one of [ <object identifier (alphanumerical letters and _)> ].", line, it));
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Bad identifier of nested object */
BOOST_FIXTURE_TEST_CASE(error_invalid_object_identifier_nested, ParserTestFixture)
{
    const std::string line = "host hpv2 interface foo*bar\n";
    const std::string::const_iterator it = line.begin() + line.find("*bar");
    parser->parseLine(line);
    expectCategoryEntered("host", "hpv2");
    expectCategoryEntered("interface", "foo");
    expectParseError(Deska::Cli::UndefinedAttributeError("Error while parsing attribute name for interface. Expected one of [ \"ip\" \"mac\" ].", line, it));
    expectCategoryLeft();
    //expectParseError(Deska::Cli::InvalidAttributeDataTypeError("Error while parsing argument value for hardware. Expected one of [ <identifier (alphanumerical letters and _)> ].", line, it));
    expectCategoryLeft();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Verify that we can enter into an embedded context with just a single line */
BOOST_FIXTURE_TEST_CASE(nested_kinds_inline_nothing_else, ParserTestFixture)
{
    parser->parseLine("host 123 interface 456\n");
    expectCategoryEntered("host", "123");
    expectCategoryEntered("interface", "456");
    expectNothingElse();
    verifyStackTwoLevels("host", "123", "interface", "456");

    parser->parseLine("end\n");
    expectCategoryLeft();
    expectNothingElse();
    verifyStackOneLevel("host", "123");

    parser->parseLine("end\n");
    expectCategoryLeft();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Verify that we can enter into an embedded context with just a single line and set attributes */
BOOST_FIXTURE_TEST_CASE(nested_kinds_inline_attr, ParserTestFixture)
{
    parser->parseLine("host 123 interface 456 ip \"x\"\n");
    expectCategoryEntered("host", "123");
    expectCategoryEntered("interface", "456");
    expectSetAttr("ip", "x");
    expectCategoryLeft();
    expectCategoryLeft();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Verify that we can use function show in no context */
BOOST_FIXTURE_TEST_CASE(function_show_no_context, ParserTestFixture)
{
    parser->parseLine("show\n");
    expectFunctionShow();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Verify that we can use function show in context */
BOOST_FIXTURE_TEST_CASE(function_show_in_context, ParserTestFixture)
{
    parser->parseLine("host 123\n");
    expectCategoryEntered("host", "123");
    parser->parseLine("show\n");
    expectFunctionShow();
    expectNothingElse();
    verifyStackOneLevel("host", "123");
}

/** @short Verify that we can use function show with parameter in no context */
BOOST_FIXTURE_TEST_CASE(function_show_param_no_context, ParserTestFixture)
{
    parser->parseLine("show host 123\n");
    expectCategoryEntered("host", "123");
    expectFunctionShow();
    expectCategoryLeft();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Verify that we can use function show with nesting in no context */
BOOST_FIXTURE_TEST_CASE(function_show_nest_no_context, ParserTestFixture)
{
    parser->parseLine("show host 123 interface eth0\n");
    expectCategoryEntered("host", "123");
    expectCategoryEntered("interface", "eth0");
    expectFunctionShow();
    expectCategoryLeft();
    expectCategoryLeft();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Verify that we can use function show with parameter in context */
BOOST_FIXTURE_TEST_CASE(function_show_param_in_context, ParserTestFixture)
{
    parser->parseLine("host 123\n");
    expectCategoryEntered("host", "123");
    parser->parseLine("show interface 456\n");
    expectCategoryEntered("interface", "456");
    expectFunctionShow();
    expectCategoryLeft();
    expectNothingElse();
    verifyStackOneLevel("host", "123");
}

/** @short Verify that we can not use function delete without parameters in no context */
BOOST_FIXTURE_TEST_CASE(function_delete_no_context, ParserTestFixture)
{
    const std::string line = "delete\n";
    const std::string::const_iterator it = line.end();
    parser->parseLine(line);
    expectParseError(Deska::Cli::InvalidObjectKind("Function delete requires kind as parameter.", line, it));
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Verify that we can not use function delete without parameters in context */
BOOST_FIXTURE_TEST_CASE(function_delete_in_context, ParserTestFixture)
{
    
    parser->parseLine("host 123\n");
    expectCategoryEntered("host", "123");
    const std::string line = "delete\n";
    const std::string::const_iterator it = line.end();
    parser->parseLine(line);
    expectParseError(Deska::Cli::InvalidObjectKind("Function delete requires kind as parameter.", line, it));
    expectNothingElse();
    verifyStackOneLevel("host", "123");
}

/** @short Verify that we can use function delete with parameter in no context */
BOOST_FIXTURE_TEST_CASE(function_delete_param_no_context, ParserTestFixture)
{
    parser->parseLine("delete host 123\n");
    expectCategoryEntered("host", "123");
    expectFunctionDelete();
    expectCategoryLeft();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Verify that we can use function delete with nesting in no context */
BOOST_FIXTURE_TEST_CASE(function_delete_nest_no_context, ParserTestFixture)
{
    parser->parseLine("delete host 123 interface eth0\n");
    expectCategoryEntered("host", "123");
    expectCategoryEntered("interface", "eth0");
    expectFunctionDelete();
    expectCategoryLeft();
    expectCategoryLeft();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Verify that we can use function delete with parameter in context */
BOOST_FIXTURE_TEST_CASE(function_delete_param_in_context, ParserTestFixture)
{
    parser->parseLine("host 123\n");
    expectCategoryEntered("host", "123");
    parser->parseLine("delete interface 456\n");
    expectCategoryEntered("interface", "456");
    expectFunctionDelete();
    expectCategoryLeft();
    expectNothingElse();
    verifyStackOneLevel("host", "123");
}

/** @short Bad kind name in function delete with parameter in no context */
BOOST_FIXTURE_TEST_CASE(error_invalid_kind_name_function_delete_param_no_context, ParserTestFixture)
{
    const std::string line = "delete hot 123\n";
    const std::string::const_iterator it = line.begin() + line.find("hot");
    parser->parseLine(line);
    expectParseError(Deska::Cli::InvalidObjectKind("Error while parsing kind name. Unknown top-level kind. Expected one of [ \"hardware\" \"host\" \"interface\" ].", line, it));
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Bad kind name in function delete with nesting in no context */
BOOST_FIXTURE_TEST_CASE(error_invalid_kind_name_function_delete_nest_no_context, ParserTestFixture)
{
    const std::string line = "delete host 123 inteface eth0\n";
    const std::string::const_iterator it = line.begin() + line.find("inteface");
    parser->parseLine(line);
    expectCategoryEntered("host", "123");
    expectParseError(Deska::Cli::InvalidObjectKind("Error while parsing kind name of nested object in host. Expected one of [ \"interface\" ].", line, it));
    expectCategoryLeft();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Bad kind name in function delete with parameter in context */
BOOST_FIXTURE_TEST_CASE(error_invalid_kind_name_function_delete_param_in_context, ParserTestFixture)
{
    parser->parseLine("host 123\n");
    expectCategoryEntered("host", "123");
    const std::string line = "delete inteface eth0\n";
    const std::string::const_iterator it = line.begin() + line.find("inteface");
    parser->parseLine(line);
    expectParseError(Deska::Cli::InvalidObjectKind("Error while parsing kind name of nested object in host. Expected one of [ \"interface\" ].", line, it));
    expectNothingElse();
    verifyStackOneLevel("host", "123");
}

/** @short Bad kind name in function show with parameter in no context */
BOOST_FIXTURE_TEST_CASE(error_invalid_kind_name_function_show_param_no_context, ParserTestFixture)
{
    const std::string line = "show hot 123\n";
    const std::string::const_iterator it = line.begin() + line.find("hot");
    parser->parseLine(line);
    expectParseError(Deska::Cli::InvalidObjectKind("Error while parsing kind name. Unknown top-level kind. Expected one of [ \"hardware\" \"host\" \"interface\" ].", line, it));
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Bad kind name in function show with nesting in no context */
BOOST_FIXTURE_TEST_CASE(error_invalid_kind_name_function_show_nest_no_context, ParserTestFixture)
{
    const std::string line = "show host 123 inteface eth0\n";
    const std::string::const_iterator it = line.begin() + line.find("inteface");
    parser->parseLine(line);
    expectCategoryEntered("host", "123");
    expectParseError(Deska::Cli::InvalidObjectKind("Error while parsing kind name of nested object in host. Expected one of [ \"interface\" ].", line, it));
    expectCategoryLeft();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Bad kind name in function show with parameter in context */
BOOST_FIXTURE_TEST_CASE(error_invalid_kind_name_function_show_param_in_context, ParserTestFixture)
{
    parser->parseLine("host 123\n");
    expectCategoryEntered("host", "123");
    const std::string line = "show inteface eth0\n";
    const std::string::const_iterator it = line.begin() + line.find("inteface");
    parser->parseLine(line);
    expectParseError(Deska::Cli::InvalidObjectKind("Error while parsing kind name of nested object in host. Expected one of [ \"interface\" ].", line, it));
    expectNothingElse();
    verifyStackOneLevel("host", "123");
}

/** @short Function show with parameter in context where are no nested kinds */
BOOST_FIXTURE_TEST_CASE(error_function_show_param_in_context_no_nested, ParserTestFixture)
{
    parser->parseLine("hardware 123\n");
    expectCategoryEntered("hardware", "123");
    const std::string line = "show interface eth0\n";
    const std::string::const_iterator it = line.begin() + line.find("interface");
    parser->parseLine(line);
    expectParseError(Deska::Cli::ObjectNotFound("Error while parsing kind name of nested object in hardware.", line, it));
    expectNothingElse();
    verifyStackOneLevel("hardware", "123");
}

/** @short Function delete with parameter in context where are no nested kinds */
BOOST_FIXTURE_TEST_CASE(error_function_delete_param_in_context_no_nested, ParserTestFixture)
{
    parser->parseLine("hardware 123\n");
    expectCategoryEntered("hardware", "123");
    const std::string line = "show interface eth0\n";
    const std::string::const_iterator it = line.begin() + line.find("interface");
    parser->parseLine(line);
    expectParseError(Deska::Cli::ObjectNotFound("Error while parsing kind name of nested object in hardware.", line, it));
    expectNothingElse();
    verifyStackOneLevel("hardware", "123");
}
