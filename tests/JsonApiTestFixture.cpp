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
#include <boost/test/unit_test_monitor.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include "JsonApiTestFixture.h"
#include "deska/db/JsonApi.h"

JsonApiTestFixture::JsonApiTestFixture():
    mockBuffer(1), readStream(&mockBuffer), writeStream(&mockBuffer)
{
    j = new Deska::Db::JsonApiParser();
    bindStreams();
    boost::unit_test::unit_test_monitor.register_exception_translator<Deska::Db::JsonParseError>(&handleJsonParseError);
}

JsonApiTestFixture::~JsonApiTestFixture()
{
    delete j;
}

void JsonApiTestFixture::expectEmpty()
{
    BOOST_CHECK(mockBuffer.consumedEverything());
}

void JsonApiTestFixture::expectRead(const std::string &str)
{
    mockBuffer.expectRead(str);
}

void JsonApiTestFixture::expectWrite(const std::string &str)
{
    mockBuffer.expectWrite(str);
}

std::istream *JsonApiTestFixture::getReadStream()
{
    return &readStream;
}

std::ostream *JsonApiTestFixture::getWriteStream()
{
    return &writeStream;
}

void JsonApiTestFixture::bindStreams()
{
    j->willRead.connect(boost::phoenix::bind(&JsonApiTestFixture::getReadStream, this));
    j->willWrite.connect(boost::phoenix::bind(&JsonApiTestFixture::getWriteStream, this));
}

void handleJsonParseError(const Deska::Db::JsonParseError &e)
{
    BOOST_ERROR(e.whatWithBacktrace());
}
