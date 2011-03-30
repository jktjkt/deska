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

#include "MockStreamBuffer.h"

struct MockStreamFixture
{
    MockStreamBuffer buf;
    std::istream is;
    std::ostream os;
    bool thrown;
    MockStreamFixture(const std::size_t bufsize=1):
        buf(bufsize), is(&buf), os(&buf), thrown(false)
    {
    }
};

struct MockStreamFixtureNoThrow: public MockStreamFixture
{
    MockStreamFixtureNoThrow()
    {
        buf.useBoostTestOnThrow();
    }
};

/** @short Make sure we can catch a simple write to the stream */
BOOST_FIXTURE_TEST_CASE(simple_write, MockStreamFixtureNoThrow)
{
    buf.expectWrite("ahoj");
    os << "ahoj" << std::flush;
    BOOST_CHECK(buf.consumedEverything());
    BOOST_REQUIRE(!(is.bad() || os.bad() || thrown));
}

/** @short Test reading into a string, separated by a whitespace */
BOOST_FIXTURE_TEST_CASE(simple_read_space, MockStreamFixtureNoThrow)
{
    buf.expectRead("ahoj ");
    std::string tmp;
    is >> tmp;
    BOOST_CHECK_EQUAL(tmp, std::string("ahoj"));
    BOOST_CHECK(buf.consumedEverything());
    // we aren't at EOF yet, we're supposed to be at the space
    BOOST_CHECK(!is.eof());
    BOOST_REQUIRE(!(is.bad() || os.bad() || thrown));
}

/** @short Test reading into a string immediately followed by an EOF */
BOOST_FIXTURE_TEST_CASE(simple_read_eof, MockStreamFixtureNoThrow)
{
    buf.expectRead("ahoj");
    buf.expectReadEof();
    std::string tmp;
    is >> tmp;
    BOOST_CHECK_EQUAL(tmp, std::string("ahoj"));
    BOOST_CHECK(buf.consumedEverything());
    BOOST_CHECK(is.eof());
    BOOST_REQUIRE(!(is.bad() || os.bad() || thrown));
}

/** @short Test a simple RW interaction */
BOOST_FIXTURE_TEST_CASE(simple_rw, MockStreamFixtureNoThrow)
{
    buf.expectWrite("foo bar baz");
    buf.expectWrite("pwn");
    buf.expectRead("abc ");
    buf.expectReadEof();
    os << "foo " << "bar " << "baz" << std::flush;
    os << "pwn" << std::flush;

    std::string tmp;
    is >> tmp;
    BOOST_CHECK_EQUAL(tmp, "abc");
    BOOST_CHECK(!is.fail());
    BOOST_CHECK(!is.eof());
    is >> tmp;
    BOOST_CHECK(is.fail());
    BOOST_CHECK(is.eof());
    BOOST_CHECK(buf.consumedEverything());
    BOOST_REQUIRE(!(is.bad() || os.bad() || thrown));
}

/** @short Unexpected read */
BOOST_FIXTURE_TEST_CASE(unexpected_read, MockStreamFixture)
{
    try {
        std::string tmp;
        is >> tmp;
    } catch (MockStreamBufferError&) {
        thrown = true;
    }
    //BOOST_CHECK(is.bad());
    BOOST_CHECK(thrown);
    BOOST_REQUIRE(is.bad() || os.bad() || thrown);
    BOOST_CHECK(buf.consumedEverything());
}

/** @short Unexpected read instead of write */
BOOST_FIXTURE_TEST_CASE(unexpected_read_instead_of_write, MockStreamFixture)
{
    buf.expectWrite("x");
    try {
        std::string tmp;
        is >> tmp;
    } catch (MockStreamBufferError&) {
        thrown = true;
    }
    //BOOST_CHECK(is.bad());
    BOOST_CHECK(thrown);
    BOOST_REQUIRE(is.bad() || os.bad() || thrown);
    os << "x" << std::flush;
    BOOST_CHECK(buf.consumedEverything());
}

/** @short Unexpected write */
BOOST_FIXTURE_TEST_CASE(unexpected_write, MockStreamFixture)
{
    try {
        os << 123;
    } catch (MockStreamBufferError&) {
        thrown = true;
    }
    BOOST_CHECK(os.bad());
    //BOOST_CHECK(thrown);
    BOOST_REQUIRE(is.bad() || os.bad() || thrown);
    BOOST_CHECK(buf.consumedEverything());
}

/** @short Unexpected write instead of read */
BOOST_FIXTURE_TEST_CASE(unexpected_write_instead_of_read, MockStreamFixture)
{
    buf.expectRead("");
    try {
        os << 666;
    } catch (MockStreamBufferError&) {
        thrown = true;
    }
    BOOST_CHECK(os.bad());
    //BOOST_CHECK(thrown);
    BOOST_REQUIRE(is.bad() || os.bad() || thrown);
    BOOST_CHECK_EQUAL(is.get(), std::istream::traits_type::eof());
    BOOST_CHECK(buf.consumedEverything());
}

/** @short Unexpected write instead of reading an EOF */
BOOST_FIXTURE_TEST_CASE(unexpected_write_instead_of_read_eof, MockStreamFixture)
{
    buf.expectReadEof();
    try {
        os << 666;
    } catch (MockStreamBufferError&) {
        thrown = true;
    }
    BOOST_CHECK(os.bad());
    //BOOST_CHECK(thrown);
    BOOST_REQUIRE(is.bad() || os.bad() || thrown);
    BOOST_CHECK_EQUAL(is.get(), std::istream::traits_type::eof());
    BOOST_CHECK(buf.consumedEverything());
}
