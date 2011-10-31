/*
* Copyright (C) 2011 Tomáš Hubík <hubik.tomas@gmail.com>
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

#define BOOST_TEST_MODULE cli_parser
#include <boost/test/unit_test.hpp>
#include "deska/cli/Parser.h"
#include "ParserTestFixture.h"


/** @short Verify that parser succeeds when parsing empty string */
BOOST_FIXTURE_TEST_CASE( parsing_empty_string, ParserTestFixture )
{
    parser->parseLine("");
    expectParsingStarted();
    expectParsingFinished();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Verify that parser succeeds when parsing empty line */
BOOST_FIXTURE_TEST_CASE( parsing_empty_line, ParserTestFixture )
{
    parser->parseLine("\n");
    expectParsingStarted();
    expectParsingFinished();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Verify that parser succeeds when parsing line of whitespaces */
BOOST_FIXTURE_TEST_CASE( parsing_whitespaces, ParserTestFixture )
{
    parser->parseLine("\t  \t \n");
    expectParsingStarted();
    expectParsingFinished();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Verify that we don't fail when leaving a context immediately we've entered it */
BOOST_FIXTURE_TEST_CASE( parsing_top_level_object_on_two_lines, ParserTestFixture )
{
    // Start a new context with nothing inside
    parser->parseLine("hardware hpv2\r\n");
    expectParsingStarted();
    expectCategoryEntered("hardware", "hpv2");
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("hardware", Deska::Db::Identifier("hpv2"));

    // ...and leave it immediately
    parser->parseLine("end\r\n");
    expectParsingStarted();
    expectCategoryLeft();
    expectParsingFinished();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Assign a simple value to an object using verbose syntax */
BOOST_FIXTURE_TEST_CASE( parsing_trivial_argument, ParserTestFixture )
{
    // Start a new context
    parser->parseLine("hardware hpv2\r\n");
    expectParsingStarted();
    expectCategoryEntered("hardware", "hpv2");
    expectParsingFinished();
    expectNothingElse();

    // Verify stack nesting
    verifyStackOneLevel("hardware", Deska::Db::Identifier("hpv2"));

    // Set the attribute
    parser->parseLine("hardware_name \"foo bar baz\"\r\n");
    expectParsingStarted();
    expectSetAttr("hardware", "hardware_name", Deska::Db::Value(std::string("foo bar baz")));
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("hardware", Deska::Db::Identifier("hpv2"));

    // And terminate the input
    parser->parseLine("end\r\n");
    expectParsingStarted();
    expectCategoryLeft();
    expectParsingFinished();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Assigning a simple value to an object using the inline syntax*/
BOOST_FIXTURE_TEST_CASE( parsing_trivial_argument_inline, ParserTestFixture )
{
    // Start a new context
    parser->parseLine("hardware hpv2 hardware_name \"foo bar baz\"\r\n");
    expectParsingStarted();
    expectCategoryEntered("hardware", "hpv2");
    expectSetAttr("hardware", "hardware_name", Deska::Db::Value(std::string("foo bar baz")));
    expectCategoryLeft();
    expectParsingFinished();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Set two attributes of an object using the multiline variant of the syntax */
BOOST_FIXTURE_TEST_CASE( parsing_two_arguments, ParserTestFixture )
{
    // Start a new context
    parser->parseLine("hardware hpv2\r\n");
    expectParsingStarted();
    expectCategoryEntered("hardware", "hpv2");
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("hardware", Deska::Db::Identifier("hpv2"));

    // Set the first attribute
    parser->parseLine("price 666\r\n");
    expectParsingStarted();
    expectSetAttr("hardware", "price", Deska::Db::Value(666.0));
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("hardware", Deska::Db::Identifier("hpv2"));

    // Set the second one
    parser->parseLine("hardware_name \"foo bar baz\"\r\n");
    expectParsingStarted();
    expectSetAttr("hardware", "hardware_name", Deska::Db::Value(std::string("foo bar baz")));
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("hardware", Deska::Db::Identifier("hpv2"));

    // And terminate the input
    parser->parseLine("end\r\n");
    expectParsingStarted();
    expectCategoryLeft();
    expectParsingFinished();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Set two attributes of an object inline */
BOOST_FIXTURE_TEST_CASE( parsing_two_arguments_inline, ParserTestFixture )
{
    // Start a new context
    parser->parseLine("hardware hpv2 price 666 hardware_name \"foo bar baz\"\r\n");
    expectParsingStarted();
    expectCategoryEntered("hardware", "hpv2");
    expectSetAttr("hardware", "price", Deska::Db::Value(666.0));
    expectSetAttr("hardware", "hardware_name", Deska::Db::Value(std::string("foo bar baz")));
    expectCategoryLeft();
    expectParsingFinished();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Make sure we can indeed handle multiple top-level objects */
BOOST_FIXTURE_TEST_CASE( parsing_two_toplevel, ParserTestFixture )
{
    // create hpv2
    parser->parseLine("hardware hpv2\r\n");
    expectParsingStarted();
    expectCategoryEntered("hardware", "hpv2");
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("hardware", Deska::Db::Identifier("hpv2"));

    // terminate hpv2
    parser->parseLine("end\r\n");
    expectParsingStarted();
    expectCategoryLeft();
    expectParsingFinished();
    expectNothingElse();
    verifyEmptyStack();

    // create second object
    parser->parseLine("host hpv2\r\n");
    expectParsingStarted();
    expectCategoryEntered("host", "hpv2");
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("hpv2"));

    // terminate the host
    parser->parseLine("end\r\n");
    expectParsingStarted();
    expectCategoryLeft();
    expectParsingFinished();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short test correct parsing of multiple arguments, all passed inline */
BOOST_FIXTURE_TEST_CASE(parsing_multiple_arguments_inline, ParserTestFixture)
{
    parser->parseLine("hardware abcde id 1243 hardware_name \"jmeno\" price 1234.5\n");
    expectParsingStarted();
    expectCategoryEntered("hardware", "abcde");
    expectSetAttr("hardware", "id", Deska::Db::Value(1243));
    expectSetAttr("hardware", "hardware_name", Deska::Db::Value(std::string("jmeno")));
    expectSetAttr("hardware", "price", Deska::Db::Value(1234.5));
    expectCategoryLeft();
    expectParsingFinished();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short test correct parsing of simple string argument */
BOOST_FIXTURE_TEST_CASE(parsing_simple_string_inline, ParserTestFixture)
{
    parser->parseLine("hardware abcde id 1243 hardware_name jmeno price 1234.5\n");
    expectParsingStarted();
    expectCategoryEntered("hardware", "abcde");
    expectSetAttr("hardware", "id", Deska::Db::Value(1243));
    expectSetAttr("hardware", "hardware_name", Deska::Db::Value(std::string("jmeno")));
    expectSetAttr("hardware", "price", Deska::Db::Value(1234.5));
    expectCategoryLeft();
    expectParsingFinished();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short error in parsing of simple string argument */
BOOST_FIXTURE_TEST_CASE(error_in_simple_string_inline, ParserTestFixture)
{
    const std::string line = "hardware abcde id 1243 hardware_name jmeno pokracuje price 1234.5\n";
    const std::string::const_iterator it = line.begin() + line.find("pokracuje");
    parser->parseLine(line);
    expectParsingStarted();
    expectCategoryEntered("hardware", "abcde");
    expectSetAttr("hardware", "id", Deska::Db::Value(1243));
    expectSetAttr("hardware", "hardware_name", Deska::Db::Value(std::string("jmeno")));
    expectParseError(Deska::Cli::UndefinedAttributeError("Error while parsing attribute name or nested kind name for hardware. Expected one of [ \"hardware_id\" \"hardware_name\" \"host_name\" \"id\" \"price\" \"role\" \"interface\" ].", line, it));
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Syntax error in the data type of the first attribute

A single-line input which enters a new category and immediately after that encounters an exception.
The idea here is that the stack should not roll back after the exception.
*/
BOOST_FIXTURE_TEST_CASE(error_in_datatype_of_first_inline, ParserTestFixture)
{
    const std::string line = "hardware abcde id xx hardware_name \"jmeno\" price 1234.5\n";
    const std::string::const_iterator it = line.begin() + line.find("xx");
    parser->parseLine(line);
    expectParsingStarted();
    expectCategoryEntered("hardware", "abcde");
    expectParseError(Deska::Cli::InvalidAttributeDataTypeError("Error while parsing argument value for id. Expected one of [ <integer> ].", line, it));
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Syntax error in the name of the first attribute

Similar to error_in_datatype_of_first_inline, but the mistake is not in the value, but rather in the attribute identifier.

@see error_in_datatype_of_first_inline

*/
BOOST_FIXTURE_TEST_CASE(error_in_first_attr_name_inline, ParserTestFixture)
{
    const std::string line = "hardware abcde isd 123 hardware_name \"jmeno\" price 1234.5\n";
    const std::string::const_iterator it = line.begin() + line.find("isd");
    parser->parseLine(line);
    expectParsingStarted();
    expectCategoryEntered("hardware", "abcde");
    expectParseError(Deska::Cli::UndefinedAttributeError("Error while parsing attribute name or nested kind name for hardware. Expected one of [ \"hardware_id\" \"hardware_name\" \"host_name\" \"id\" \"price\" \"role\" \"interface\" ].", line, it));
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Syntax error in the kind of a top-level object */
BOOST_FIXTURE_TEST_CASE(error_toplevel_name, ParserTestFixture)
{
    const std::string line = "haware abcde id 123 hardware_name \"jmeno\" price 1234.5\n";
    const std::string::const_iterator it = line.begin();
    parser->parseLine(line);
    expectParsingStarted();
    expectParseError(Deska::Cli::InvalidObjectKind("Error while parsing kind name. Unknown top-level kind. Expected one of [ \"hardware\" \"host\" \"interface\" ].", line, it));
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Test parsing of an object nested into the parent one */
BOOST_FIXTURE_TEST_CASE(nested_interface, ParserTestFixture)
{
    parser->parseLine("host abcde\n");
    expectParsingStarted();
    expectCategoryEntered("host", "abcde");
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("abcde"));
    
    parser->parseLine("host_name \"as123\"\n");
    expectParsingStarted();
    expectSetAttr("host", "host_name", Deska::Db::Value(std::string("as123")));
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("abcde"));

    parser->parseLine("interface eth0\n");
    expectParsingStarted();
    expectCategoryEntered("interface", "eth0");
    expectParsingFinished();
    expectNothingElse();
    verifyStackTwoLevels("host", Deska::Db::Identifier("abcde"), "interface", Deska::Db::Identifier("eth0"));

    parser->parseLine("mac 00:11:22:33:44:55\n");
    expectParsingStarted();
    expectSetAttr("interface", "mac", Deska::Db::Value(Deska::Db::MacAddress("00:11:22:33:44:55")));
    expectParsingFinished();
    expectNothingElse();
    verifyStackTwoLevels("host", Deska::Db::Identifier("abcde"), "interface", Deska::Db::Identifier("eth0"));
}

/** @short An attribute for parent is listed inline after an embedded object -> fail */
BOOST_FIXTURE_TEST_CASE(nested_interface_inline_with_attr_for_parent, ParserTestFixture)
{
    const std::string line ="host abcde hardware_id 123 host_name \"jmeno\" interface eth0 mac 00:11:22:33:44:55 price 1234.5";
    const std::string::const_iterator it = line.begin() + line.find("price");
    parser->parseLine(line);
    expectParsingStarted();
    expectCategoryEntered("host", "abcde");
    // hardware_id is an identifier, not an int
    expectSetAttr("host", "hardware_id", Deska::Db::Value(std::string("123")));
    expectSetAttr("host", "host_name", Deska::Db::Value(std::string("jmeno")));
    expectCategoryEntered("interface", "eth0");
    expectSetAttr("interface", "mac", Deska::Db::Value(Deska::Db::MacAddress("00:11:22:33:44:55")));
    expectParseError(Deska::Cli::UndefinedAttributeError("Error while parsing attribute name for interface. Expected one of [ \"ip\" \"mac\" ].", line, it));
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Inline definition of an embedded object given immediately after the parent */
BOOST_FIXTURE_TEST_CASE(nested_interface_immediately_inline, ParserTestFixture)
{
    parser->parseLine("host abcde interface eth0 mac 00:11:22:33:44:55\n");
    expectParsingStarted();
    expectCategoryEntered("host", "abcde");
    expectCategoryEntered("interface", "eth0");
    expectSetAttr("interface", "mac", Deska::Db::Value(Deska::Db::MacAddress("00:11:22:33:44:55")));
    expectCategoryLeft();
    expectCategoryLeft();
    expectParsingFinished();
    expectNothingElse();
}


/** @short Inline definition of an embedded object after a paren't attr */
BOOST_FIXTURE_TEST_CASE(nested_interface_after_parent_attr_inline, ParserTestFixture)
{
    parser->parseLine("host abcde hardware_id 1 interface eth0 mac 00:11:22:33:44:55\n");
    expectParsingStarted();
    expectCategoryEntered("host", "abcde");
    expectSetAttr("host", "hardware_id", Deska::Db::Value(std::string("1"))); // identifier, not an int
    expectCategoryEntered("interface", "eth0");
    expectSetAttr("interface", "mac", Deska::Db::Value(Deska::Db::MacAddress("00:11:22:33:44:55")));
    expectCategoryLeft();
    expectCategoryLeft();
    expectParsingFinished();
    expectNothingElse();
}

/** @short Embedding incompatible types after a paren't attribute */
BOOST_FIXTURE_TEST_CASE(embed_incompatible_types_with_attr_inline, ParserTestFixture)
{
    /*
    const std::string line = "hardware abcde id 123 interface eth0";
    const std::string::const_iterator it = line.begin() + line.find("interface");
    parser->parseLine(line);
    expectParsingStarted();
    expectCategoryEntered("hardware", "abcde");
    expectSetAttr("hardware", "id", Deska::Db::Value(123));
    expectParseError(Deska::Cli::UndefinedAttributeError("Error while parsing attribute name or nested kind name for hardware. Expected one of [ \"hardware_id\" \"hardware_name\" \"host_name\" \"id\" \"price\" \"role\" \"interface\" ].", line, it));
    expectNothingElse();
    verifyEmptyStack();
    */
    parser->parseLine("hardware abcde id 123 interface eth0\n");
    expectParsingStarted();
    expectCategoryEntered("hardware", "abcde");
    expectSetAttr("hardware", "id", Deska::Db::Value(123));
    expectCategoryEntered("interface", "eth0");
    expectCategoryLeft();
    expectCategoryLeft();
    expectParsingFinished();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Embedding incompatible types immediately after paren't definition */
BOOST_FIXTURE_TEST_CASE(embed_incompatible_immediately_inline, ParserTestFixture)
{
    /*
    const std::string line = "hardware abcde interface eth0";
    const std::string::const_iterator it = line.begin() + line.find("interface");
    parser->parseLine(line);
    expectParsingStarted();
    expectCategoryEntered("hardware", "abcde");
    expectParseError(Deska::Cli::UndefinedAttributeError("Error while parsing attribute name or nested kind name for hardware. Expected one of [ \"hardware_id\" \"hardware_name\" \"host_name\" \"id\" \"price\" \"role\" \"interface\" ].", line, it));
    expectNothingElse();
    verifyEmptyStack();
    */
    parser->parseLine("hardware abcde interface eth0\n");
    expectParsingStarted();
    expectCategoryEntered("hardware", "abcde");
    expectCategoryEntered("interface", "eth0");
    expectParsingFinished();
    expectNothingElse();
    verifyStackTwoLevels("hardware", Deska::Db::Identifier("abcde"), "interface", Deska::Db::Identifier("eth0"));
}

/** @short An embedded object in an inline form should not cause full rollback to empty state, but stay in the previous context */
BOOST_FIXTURE_TEST_CASE(multiline_with_error_in_inline_embed, ParserTestFixture)
{
    parser->parseLine("host abcde\r\n");
    expectParsingStarted();
    expectCategoryEntered("host", "abcde");
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("abcde"));
    
    parser->parseLine("host_name \"jmeno\"\r\n");
    expectParsingStarted();
    expectSetAttr("host", "host_name", Deska::Db::Value(std::string("jmeno")));
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("abcde"));
    
    const std::string line = "interface eth0 mac 00:11:22:33:44:55 bar baz\r\n";
    const std::string::const_iterator it = line.begin() + line.find("bar");
    parser->parseLine(line);
    expectParsingStarted();
    expectCategoryEntered("interface", "eth0");
    expectSetAttr("interface", "mac", Deska::Db::Value(Deska::Db::MacAddress("00:11:22:33:44:55")));
    expectParseError(Deska::Cli::UndefinedAttributeError("Error while parsing attribute name for interface. Expected one of [ \"ip\" \"mac\" ].", line, it));
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("abcde"));
}

/** @short An embedded object in an inline form should then return to the previous context */
BOOST_FIXTURE_TEST_CASE(multiline_with_inline_embed, ParserTestFixture)
{
    parser->parseLine("host abcde\r\n");
    expectParsingStarted();
    expectCategoryEntered("host", "abcde");
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("abcde"));
    
    parser->parseLine("host_name \"jmeno\"\r\n");
    expectParsingStarted();
    expectSetAttr("host", "host_name", Deska::Db::Value(std::string("jmeno")));
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("abcde"));
    
    parser->parseLine("interface eth0 mac AA:11:CC:33:D4:55\r\n");
    expectParsingStarted();
    expectCategoryEntered("interface", "eth0");
    expectSetAttr("interface", "mac", Deska::Db::Value(Deska::Db::MacAddress("AA:11:CC:33:D4:55")));
    expectCategoryLeft();
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("abcde"));
}

/** @short Generic test for multiline embed */
BOOST_FIXTURE_TEST_CASE(multiline_with_embed, ParserTestFixture)
{
    parser->parseLine("host abcde\r\n");
    expectParsingStarted();
    expectCategoryEntered("host", "abcde");
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("abcde"));

    parser->parseLine("host_name \"jmeno\"\r\n");
    expectParsingStarted();
    expectSetAttr("host", "host_name", Deska::Db::Value(std::string("jmeno")));
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("abcde"));

    parser->parseLine("interface eth0\r\n");
    expectParsingStarted();
    expectCategoryEntered("interface", "eth0");
    expectParsingFinished();
    expectNothingElse();
    verifyStackTwoLevels("host", Deska::Db::Identifier("abcde"), "interface", Deska::Db::Identifier("eth0"));

    parser->parseLine("mac AA:11:CC:33:D4:55\r\n");
    expectParsingStarted();
    expectSetAttr("interface", "mac", Deska::Db::Value(Deska::Db::MacAddress("AA:11:CC:33:D4:55")));
    expectParsingFinished();
    expectNothingElse();
    verifyStackTwoLevels("host", Deska::Db::Identifier("abcde"), "interface", Deska::Db::Identifier("eth0"));

    parser->parseLine("end\r\n");
    expectParsingStarted();
    expectCategoryLeft();
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("abcde"));

    parser->parseLine("end\r\n");
    expectParsingStarted();
    expectCategoryLeft();
    expectParsingFinished();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short An error in multiline embed should not manipulate the context at all */
BOOST_FIXTURE_TEST_CASE(multiline_with_error_in_multiline_embed, ParserTestFixture)
{
    parser->parseLine("host abcde\r\n");
    expectParsingStarted();
    expectCategoryEntered("host", "abcde");
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("abcde"));

    parser->parseLine("host_name \"jmeno\"\r\n");
    expectParsingStarted();
    expectSetAttr("host", "host_name", Deska::Db::Value(std::string("jmeno")));
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("abcde"));

    parser->parseLine("interface eth0\r\n");
    expectParsingStarted();
    expectCategoryEntered("interface", "eth0");
    expectParsingFinished();
    expectNothingElse();
    verifyStackTwoLevels("host", Deska::Db::Identifier("abcde"), "interface", Deska::Db::Identifier("eth0"));

    const std::string line = "maaaac \"foo\"\r\n";
    const std::string::const_iterator it = line.begin();
    parser->parseLine(line);
    expectParsingStarted();
    expectParseError(Deska::Cli::UndefinedAttributeError("Error while parsing attribute name for interface. Expected one of [ \"ip\" \"mac\" ].", line, it));
    expectNothingElse();
    verifyStackTwoLevels("host", Deska::Db::Identifier("abcde"), "interface", Deska::Db::Identifier("eth0"));

    parser->parseLine("end\r\n");
    expectParsingStarted();
    expectCategoryLeft();
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("abcde"));

    parser->parseLine("end\r\n");
    expectParsingStarted();
    expectCategoryLeft();
    expectParsingFinished();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short An error in kind name of embedded object */
BOOST_FIXTURE_TEST_CASE(invalid_kind_name_of_embed_object, ParserTestFixture)
{
    const std::string line = "host 123 int3rf4ce 456\n";
    const std::string::const_iterator it = line.begin() + line.find("int3rf4ce");
    parser->parseLine(line);
    expectParsingStarted();
    expectCategoryEntered("host", "123");
    expectParseError(Deska::Cli::UndefinedAttributeError("Error while parsing attribute name or nested kind name for host. Expected one of [ \"hardware_id\" \"hardware_name\" \"host_name\" \"id\" \"price\" \"role\" \"interface\" ].", line, it));
    expectNothingElse();
    verifyEmptyStack();
}

/** @short An error in kind identifier with single definition on line */
BOOST_FIXTURE_TEST_CASE(error_invalid_kind_name_single_definition, ParserTestFixture)
{
    const std::string line = "haware foo\n";
    const std::string::const_iterator it = line.begin();
    parser->parseLine(line);
    expectParsingStarted();
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
    expectParsingStarted();
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
    expectParsingStarted();
    expectCategoryEntered("hardware", "foo");
    expectParseError(Deska::Cli::UndefinedAttributeError("Error while parsing attribute name or nested kind name for hardware. Expected one of [ \"hardware_id\" \"hardware_name\" \"host_name\" \"id\" \"price\" \"role\" \"interface\" ].", line, it));
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
    expectParsingStarted();
    expectParseError(Deska::Cli::MalformedIdentifier("Error while parsing object name for hardware. Expected one of [ <identifier (alphanumerical letters and _)> ].", line, it));
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Bad identifier of nested object */
BOOST_FIXTURE_TEST_CASE(error_invalid_object_identifier_nested, ParserTestFixture)
{
    const std::string line = "host hpv2 interface foo*bar\n";
    const std::string::const_iterator it = line.begin() + line.find("*bar");
    parser->parseLine(line);
    expectParsingStarted();
    expectCategoryEntered("host", "hpv2");
    expectCategoryEntered("interface", "foo");
    expectParseError(Deska::Cli::UndefinedAttributeError("Error while parsing attribute name for interface. Expected one of [ \"ip\" \"mac\" ].", line, it));
    //expectParseError(Deska::Cli::InvalidAttributeDataTypeError("Error while parsing argument value for hardware. Expected one of [ <identifier (alphanumerical letters and _)> ].", line, it));
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Verify that we can enter into an embedded context with just a single line */
BOOST_FIXTURE_TEST_CASE(nested_kinds_inline_nothing_else, ParserTestFixture)
{
    parser->parseLine("host 123 interface 456\n");
    expectParsingStarted();
    expectCategoryEntered("host", "123");
    expectCategoryEntered("interface", "456");
    expectParsingFinished();
    expectNothingElse();
    verifyStackTwoLevels("host", Deska::Db::Identifier("123"), "interface", Deska::Db::Identifier("456"));

    parser->parseLine("end\n");
    expectParsingStarted();
    expectCategoryLeft();
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("123"));

    parser->parseLine("end\n");
    expectParsingStarted();
    expectCategoryLeft();
    expectParsingFinished();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Verify that we can enter into an embedded context with just a single line and set attributes */
BOOST_FIXTURE_TEST_CASE(nested_kinds_inline_attr, ParserTestFixture)
{
    parser->parseLine("host 123 interface 456 ip 192.168.1.25\n");
    expectParsingStarted();
    expectCategoryEntered("host", "123");
    expectCategoryEntered("interface", "456");
    expectSetAttr("interface", "ip", Deska::Db::Value(boost::asio::ip::address_v4::from_string("192.168.1.25")));
    expectCategoryLeft();
    expectCategoryLeft();
    expectParsingFinished();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Verify that we can use function show in no context */
BOOST_FIXTURE_TEST_CASE(function_show_no_context, ParserTestFixture)
{
    parser->parseLine("show\n");
    expectParsingStarted();
    expectFunctionShow();
    expectParsingFinished();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Verify that we can use function show in context */
BOOST_FIXTURE_TEST_CASE(function_show_in_context, ParserTestFixture)
{
    parser->parseLine("host 123\n");
    expectParsingStarted();
    expectCategoryEntered("host", "123");
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("123"));
    
    parser->parseLine("show\n");
    expectParsingStarted();
    expectFunctionShow();
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("123"));
}

/** @short Verify that we can use function show with parameter in no context */
BOOST_FIXTURE_TEST_CASE(function_show_param_no_context, ParserTestFixture)
{
    const std::string line = "show host 123\n";
    const std::string::const_iterator it = line.begin() + line.find("123");
    parser->parseLine(line);
    expectParsingStarted();
    expectCategoryEntered("host", "123");
    expectParseError(Deska::Cli::ObjectNotFound("Error while parsing object name. Object host 123 does not exist.", line, it));
    //expectFunctionShow();
    //expectCategoryLeft();
    //expectParsingFinished();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Verify that we can use function show with nesting in no context */
BOOST_FIXTURE_TEST_CASE(function_show_nest_no_context, ParserTestFixture)
{
    const std::string line = "show host 123 interface eth0\n";
    const std::string::const_iterator it = line.begin() + line.find("123");
    parser->parseLine(line);
    expectParsingStarted();
    expectCategoryEntered("host", "123");
    expectParseError(Deska::Cli::ObjectNotFound("Error while parsing object name. Object host 123 does not exist.", line, it));
    //expectCategoryEntered("interface", "eth0");
    //expectFunctionShow();
    //expectCategoryLeft();
    //expectCategoryLeft();
    //expectParsingFinished();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Verify that we can use function show with parameter in context */
BOOST_FIXTURE_TEST_CASE(function_show_param_in_context, ParserTestFixture)
{
    parser->parseLine("host 123\n");
    expectParsingStarted();
    expectCategoryEntered("host", "123");
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("123"));

    const std::string line = "show interface 456\n";
    const std::string::const_iterator it = line.begin() + line.find("456");
    parser->parseLine(line);    
    expectParsingStarted();
    expectCategoryEntered("interface", "456");
    expectParseError(Deska::Cli::ObjectNotFound("Error while parsing object name. Object interface 456 does not exist.", line, it));
    //expectFunctionShow();
    //expectCategoryLeft();
    //expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("123"));
}

/** @short Verify that we can not use function delete without parameters in no context */
BOOST_FIXTURE_TEST_CASE(function_delete_no_context, ParserTestFixture)
{
    const std::string line = "delete\n";
    const std::string::const_iterator it = line.end();
    parser->parseLine(line);
    expectParsingStarted();
    expectParseError(Deska::Cli::ObjectDefinitionNotFound("Error while parsing kind name. No object definition found. Expected one of [ \"hardware\" \"host\" \"interface\" ].", line, it));
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Verify that we can not use function delete without parameters in context */
BOOST_FIXTURE_TEST_CASE(function_delete_in_context, ParserTestFixture)
{
    
    parser->parseLine("host 123\n");
    expectParsingStarted();
    expectCategoryEntered("host", "123");
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("123"));
    
    const std::string line = "delete\n";
    const std::string::const_iterator it = line.end();
    parser->parseLine(line);
    expectParsingStarted();
    expectParseError(Deska::Cli::ObjectDefinitionNotFound("Error while parsing kind name. No object definition found. Expected one of [ \"interface\" ].", line, it));
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("123"));
}

/** @short Verify that we can use function delete with parameter in no context */
BOOST_FIXTURE_TEST_CASE(function_delete_param_no_context, ParserTestFixture)
{
    const std::string line = "delete host 123\n";
    const std::string::const_iterator it = line.begin() + line.find("123");
    parser->parseLine(line);
    expectParsingStarted();
    expectCategoryEntered("host", "123");
    expectParseError(Deska::Cli::ObjectNotFound("Error while parsing object name. Object host 123 does not exist.", line, it));
    //expectFunctionDelete();
    //expectCategoryLeft();
    //expectParsingFinished();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Verify that we can use function delete with nesting in no context */
BOOST_FIXTURE_TEST_CASE(function_delete_nest_no_context, ParserTestFixture)
{
    const std::string line = "delete host 123 interface eth0\n";
    const std::string::const_iterator it = line.begin() + line.find("123");
    parser->parseLine(line);
    expectParsingStarted();
    expectCategoryEntered("host", "123");
    expectParseError(Deska::Cli::ObjectNotFound("Error while parsing object name. Object host 123 does not exist.", line, it));
    //expectCategoryEntered("interface", "eth0");
    //expectFunctionDelete();
    //expectCategoryLeft();
    //expectCategoryLeft();
    //expectParsingFinished();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Verify that we can use function delete with parameter in context */
BOOST_FIXTURE_TEST_CASE(function_delete_param_in_context, ParserTestFixture)
{
    parser->parseLine("host 123\n");
    expectParsingStarted();
    expectCategoryEntered("host", "123");
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("123"));
    
    const std::string line = "delete interface 456\n";
    const std::string::const_iterator it = line.begin() + line.find("456");
    parser->parseLine(line);
    expectParsingStarted();
    expectCategoryEntered("interface", "456");
    expectParseError(Deska::Cli::ObjectNotFound("Error while parsing object name. Object interface 456 does not exist.", line, it));
    //expectFunctionDelete();
    //expectCategoryLeft();
    //expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("123"));
}

/** @short Bad kind name in function delete with parameter in no context */
BOOST_FIXTURE_TEST_CASE(error_invalid_kind_name_function_delete_param_no_context, ParserTestFixture)
{
    const std::string line = "delete hot 123\n";
    const std::string::const_iterator it = line.begin() + line.find("hot");
    parser->parseLine(line);
    expectParsingStarted();
    expectParseError(Deska::Cli::InvalidObjectKind("Error while parsing kind name. Unknown top-level kind. Expected one of [ \"hardware\" \"host\" \"interface\" ].", line, it));
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Bad kind name in function delete with nesting in no context */
BOOST_FIXTURE_TEST_CASE(error_invalid_kind_name_function_delete_nest_no_context, ParserTestFixture)
{
    const std::string line = "delete host 123 inteface eth0\n";
    const std::string::const_iterator it = line.begin() + line.find("123");//("inteface");
    parser->parseLine(line);
    expectParsingStarted();
    expectCategoryEntered("host", "123");
    expectParseError(Deska::Cli::ObjectNotFound("Error while parsing object name. Object host 123 does not exist.", line, it));
    //expectParseError(Deska::Cli::InvalidObjectKind("Error while parsing kind name of nested object in host. Expected one of [ \"interface\" ].", line, it));
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Bad kind name in function delete with parameter in context */
BOOST_FIXTURE_TEST_CASE(error_invalid_kind_name_function_delete_param_in_context, ParserTestFixture)
{
    parser->parseLine("host 123\n");
    expectParsingStarted();
    expectCategoryEntered("host", "123");
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("123"));
    
    const std::string line = "delete inteface eth0\n";
    const std::string::const_iterator it = line.begin() + line.find("inteface");
    parser->parseLine(line);
    expectParsingStarted();
    expectParseError(Deska::Cli::InvalidObjectKind("Error while parsing kind name of nested object in host. Expected one of [ \"interface\" ].", line, it));
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("123"));
}

/** @short Bad kind name in function show with parameter in no context */
BOOST_FIXTURE_TEST_CASE(error_invalid_kind_name_function_show_param_no_context, ParserTestFixture)
{
    const std::string line = "show hot 123\n";
    const std::string::const_iterator it = line.begin() + line.find("hot");
    parser->parseLine(line);
    expectParsingStarted();
    expectParseError(Deska::Cli::InvalidObjectKind("Error while parsing kind name. Unknown top-level kind. Expected one of [ \"hardware\" \"host\" \"interface\" ].", line, it));
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Bad kind name in function show with nesting in no context */
BOOST_FIXTURE_TEST_CASE(error_invalid_kind_name_function_show_nest_no_context, ParserTestFixture)
{
    const std::string line = "show host 123 inteface eth0\n";
    const std::string::const_iterator it = line.begin() + line.find("123");//("inteface");
    parser->parseLine(line);
    expectParsingStarted();
    expectCategoryEntered("host", "123");
    expectParseError(Deska::Cli::ObjectNotFound("Error while parsing object name. Object host 123 does not exist.", line, it));
    //expectParseError(Deska::Cli::InvalidObjectKind("Error while parsing kind name of nested object in host. Expected one of [ \"interface\" ].", line, it));
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Bad kind name in function show with parameter in context */
BOOST_FIXTURE_TEST_CASE(error_invalid_kind_name_function_show_param_in_context, ParserTestFixture)
{
    parser->parseLine("host 123\n");
    expectParsingStarted();
    expectCategoryEntered("host", "123");
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("123"));
    
    const std::string line = "show inteface eth0\n";
    const std::string::const_iterator it = line.begin() + line.find("inteface");
    parser->parseLine(line);
    expectParsingStarted();
    expectParseError(Deska::Cli::InvalidObjectKind("Error while parsing kind name of nested object in host. Expected one of [ \"interface\" ].", line, it));
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("123"));
}

/** @short Function show with parameter in context where are no nested kinds */
BOOST_FIXTURE_TEST_CASE(error_function_show_param_in_context_no_nested, ParserTestFixture)
{
    parser->parseLine("hardware 123\n");
    expectParsingStarted();
    expectCategoryEntered("hardware", "123");
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("hardware", Deska::Db::Identifier("123"));
    
    /*
    const std::string line = "show interface eth0\n";
    const std::string::const_iterator it = line.begin() + line.find("interface");
    parser->parseLine(line);
    expectParsingStarted();
    expectParseError(Deska::Cli::InvalidObjectKind("Error while parsing kind name of nested object in hardware.", line, it));
    */
    const std::string line = "show interface eth0\n";
    const std::string::const_iterator it = line.begin() + line.find("eth0");
    parser->parseLine(line);
    expectParsingStarted();
    expectCategoryEntered("interface", "eth0");
    expectParseError(Deska::Cli::ObjectNotFound("Error while parsing object name. Object interface eth0 does not exist.", line, it));
    expectNothingElse();
    verifyStackOneLevel("hardware", Deska::Db::Identifier("123"));
}

/** @short Function delete with parameter in context where are no nested kinds */
BOOST_FIXTURE_TEST_CASE(error_function_delete_param_in_context_no_nested, ParserTestFixture)
{
    parser->parseLine("hardware 123\n");
    expectParsingStarted();
    expectCategoryEntered("hardware", "123");
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("hardware", Deska::Db::Identifier("123"));
    
    const std::string line = "show interface eth0\n";
    const std::string::const_iterator it = line.begin() + line.find("eth0");
    parser->parseLine(line);
    expectParsingStarted();
    expectCategoryEntered("interface", "eth0");
    expectParseError(Deska::Cli::ObjectNotFound("Error while parsing object name. Object interface eth0 does not exist.", line, it));
    expectNothingElse();
    verifyStackOneLevel("hardware", Deska::Db::Identifier("123"));
}

/** @short Remove an attribute from an object using verbose syntax */
BOOST_FIXTURE_TEST_CASE( parsing_trivial_remove_argument, ParserTestFixture )
{
    // Start a new context
    parser->parseLine("hardware hpv2\r\n");
    expectParsingStarted();
    expectCategoryEntered("hardware", "hpv2");
    expectParsingFinished();
    expectNothingElse();

    // Verify stack nesting
    verifyStackOneLevel("hardware", Deska::Db::Identifier("hpv2"));

    // Set the attribute
    parser->parseLine("no hardware_name\r\n");
    expectParsingStarted();
    expectRemoveAttr("hardware", "hardware_name");
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("hardware", Deska::Db::Identifier("hpv2"));

    // And terminate the input
    parser->parseLine("end\r\n");
    expectParsingStarted();
    expectCategoryLeft();
    expectParsingFinished();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Remove an attribute from an object using the inline syntax*/
BOOST_FIXTURE_TEST_CASE( parsing_trivial_remove_argument_inline, ParserTestFixture )
{
    // Start a new context
    parser->parseLine("hardware hpv2 no hardware_name\r\n");
    expectParsingStarted();
    expectCategoryEntered("hardware", "hpv2");
    expectRemoveAttr("hardware", "hardware_name");
    expectCategoryLeft();
    expectParsingFinished();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Remove two attributes of an object using the multiline variant of the syntax */
BOOST_FIXTURE_TEST_CASE( parsing_remove_two_arguments, ParserTestFixture )
{
    // Start a new context
    parser->parseLine("hardware hpv2\r\n");
    expectParsingStarted();
    expectCategoryEntered("hardware", "hpv2");
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("hardware", Deska::Db::Identifier("hpv2"));

    // Remove the first attribute
    parser->parseLine("no price\r\n");
    expectParsingStarted();
    expectRemoveAttr("hardware", "price");
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("hardware", Deska::Db::Identifier("hpv2"));

    // Remove the second one
    parser->parseLine("no hardware_name\r\n");
    expectParsingStarted();
    expectRemoveAttr("hardware", "hardware_name");
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("hardware", Deska::Db::Identifier("hpv2"));

    // And terminate the input
    parser->parseLine("end\r\n");
    expectParsingStarted();
    expectCategoryLeft();
    expectParsingFinished();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Set one attribute and remove one of an object using the multiline variant of the syntax */
BOOST_FIXTURE_TEST_CASE( parsing_set_one_remove_one_argument, ParserTestFixture )
{
    // Start a new context
    parser->parseLine("hardware hpv2\r\n");
    expectParsingStarted();
    expectCategoryEntered("hardware", "hpv2");
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("hardware", Deska::Db::Identifier("hpv2"));

    // Set the first attribute
    parser->parseLine("price 666\r\n");
    expectParsingStarted();
    expectSetAttr("hardware", "price", Deska::Db::Value(666.0));
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("hardware", Deska::Db::Identifier("hpv2"));

    // Remove the second one
    parser->parseLine("no hardware_name\r\n");
    expectParsingStarted();
    expectRemoveAttr("hardware", "hardware_name");
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("hardware", Deska::Db::Identifier("hpv2"));

    // And terminate the input
    parser->parseLine("end\r\n");
    expectParsingStarted();
    expectCategoryLeft();
    expectParsingFinished();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Remove one attribute and set one of an object using the multiline variant of the syntax */
BOOST_FIXTURE_TEST_CASE( parsing_remove_one_set_one_argument, ParserTestFixture )
{
    // Start a new context
    parser->parseLine("hardware hpv2\r\n");
    expectParsingStarted();
    expectCategoryEntered("hardware", "hpv2");
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("hardware", Deska::Db::Identifier("hpv2"));

    // Remove the first attribute
    parser->parseLine("no price\r\n");
    expectParsingStarted();
    expectRemoveAttr("hardware", "price");
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("hardware", Deska::Db::Identifier("hpv2"));

    // Set the second one
    parser->parseLine("hardware_name \"foo bar baz\"\r\n");
    expectParsingStarted();
    expectSetAttr("hardware", "hardware_name", Deska::Db::Value(std::string("foo bar baz")));
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("hardware", Deska::Db::Identifier("hpv2"));

    // And terminate the input
    parser->parseLine("end\r\n");
    expectParsingStarted();
    expectCategoryLeft();
    expectParsingFinished();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Remove and set one attribute of an object using the multiline variant of the syntax */
BOOST_FIXTURE_TEST_CASE( parsing_remove_and_set_one_argument, ParserTestFixture )
{
    // Start a new context
    parser->parseLine("hardware hpv2\r\n");
    expectParsingStarted();
    expectCategoryEntered("hardware", "hpv2");
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("hardware", Deska::Db::Identifier("hpv2"));

    // Remove the attribute
    parser->parseLine("no price\r\n");
    expectParsingStarted();
    expectRemoveAttr("hardware", "price");
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("hardware", Deska::Db::Identifier("hpv2"));

    // Set it again
    parser->parseLine("price 666\r\n");
    expectParsingStarted();
    expectSetAttr("hardware", "price", Deska::Db::Value(666.0));
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("hardware", Deska::Db::Identifier("hpv2"));

    // And terminate the input
    parser->parseLine("end\r\n");
    expectParsingStarted();
    expectCategoryLeft();
    expectParsingFinished();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Set and remove one attribute of an object using the multiline variant of the syntax */
BOOST_FIXTURE_TEST_CASE( parsing_set_and_remove_one_argument, ParserTestFixture )
{
    // Start a new context
    parser->parseLine("hardware hpv2\r\n");
    expectParsingStarted();
    expectCategoryEntered("hardware", "hpv2");
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("hardware", Deska::Db::Identifier("hpv2"));

    // Set the attribute
    parser->parseLine("price 666\r\n");
    expectParsingStarted();
    expectSetAttr("hardware", "price", Deska::Db::Value(666.0));
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("hardware", Deska::Db::Identifier("hpv2"));
    
    // And remove it
    parser->parseLine("no price\r\n");
    expectParsingStarted();
    expectRemoveAttr("hardware", "price");
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("hardware", Deska::Db::Identifier("hpv2"));

    // And terminate the input
    parser->parseLine("end\r\n");
    expectParsingStarted();
    expectCategoryLeft();
    expectParsingFinished();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Remove two attributes of an object inline */
BOOST_FIXTURE_TEST_CASE( parsing_remove_two_arguments_inline, ParserTestFixture )
{
    // Start a new context
    parser->parseLine("hardware hpv2 no price no hardware_name\r\n");
    expectParsingStarted();
    expectCategoryEntered("hardware", "hpv2");
    expectRemoveAttr("hardware", "price");
    expectRemoveAttr("hardware", "hardware_name");
    expectCategoryLeft();
    expectParsingFinished();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Set one attribute and remove one of an object inline */
BOOST_FIXTURE_TEST_CASE( parsing_set_one_remove_one_argument_inline, ParserTestFixture )
{
    // Start a new context
    parser->parseLine("hardware hpv2 price 666 no hardware_name\r\n");
    expectParsingStarted();
    expectCategoryEntered("hardware", "hpv2");
    expectSetAttr("hardware", "price", Deska::Db::Value(666.0));
    expectRemoveAttr("hardware", "hardware_name");
    expectCategoryLeft();
    expectParsingFinished();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Remove one attribute and set one of an object inline */
BOOST_FIXTURE_TEST_CASE( parsing_remove_one_set_one_argument_inline, ParserTestFixture )
{
    // Start a new context
    parser->parseLine("hardware hpv2 no price hardware_name \"foo bar baz\"\r\n");
    expectParsingStarted();
    expectCategoryEntered("hardware", "hpv2");
    expectRemoveAttr("hardware", "price");
    expectSetAttr("hardware", "hardware_name", Deska::Db::Value(std::string("foo bar baz")));
    expectCategoryLeft();
    expectParsingFinished();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Remove and set one attribute of an object inline */
BOOST_FIXTURE_TEST_CASE( parsing_remove_and_set_one_argument_inline, ParserTestFixture )
{
    // Start a new context
    parser->parseLine("hardware hpv2 no price price 666\r\n");
    expectParsingStarted();
    expectCategoryEntered("hardware", "hpv2");
    expectRemoveAttr("hardware", "price");
    expectSetAttr("hardware", "price", Deska::Db::Value(666.0));
    expectCategoryLeft();
    expectParsingFinished();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Set and remove one attribute of an object inline */
BOOST_FIXTURE_TEST_CASE( parsing_set_and_remove_one_argument_inline, ParserTestFixture )
{
    // Start a new context
    parser->parseLine("hardware hpv2 price 666 no price\r\n");
    expectParsingStarted();
    expectCategoryEntered("hardware", "hpv2");
    expectSetAttr("hardware", "price", Deska::Db::Value(666.0));
    expectRemoveAttr("hardware", "price");
    expectCategoryLeft();
    expectParsingFinished();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short test correct parsing of multiple arguments removal, all passed inline */
BOOST_FIXTURE_TEST_CASE(parsing_remove_multiple_arguments_inline, ParserTestFixture)
{
    parser->parseLine("hardware abcde no id no hardware_name no price\n");
    expectParsingStarted();
    expectCategoryEntered("hardware", "abcde");
    expectRemoveAttr("hardware", "id");
    expectRemoveAttr("hardware", "hardware_name");
    expectRemoveAttr("hardware", "price");
    expectCategoryLeft();
    expectParsingFinished();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Syntax error in the name of the first attribute removal */
BOOST_FIXTURE_TEST_CASE(error_in_first_attr_name_removal_inline, ParserTestFixture)
{
    const std::string line = "hardware abcde no isd hardware_name \"jmeno\" price 1234.5\n";
    const std::string::const_iterator it = line.begin() + line.find(" isd");
    parser->parseLine(line);
    expectParsingStarted();
    expectCategoryEntered("hardware", "abcde");
    expectParseError(Deska::Cli::UndefinedAttributeError("Error while parsing attribute name for hardware. Expected one of [ \"hardware_id\" \"hardware_name\" \"host_name\" \"id\" \"price\" \"role\" ].", line, it));
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Test parsing of an object nested into the parent one with attributes removal */
BOOST_FIXTURE_TEST_CASE(nested_interface_attrs_removal, ParserTestFixture)
{
    parser->parseLine("host abcde\n");
    expectParsingStarted();
    expectCategoryEntered("host", "abcde");
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("abcde"));
    
    parser->parseLine("no host_name\n");
    expectParsingStarted();
    expectRemoveAttr("host", "host_name");
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("abcde"));

    parser->parseLine("interface eth0\n");
    expectParsingStarted();
    expectCategoryEntered("interface", "eth0");
    expectParsingFinished();
    expectNothingElse();
    verifyStackTwoLevels("host", Deska::Db::Identifier("abcde"), "interface", Deska::Db::Identifier("eth0"));

    parser->parseLine("no mac\n");
    expectParsingStarted();
    expectRemoveAttr("interface", "mac");
    expectParsingFinished();
    expectNothingElse();
    verifyStackTwoLevels("host", Deska::Db::Identifier("abcde"), "interface", Deska::Db::Identifier("eth0"));
}

/** @short Inline definition of an embedded object given immediately after the parent with attribute removal */
BOOST_FIXTURE_TEST_CASE(nested_interface_immediately_attr_removal_inline, ParserTestFixture)
{
    parser->parseLine("host abcde interface eth0 no mac\n");
    expectParsingStarted();
    expectCategoryEntered("host", "abcde");
    expectCategoryEntered("interface", "eth0");
    expectRemoveAttr("interface", "mac");
    expectCategoryLeft();
    expectCategoryLeft();
    expectParsingFinished();
    expectNothingElse();
}


/** @short Inline definition of an embedded object after a paren't attr with attribute removal */
BOOST_FIXTURE_TEST_CASE(nested_interface_after_parent_attr_attr_removal_inline, ParserTestFixture)
{
    parser->parseLine("host abcde hardware_id 1 interface eth0 no mac\n");
    expectParsingStarted();
    expectCategoryEntered("host", "abcde");
    expectSetAttr("host", "hardware_id", Deska::Db::Value(std::string("1"))); // identifier, not an int
    expectCategoryEntered("interface", "eth0");
    expectRemoveAttr("interface", "mac");
    expectCategoryLeft();
    expectCategoryLeft();
    expectParsingFinished();
    expectNothingElse();
}

/** @short An embedded object in an inline form should then return to the previous context while removin attribute */
BOOST_FIXTURE_TEST_CASE(multiline_with_inline_embed_attr_remove, ParserTestFixture)
{
    parser->parseLine("host abcde\r\n");
    expectParsingStarted();
    expectCategoryEntered("host", "abcde");
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("abcde"));
    
    parser->parseLine("host_name \"jmeno\"\r\n");
    expectParsingStarted();
    expectSetAttr("host", "host_name", Deska::Db::Value(std::string("jmeno")));
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("abcde"));
    
    parser->parseLine("interface eth0 no mac\r\n");
    expectParsingStarted();
    expectCategoryEntered("interface", "eth0");
    expectRemoveAttr("interface", "mac");
    expectCategoryLeft();
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("abcde"));
}

/** @short Generic test for multiline embed with attributes remove*/
BOOST_FIXTURE_TEST_CASE(multiline_with_embed_attrs_remove, ParserTestFixture)
{
    parser->parseLine("host abcde\r\n");
    expectParsingStarted();
    expectCategoryEntered("host", "abcde");
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("abcde"));

    parser->parseLine("no host_name\r\n");
    expectParsingStarted();
    expectRemoveAttr("host", "host_name");
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("abcde"));

    parser->parseLine("interface eth0\r\n");
    expectParsingStarted();
    expectCategoryEntered("interface", "eth0");
    expectParsingFinished();
    expectNothingElse();
    verifyStackTwoLevels("host", Deska::Db::Identifier("abcde"), "interface", Deska::Db::Identifier("eth0"));

    parser->parseLine("no mac\r\n");
    expectParsingStarted();
    expectRemoveAttr("interface", "mac");
    expectParsingFinished();
    expectNothingElse();
    verifyStackTwoLevels("host", Deska::Db::Identifier("abcde"), "interface", Deska::Db::Identifier("eth0"));

    parser->parseLine("end\r\n");
    expectParsingStarted();
    expectCategoryLeft();
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("abcde"));

    parser->parseLine("end\r\n");
    expectParsingStarted();
    expectCategoryLeft();
    expectParsingFinished();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short An error in multiline embed should not manipulate the context at all while removing attribute*/
BOOST_FIXTURE_TEST_CASE(multiline_with_error_in_multiline_embed_attr_remove, ParserTestFixture)
{
    parser->parseLine("host abcde\r\n");
    expectParsingStarted();
    expectCategoryEntered("host", "abcde");
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("abcde"));

    parser->parseLine("no host_name\r\n");
    expectParsingStarted();
    expectRemoveAttr("host", "host_name");
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("abcde"));

    parser->parseLine("interface eth0\r\n");
    expectParsingStarted();
    expectCategoryEntered("interface", "eth0");
    expectParsingFinished();
    expectNothingElse();
    verifyStackTwoLevels("host", Deska::Db::Identifier("abcde"), "interface", Deska::Db::Identifier("eth0"));

    const std::string line = "no maaaac\r\n";
    const std::string::const_iterator it = line.begin() + line.find(" maaaac");
    parser->parseLine(line);
    expectParsingStarted();
    expectParseError(Deska::Cli::UndefinedAttributeError("Error while parsing attribute name for interface. Expected one of [ \"ip\" \"mac\" ].", line, it));
    expectNothingElse();
    verifyStackTwoLevels("host", Deska::Db::Identifier("abcde"), "interface", Deska::Db::Identifier("eth0"));

    parser->parseLine("end\r\n");
    expectParsingStarted();
    expectCategoryLeft();
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("abcde"));

    parser->parseLine("end\r\n");
    expectParsingStarted();
    expectCategoryLeft();
    expectParsingFinished();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Verify that we can enter into an embedded context with just a single line and remove attribute */
BOOST_FIXTURE_TEST_CASE(nested_kinds_inline_attr_removal, ParserTestFixture)
{
    parser->parseLine("host 123 interface 456 no ip\n");
    expectParsingStarted();
    expectCategoryEntered("host", "123");
    expectCategoryEntered("interface", "456");
    expectRemoveAttr("interface", "ip");
    expectCategoryLeft();
    expectCategoryLeft();
    expectParsingFinished();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Verify that when parsing removal of invalid attribute, only attributes and no nested kinds will be expected */
BOOST_FIXTURE_TEST_CASE(invalid_attr_removal, ParserTestFixture)
{
    const std::string line = "host 123 no bar\n";
    const std::string::const_iterator it = line.begin() + line.find(" bar");
    parser->parseLine(line);
    expectParsingStarted();
    expectCategoryEntered("host", "123");
    expectParseError(Deska::Cli::UndefinedAttributeError("Error while parsing attribute name for host. Expected one of [ \"hardware_id\" \"hardware_name\" \"host_name\" \"id\" \"price\" \"role\" ].", line, it));
    expectNothingElse();
    verifyEmptyStack();
}

/** @short We cant use function rename with no context and no parameter */
BOOST_FIXTURE_TEST_CASE(error_function_rename_no_context, ParserTestFixture)
{
    const std::string line = "rename\n";
    const std::string::const_iterator it = line.end();
    parser->parseLine(line);
    expectParsingStarted();
    expectParseError(Deska::Cli::ObjectDefinitionNotFound("Error while parsing kind name. No object definition found. Expected one of [ \"hardware\" \"host\" \"interface\" ].", line, it));
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Verify that we can not use function rename in context without object specifying*/
BOOST_FIXTURE_TEST_CASE(function_rename_in_context_no_object, ParserTestFixture)
{
    parser->parseLine("host 123\n");
    expectParsingStarted();
    expectCategoryEntered("host", "123");
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("123"));
    
    const std::string line ="rename 456\n";
    const std::string::const_iterator it = line.begin() + line.find("456");
    parser->parseLine(line);
    expectParsingStarted();
    expectParseError(Deska::Cli::InvalidObjectKind("Error while parsing kind name of nested object in host. Expected one of [ \"interface\" ].", line, it));
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("123"));
}

/** @short Verify that we can use function rename with parameter in no context */
BOOST_FIXTURE_TEST_CASE(function_rename_param_no_context, ParserTestFixture)
{
    const std::string line = "rename host 123 456\n";
    const std::string::const_iterator it = line.begin() + line.find("123");
    parser->parseLine(line);
    expectParsingStarted();
    expectCategoryEntered("host", "123");
    expectParseError(Deska::Cli::ObjectNotFound("Error while parsing object name. Object host 123 does not exist.", line, it));
    //expectFunctionRename("456");
    //expectCategoryLeft();
    //expectParsingFinished();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Verify that we can use function rename with nesting in no context */
BOOST_FIXTURE_TEST_CASE(function_rename_nest_no_context, ParserTestFixture)
{
    const std::string line = "rename host 123 interface eth0 eth1\n";
    const std::string::const_iterator it = line.begin() + line.find("123");
    parser->parseLine(line);
    expectParsingStarted();
    expectCategoryEntered("host", "123");
    expectParseError(Deska::Cli::ObjectNotFound("Error while parsing object name. Object host 123 does not exist.", line, it));
    //expectCategoryEntered("interface", "eth0");
    //expectFunctionRename("eth1");
    //expectCategoryLeft();
    //expectCategoryLeft();
    //expectParsingFinished();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Verify that we can use function rename with parameter in context */
BOOST_FIXTURE_TEST_CASE(function_rename_param_in_context, ParserTestFixture)
{
    parser->parseLine("host 123\n");
    expectParsingStarted();
    expectCategoryEntered("host", "123");
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("123"));

    const std::string line = "rename interface 456 789\n";
    const std::string::const_iterator it = line.begin() + line.find("456");
    parser->parseLine(line);  
    expectParsingStarted();
    expectCategoryEntered("interface", "456");
    expectParseError(Deska::Cli::ObjectNotFound("Error while parsing object name. Object interface 456 does not exist.", line, it));
    //expectFunctionRename("789");
    //expectCategoryLeft();
    //expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("123"));
}

/** @short We can not use function rename without specifying new name */
BOOST_FIXTURE_TEST_CASE(error_function_rename_no_ident_in_context, ParserTestFixture)
{
    parser->parseLine("host 123\n");
    expectParsingStarted();
    expectCategoryEntered("host", "123");
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("123"));

    const std::string line = "rename interface 456\n";
    const std::string::const_iterator it = line.begin() + line.find("456");
    //const std::string::const_iterator it = line.end();
    parser->parseLine(line);    
    expectParsingStarted();
    expectCategoryEntered("interface", "456");
    expectParseError(Deska::Cli::ObjectNotFound("Error while parsing object name. Object interface 456 does not exist.", line, it));
    //expectParseError(Deska::Cli::MalformedIdentifier("Error while parsing object identifier. Correct identifier not found or too much data entered.", line, it));
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("123"));
}

/** @short We can not use function rename without specifying new name in no context */
BOOST_FIXTURE_TEST_CASE(error_function_rename_no_ident_no_context, ParserTestFixture)
{
    const std::string line = "rename host 456\n";
    const std::string::const_iterator it = line.begin() + line.find("456");
    //const std::string::const_iterator it = line.end();
    parser->parseLine(line); 
    expectParsingStarted();
    expectCategoryEntered("host", "456");
    expectParseError(Deska::Cli::ObjectNotFound("Error while parsing object name. Object host 456 does not exist.", line, it));
    //expectParseError(Deska::Cli::MalformedIdentifier("Error while parsing object identifier. Correct identifier not found or too much data entered.", line, it));
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Enter some more data when renaming an object */
BOOST_FIXTURE_TEST_CASE(error_function_rename_more_data, ParserTestFixture)
{
    const std::string line = "rename host 456 789 abc\n";
    const std::string::const_iterator it = line.begin() + line.find("456");
    //const std::string::const_iterator it = line.begin() + line.find("abc");
    parser->parseLine(line);   
    expectParsingStarted();
    expectCategoryEntered("host", "456");
    expectParseError(Deska::Cli::ObjectNotFound("Error while parsing object name. Object host 456 does not exist.", line, it));
    //expectParseError(Deska::Cli::MalformedIdentifier("Error while parsing object identifier. Correct identifier not found or too much data entered.", line, it));
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Jumping from top level directly in the interface */
BOOST_FIXTURE_TEST_CASE(jump_in_context, ParserTestFixture)
{
    parser->parseLine("interface hpv2->eth0\n");   
    expectParsingStarted();
    expectCategoryEntered("host", "hpv2");
    expectCategoryEntered("interface", "eth0");
    expectParsingFinished();
    expectNothingElse();
    verifyStackTwoLevels("host", Deska::Db::Identifier("hpv2"), "interface", Deska::Db::Identifier("eth0"));
}

/** @short Jumping error */
BOOST_FIXTURE_TEST_CASE(jump_in_context_error, ParserTestFixture)
{
    const std::string line = "hardware hpv2->eth0\n";
    const std::string::const_iterator it = line.begin();
    parser->parseLine(line);   
    expectParsingStarted();
    expectParseError(Deska::Cli::MalformedIdentifier("Error while parsing object name for hardware hpv2->eth0. Can't find nesting parents.", line, it));
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Another jumping error */
BOOST_FIXTURE_TEST_CASE(jump_in_context_error_2, ParserTestFixture)
{
    const std::string line = "interface eth0\n";
    const std::string::const_iterator it = line.begin();
    parser->parseLine(line);   
    expectParsingStarted();
    expectParseError(Deska::Cli::MalformedIdentifier("Error while parsing object name for interface eth0. Can't find nesting parents.", line, it));
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Setting an identifiers set as whole value */
BOOST_FIXTURE_TEST_CASE(attrs_sets_set, ParserTestFixture)
{
    parser->parseLine("host hpv2\n");   
    expectParsingStarted();
    expectCategoryEntered("host", "hpv2");
    expectParsingFinished();
    verifyStackOneLevel("host", Deska::Db::Identifier("hpv2"));

    parser->parseLine("role [www, ftp, dns]\n");
    expectParsingStarted();
    std::set<Deska::Db::Identifier> roles;
    roles.insert("www");
    roles.insert("ftp");
    roles.insert("dns");
    expectSetAttr("host", "role", Deska::Db::Value(roles));
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("hpv2"));
}

/** @short Insertion into an identifiers set */
BOOST_FIXTURE_TEST_CASE(attrs_sets_insert, ParserTestFixture)
{
    parser->parseLine("host hpv2\n");   
    expectParsingStarted();
    expectCategoryEntered("host", "hpv2");
    expectParsingFinished();
    verifyStackOneLevel("host", Deska::Db::Identifier("hpv2"));

    parser->parseLine("add role www\n");
    expectParsingStarted();
    expectSetAttrInsert("host", "role", "www");
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("hpv2"));
}

/** @short Removal from an identifiers set */
BOOST_FIXTURE_TEST_CASE(attrs_sets_remove, ParserTestFixture)
{
    parser->parseLine("host hpv2\n");   
    expectParsingStarted();
    expectCategoryEntered("host", "hpv2");
    expectParsingFinished();
    verifyStackOneLevel("host", Deska::Db::Identifier("hpv2"));

    parser->parseLine("remove role www\n");
    expectParsingStarted();
    expectSetAttrRemove("host", "role", "www");
    expectParsingFinished();
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("hpv2"));
}

/** @short Insertion into an ordinary attribute error */
BOOST_FIXTURE_TEST_CASE(error_attrs_sets_insert, ParserTestFixture)
{
    parser->parseLine("host hpv2\n");   
    expectParsingStarted();
    expectCategoryEntered("host", "hpv2");
    expectParsingFinished();
    verifyStackOneLevel("host", Deska::Db::Identifier("hpv2"));

    verifyStackOneLevel("host", Deska::Db::Identifier("hpv2"));
    const std::string line = "add note www\n";
    const std::string::const_iterator it = line.begin() + line.find(" note");
    parser->parseLine(line);
    expectParsingStarted();
    expectParseError(Deska::Cli::UndefinedAttributeError("Error while parsing identifiers set name for host. Expected one of [ \"role\" ].", line, it));
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("hpv2"));
}

/** @short Removal from an ordinary attribute error */
BOOST_FIXTURE_TEST_CASE(error_attrs_sets_remove, ParserTestFixture)
{
    parser->parseLine("host hpv2\n");   
    expectParsingStarted();
    expectCategoryEntered("host", "hpv2");
    expectParsingFinished();
    verifyStackOneLevel("host", Deska::Db::Identifier("hpv2"));

    verifyStackOneLevel("host", Deska::Db::Identifier("hpv2"));
    const std::string line = "remove note www\n";
    const std::string::const_iterator it = line.begin() + line.find(" note");
    parser->parseLine(line);
    expectParsingStarted();
    expectParseError(Deska::Cli::UndefinedAttributeError("Error while parsing identifiers set name for host. Expected one of [ \"role\" ].", line, it));
    expectNothingElse();
    verifyStackOneLevel("host", Deska::Db::Identifier("hpv2"));
}

/** @short Insertion into an identifiers set */
BOOST_FIXTURE_TEST_CASE(error_attrs_sets_insert_2, ParserTestFixture)
{
    parser->parseLine("host hpv2 interface eth0\n");
    expectParsingStarted();
    expectCategoryEntered("host", "hpv2");
    expectCategoryEntered("interface", "eth0");
    expectParsingFinished();
    verifyStackTwoLevels("host", Deska::Db::Identifier("hpv2"), "interface", Deska::Db::Identifier("eth0"));

    const std::string line = "add role www\n";
    const std::string::const_iterator it = line.begin() + line.find("add");
    parser->parseLine(line);
    expectParsingStarted();
    expectParseError(Deska::Cli::UndefinedAttributeError("Error while parsing attribute name for interface. Expected one of [ \"ip\" \"mac\" ].", line, it));
    expectNothingElse();
    verifyStackTwoLevels("host", Deska::Db::Identifier("hpv2"), "interface", Deska::Db::Identifier("eth0"));
}

/** @short Removal from an identifiers set */
BOOST_FIXTURE_TEST_CASE(eror_attrs_sets_remove_2, ParserTestFixture)
{
    parser->parseLine("host hpv2 interface eth0\n");
    expectParsingStarted();
    expectCategoryEntered("host", "hpv2");
    expectCategoryEntered("interface", "eth0");
    expectParsingFinished();
    verifyStackTwoLevels("host", Deska::Db::Identifier("hpv2"), "interface", Deska::Db::Identifier("eth0"));

    const std::string line = "remove role www\n";
    const std::string::const_iterator it = line.begin() + line.find("remove");
    parser->parseLine(line);
    expectParsingStarted();
    expectParseError(Deska::Cli::UndefinedAttributeError("Error while parsing attribute name for interface. Expected one of [ \"ip\" \"mac\" ].", line, it));
    expectNothingElse();
    verifyStackTwoLevels("host", Deska::Db::Identifier("hpv2"), "interface", Deska::Db::Identifier("eth0"));
}

/** @short Entering merged kind and setting attribute */
BOOST_FIXTURE_TEST_CASE(merged_kind_set_attr, ParserTestFixture)
{
    parser->parseLine("hardware xx interface eth0 ip 1.1.1.1\n");
    expectParsingStarted();
    expectCategoryEntered("hardware", "xx");
    expectCategoryEntered("interface", "eth0");
    expectSetAttr("interface", "ip", Deska::Db::Value(boost::asio::ip::address_v4::from_string("1.1.1.1")));
    expectCategoryLeft();
    expectCategoryLeft();
    expectParsingFinished();
    expectNothingElse();
    verifyEmptyStack();
}

/** @short Construction of new interface */
BOOST_FIXTURE_TEST_CASE(new_interface, ParserTestFixture)
{
    parser->parseLine("host hpv2\n");   
    expectParsingStarted();
    expectCategoryEntered("host", "hpv2");
    expectParsingFinished();
    verifyStackOneLevel("host", Deska::Db::Identifier("hpv2"));

    parser->parseLine("new interface\n");
    expectParsingStarted();
    expectCreateObject("interface", "");
    expectParsingFinished();
    verifyStackTwoLevels("host", Deska::Db::Identifier("hpv2"), "interface", Deska::Db::Identifier(""));

    parser->parseLine("mac aa:bb:cc:dd:ee:11\n");
    expectParsingStarted();
    expectSetAttr("interface", "mac", Deska::Db::Value(Deska::Db::MacAddress("aa:bb:cc:dd:ee:11")));
    expectParsingFinished();
    verifyStackTwoLevels("host", Deska::Db::Identifier("hpv2"), "interface", Deska::Db::Identifier(""));
    expectNothingElse();
}

/** @short Construction of new interface inline */
BOOST_FIXTURE_TEST_CASE(new_interface_inline, ParserTestFixture)
{
    parser->parseLine("host hpv2\n");   
    expectParsingStarted();
    expectCategoryEntered("host", "hpv2");
    expectParsingFinished();
    verifyStackOneLevel("host", Deska::Db::Identifier("hpv2"));

    parser->parseLine("new interface mac aa:bb:cc:dd:ee:11\n");
    expectParsingStarted();
    expectCreateObject("interface", "");
    expectSetAttr("interface", "mac", Deska::Db::Value(Deska::Db::MacAddress("aa:bb:cc:dd:ee:11")));
    expectCategoryLeft();
    expectParsingFinished();
    verifyStackOneLevel("host", Deska::Db::Identifier("hpv2"));
    expectNothingElse();
}

/** @short Test merged attributes */
BOOST_FIXTURE_TEST_CASE(merge_relation, ParserTestFixture)
{
    parser->parseLine("host hpv2\n");   
    expectParsingStarted();
    expectCategoryEntered("host", "hpv2");
    expectParsingFinished();
    verifyStackOneLevel("host", Deska::Db::Identifier("hpv2"));

    parser->parseLine("price 666");
    expectParsingStarted();
    expectSetAttr("hardware", "price", Deska::Db::Value(666.0));
    expectParsingFinished();
    verifyStackOneLevel("host", Deska::Db::Identifier("hpv2"));
    expectNothingElse();
}

/** @short Construction of new object with bad kind name */
BOOST_FIXTURE_TEST_CASE(error_new_object, ParserTestFixture)
{
    parser->parseLine("host hpv2\n");   
    expectParsingStarted();
    expectCategoryEntered("host", "hpv2");
    expectParsingFinished();
    verifyStackOneLevel("host", Deska::Db::Identifier("hpv2"));

    const std::string line = "new lol\n";
    const std::string::const_iterator it = line.begin() + line.find(" lol");
    parser->parseLine(line);
    expectParsingStarted();
    expectParseError(Deska::Cli::InvalidObjectKind("Error while parsing kind name of nested object in host. Expected one of [ \"interface\" ].", line, it));
    verifyStackOneLevel("host", Deska::Db::Identifier("hpv2"));
    expectNothingElse();
}

/** @short Simple filter */
BOOST_FIXTURE_TEST_CASE(simple_filter, ParserTestFixture)
{
    parser->parseLine("host where (host_name == \"cervena karkulka\")\n");   
    expectParsingStarted();
    expectObjectsFilter("host", boost::optional<Deska::Db::Filter>(Deska::Db::AttributeExpression(Deska::Db::FILTER_COLUMN_EQ, "host", "host_name", Deska::Db::Value(std::string("cervena karkulka")))));
    expectParsingFinished();
    verifyStackOneLevel("host", boost::optional<Deska::Db::Filter>(Deska::Db::AttributeExpression(Deska::Db::FILTER_COLUMN_EQ, "host", "host_name", Deska::Db::Value(std::string("cervena karkulka")))));
    expectNothingElse();
}

/** @short Joining filter */
BOOST_FIXTURE_TEST_CASE(joining_filter, ParserTestFixture)
{
    parser->parseLine("host where (interface.ip == 192.168.15.32)\n");   
    expectParsingStarted();
    expectObjectsFilter("host", boost::optional<Deska::Db::Filter>(Deska::Db::AttributeExpression(Deska::Db::FILTER_COLUMN_EQ, "interface", "ip", Deska::Db::Value(boost::asio::ip::address_v4::from_string("192.168.15.32")))));
    expectParsingFinished();
    verifyStackOneLevel("host", boost::optional<Deska::Db::Filter>(Deska::Db::AttributeExpression(Deska::Db::FILTER_COLUMN_EQ, "interface", "ip", Deska::Db::Value(boost::asio::ip::address_v4::from_string("192.168.15.32")))));
    expectNothingElse();
}

/** @short And filter */
BOOST_FIXTURE_TEST_CASE(joining_and_filter, ParserTestFixture)
{
    parser->parseLine("host where ((interface.ip == 192.168.15.32) & (host_name != \"some name\"))\n");   
    expectParsingStarted();
    std::vector<Deska::Db::Filter> exprs;
    exprs.push_back(Deska::Db::AttributeExpression(Deska::Db::FILTER_COLUMN_EQ, "interface", "ip",
        Deska::Db::Value(boost::asio::ip::address_v4::from_string("192.168.15.32"))));
    exprs.push_back(Deska::Db::AttributeExpression(Deska::Db::FILTER_COLUMN_NE, "host", "host_name",
        Deska::Db::Value(std::string("some name"))));
    expectObjectsFilter("host", boost::optional<Deska::Db::Filter>(Deska::Db::AndFilter(exprs)));
    expectParsingFinished();
    verifyStackOneLevel("host", boost::optional<Deska::Db::Filter>(Deska::Db::AndFilter(exprs)));
    expectNothingElse();
}

/** @short More complicated nested filter */
BOOST_FIXTURE_TEST_CASE(joining_and_or_nested_filter, ParserTestFixture)
{
    parser->parseLine("host where (((interface.ip == 192.168.15.32) & (host_name != \"some name\")) | (role contains www))\n");
    expectParsingStarted();
    std::vector<Deska::Db::Filter> exprs;
    exprs.push_back(Deska::Db::AttributeExpression(Deska::Db::FILTER_COLUMN_EQ, "interface", "ip",
        Deska::Db::Value(boost::asio::ip::address_v4::from_string("192.168.15.32"))));
    exprs.push_back(Deska::Db::AttributeExpression(Deska::Db::FILTER_COLUMN_NE, "host", "host_name",
        Deska::Db::Value(std::string("some name"))));
    std::vector<Deska::Db::Filter> exprs2;
    exprs2.push_back(Deska::Db::AndFilter(exprs));
    exprs2.push_back(Deska::Db::AttributeExpression(Deska::Db::FILTER_COLUMN_CONTAINS, "host", "role",
        Deska::Db::Value(std::string("www"))));
    expectObjectsFilter("host", boost::optional<Deska::Db::Filter>(Deska::Db::OrFilter(exprs2)));
    expectParsingFinished();
    verifyStackOneLevel("host", boost::optional<Deska::Db::Filter>(Deska::Db::OrFilter(exprs2)));
    expectNothingElse();
}

/** @short Filter error in attribute value */
BOOST_FIXTURE_TEST_CASE(error_filter_attribute_value, ParserTestFixture)
{
    std::string line = "host where (interface.ip == 192.15.32)\n";
    const std::string::const_iterator it = line.begin() + line.find("192.15.32");
    parser->parseLine(line);
    expectParsingStarted();
    expectParseError(Deska::Cli::InvalidAttributeDataTypeError("Error while parsing argument value for ip. Expected one of [ <IPv4 address> ].", line, it));
    verifyEmptyStack();
    expectNothingElse();
}

/** @short Filter error in attribute name */
BOOST_FIXTURE_TEST_CASE(error_filter_attribute_name, ParserTestFixture)
{
    std::string line = "host where (blabla == 192.15.32)\n";
    const std::string::const_iterator it = line.begin() + line.find("blabla");
    parser->parseLine(line);
    expectParsingStarted();
    expectParseError(Deska::Cli::UndefinedAttributeError("Error while parsing attribute name or nested kind name for host. Expected one of [ \"hardware_id\" \"hardware_name\" \"host_name\" \"id\" \"price\" \"role\" \"hardware\" \"host\" \"interface\" ].", line, it));
    verifyEmptyStack();
    expectNothingElse();
}

/** @short Filter for all objects */
BOOST_FIXTURE_TEST_CASE(filter_all_objects, ParserTestFixture)
{
    parser->parseLine("all host\n");
    expectParsingStarted();
    expectObjectsFilter("host", boost::optional<Deska::Db::Filter>());
    expectParsingFinished();
    verifyStackOneLevel("host", boost::optional<Deska::Db::Filter>());
    expectNothingElse();
}

/** @short Filter for all nested objects accessed from top-level */
BOOST_FIXTURE_TEST_CASE(filter_all_objects_2, ParserTestFixture)
{
    parser->parseLine("all interface\n");
    expectParsingStarted();
    expectObjectsFilter("interface", boost::optional<Deska::Db::Filter>());
    expectParsingFinished();
    verifyStackOneLevel("interface", boost::optional<Deska::Db::Filter>());
    expectNothingElse();
}

/** @short Filter for all nested objects */
BOOST_FIXTURE_TEST_CASE(filter_all_nested_objects, ParserTestFixture)
{
    parser->parseLine("host hpv2\n");   
    expectParsingStarted();
    expectCategoryEntered("host", "hpv2");
    expectParsingFinished();
    verifyStackOneLevel("host", Deska::Db::Identifier("hpv2"));

    parser->parseLine("all interface\n");
    expectParsingStarted();
    expectObjectsFilter("interface", boost::optional<Deska::Db::Filter>());
    expectParsingFinished();
    verifyStackTwoLevels("host", Deska::Db::Identifier("hpv2"), "interface", boost::optional<Deska::Db::Filter>());
    expectNothingElse();
}

/** @short Filter for last nested objects */
BOOST_FIXTURE_TEST_CASE(filter_last_nested_objects, ParserTestFixture)
{
    parser->parseLine("host hpv2\n");   
    expectParsingStarted();
    expectCategoryEntered("host", "hpv2");
    expectParsingFinished();
    verifyStackOneLevel("host", Deska::Db::Identifier("hpv2"));

    parser->parseLine("last interface\n");
    expectParsingStarted();
    expectObjectsFilter("interface", boost::optional<Deska::Db::Filter>(Deska::Db::SpecialExpression(Deska::Db::FILTER_SPECIAL_EMBEDDED_LAST_ONE, "interface")));
    expectParsingFinished();
    verifyStackTwoLevels("host", Deska::Db::Identifier("hpv2"), "interface", boost::optional<Deska::Db::Filter>(Deska::Db::SpecialExpression(Deska::Db::FILTER_SPECIAL_EMBEDDED_LAST_ONE, "interface")));
    expectNothingElse();
}

/** @short Error in filter for all objects */
BOOST_FIXTURE_TEST_CASE(error_filter_all_objects, ParserTestFixture)
{
    std::string line = "all bla\n";
    const std::string::const_iterator it = line.begin() + line.find(" bla");
    parser->parseLine(line);
    expectParsingStarted();
    expectParseError(Deska::Cli::InvalidObjectKind("Error while parsing kind name in a special filter. Unknown kind. Expected one of [ \"hardware\" \"host\" \"interface\" ].", line, it));
    verifyEmptyStack();
    expectNothingElse();
}
