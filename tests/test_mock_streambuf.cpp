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
#include <iostream>

struct MockStreamFixture {
    MockStreamBuffer buf;
    std::istream is;
    std::ostream os;
    MockStreamFixture(const std::size_t bufsize=666):
        buf(bufsize), is(&buf), os(&buf)
    {
    }
};

BOOST_FIXTURE_TEST_CASE(simple_write, MockStreamFixture)
{
    buf.expectWrite("ahoj");
    os << "ahoj" << std::flush;
    BOOST_CHECK(buf.consumedEverything());
}

BOOST_FIXTURE_TEST_CASE(simple_read_space, MockStreamFixture)
{
    buf.expectRead("ahoj ");
    std::string tmp;
    is >> tmp;
    BOOST_CHECK_EQUAL(tmp, std::string("ahoj"));
    BOOST_CHECK(buf.consumedEverything());
}

BOOST_FIXTURE_TEST_CASE(simple_read_eof, MockStreamFixture)
{
    buf.expectRead("ahoj");
    buf.expectReadEof();
    std::string tmp;
    is >> tmp;
    BOOST_CHECK_EQUAL(tmp, std::string("ahoj"));
    BOOST_CHECK(buf.consumedEverything());
}

BOOST_FIXTURE_TEST_CASE(simple_rw, MockStreamFixture)
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
}
