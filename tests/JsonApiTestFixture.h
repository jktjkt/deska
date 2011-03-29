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

#ifndef DESKA_TEST_JSONAPITESTFIXTURE_H
#define DESKA_TEST_JSONAPITESTFIXTURE_H

#include <string>
#include <queue>
#include <boost/iostreams/concepts.hpp>
#include <boost/iostreams/stream.hpp>
#include <stdexcept>

namespace Deska {
namespace Db {
class JsonApiParser;
}
}

struct JsonApiTestFixture
{
    JsonApiTestFixture();
    ~JsonApiTestFixture();

    void expectRead(const std::string &str);
    void expectWrite(const std::string &str);
    void expectEmpty();

    Deska::Db::JsonApiParser *j;

    std::queue<std::string> jsonDbInput;
    std::queue<std::string> jsonDbOutput;
};




struct MockStreamEvent {
    typedef enum {READ, WRITE} Direction;
    Direction mode_;
    std::string data_;
    std::streamsize offset_;

    MockStreamEvent(const Direction mode, const std::string &data):
        mode_(mode), data_(data), offset_(0)
    {
    }
};

class MockStreamTuple;

class StreamTupleSource: public boost::iostreams::source {
public:
    StreamTupleSource(MockStreamTuple *p):
        p_(p)
    {
    }

    std::streamsize read(char *s, std::streamsize n);
private:
    MockStreamTuple *p_;
};

class StreamTupleSink: public boost::iostreams::sink {
public:
    StreamTupleSink(MockStreamTuple *p):
        p_(p)
    {
    }

    std::streamsize write(char *s, std::streamsize n);
private:
    MockStreamTuple *p_;
};

class MockStreamTuple {
public:
    MockStreamTuple();

    //std::ostream &ostream();
    std::istream &istream();

    /** @short Next operation expected from the device is read, and we will return this data */
    void expectRead(const std::string &s);

    /** @short Next expected operation is a write with this data */
    void expectWrite(const std::string &s);

    /** @short Scream loudly if there's any pending IO */
    void verifyEmpty();

private:
    std::queue<MockStreamEvent> events_;
    friend class StreamTupleSource;
    friend class StreamTupleSink;
    //std::ostream ostream_;
    boost::iostreams::stream_buffer<StreamTupleSource> ibuf_;
    std::istream istream_;
};

#endif // DESKA_TEST_JSONAPITESTFIXTURE_H
