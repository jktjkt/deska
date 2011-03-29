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
#include <boost/bind.hpp>

#include "JsonApiTestFixture.h"
#include "deska/db/JsonApi.h"

#include <iostream>

JsonApiTestFixture::JsonApiTestFixture()
{
    j = new Deska::Db::JsonApiParser();
    // FIXME: change this to actually check the data, and provide some output back...
    //j->setStreams(&writeStream, &readStream);
}

JsonApiTestFixture::~JsonApiTestFixture()
{
    delete j;
}

void JsonApiTestFixture::expectEmpty()
{
    BOOST_CHECK(jsonDbInput.empty());
    BOOST_CHECK(jsonDbOutput.empty());
}

void JsonApiTestFixture::expectRead(const std::string &str)
{
    jsonDbOutput.push(str);
}

void JsonApiTestFixture::expectWrite(const std::string &str)
{
    jsonDbInput.push(str);
}

std::streamsize StreamTupleSource::read(char *s, std::streamsize n)
{
    std::cerr << "Called read(" << n << ")" << std::endl;
    if (p_->events_.empty()) {
        std::cerr << "Tried to read, but no further IO expected" << std::endl;
        throw "Tried to read, but no further IO expected";
    }
    MockStreamEvent &e = p_->events_.front();
    if (e.mode_ != MockStreamEvent::READ) {
        std::cerr << "Tried to read, but th expected operation is not a read" << std::endl;
        throw "Tried to read, but th expected operation is not a read";
    }
    std::streamsize toRead = std::min(n, static_cast<std::streamsize>(e.data_.size() - e.offset_));
    std::cerr << "toRead " << toRead << std::endl;
    BOOST_ASSERT(toRead > 0);
    std::copy(e.data_.begin() + e.offset_, e.data_.begin() + e.offset_ + toRead, s);
    e.offset_ += toRead;
    if (e.offset_ == e.data_.size()) {
        std::cerr << "killing line" << std::endl;
        p_->events_.pop();
    }
    std::cerr << "return " << toRead << std::endl;
    return toRead;
}

MockStreamTuple::MockStreamTuple():
   istream_(&ibuf_)
{
    ibuf_.open(StreamTupleSource(this));
}

/*std::ostream &MockStreamTuple::ostream()
{
    return ostream_;
}*/

std::istream &MockStreamTuple::istream()
{
    return istream_;
}

void MockStreamTuple::expectRead(const std::string &s)
{
    events_.push(MockStreamEvent(MockStreamEvent::READ, s));
}

void MockStreamTuple::expectWrite(const std::string &s)
{
    events_.push(MockStreamEvent(MockStreamEvent::WRITE, s));
}

void MockStreamTuple::verifyEmpty()
{
    if (!events_.empty())
        throw std::runtime_error("Some IO did not happen");
}
