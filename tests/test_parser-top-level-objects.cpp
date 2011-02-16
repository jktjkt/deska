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

#include "deska/db/FakeApi.h"
#include "deska/cli/Parser.h"

struct F {
    F()
    {
        using namespace Deska;
        Deska::FakeApi *fake = new FakeApi();
        fake->attrs["hardware"].push_back( KindAttributeDataType( "id", "integer" ) );
        fake->attrs["hardware"].push_back( KindAttributeDataType( "name", "string" ) );
        fake->attrs["hardware"].push_back( KindAttributeDataType( "price", "integer" ) );
        fake->attrs["interface"].push_back( KindAttributeDataType( "ip", "string" ) );
        fake->attrs["interface"].push_back( KindAttributeDataType( "mac", "string" ) );
        fake->attrs["host"].push_back( KindAttributeDataType( "hardware", "identifier" ) );
        fake->attrs["host"].push_back( KindAttributeDataType( "name", "string" ) );

        fake->relations["interface"].push_back( ObjectRelation::embedInto("host") );
        db = fake;
    }
    ~F()
    {
        delete db;
    }

    Deska::Api *db;
};


BOOST_FIXTURE_TEST_CASE( parsing_top_level_objects, F )
{
    Deska::CLI::Parser parser(db);
}
