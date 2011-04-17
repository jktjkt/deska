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
#include "MockStreamBuffer.h"

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

    MockStreamBuffer mockBuffer;
    std::istream readStream;
    std::ostream writeStream;

    std::istream *getReadStream();
    std::ostream *getWriteStream();
    void bindStreams();
};

/** @short A fixture that will complain when the streams complain */
struct JsonApiTestFixtureFailOnStreamThrow: public JsonApiTestFixture
{
    JsonApiTestFixtureFailOnStreamThrow()
    {
        mockBuffer.useBoostTestOnThrow();
    }
};

#endif // DESKA_TEST_JSONAPITESTFIXTURE_H
