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

#include <boost/foreach.hpp>
#define BOOST_TEST_MODULE example
#include <boost/test/unit_test.hpp>
#include "JsonApiTestFixture.h"
#include "deska/db/CachingJsonApi.h"

using std::vector;
using std::map;
using namespace Deska::Db;

/** @short Make sure that CachingJsonApi's caching really works

This tests tries to verify that when we call any metadata-querying method of the API, the caching layer
will rmember the result and use it for further inquiries.
*/
BOOST_FIXTURE_TEST_CASE(json_kindNames, JsonApiTestFixture)
{
    // At first, it has to find out what the top-level object types are
    expectWrite("{\"command\":\"getTopLevelObjectNames\"}");
    expectRead("{\"response\": \"getTopLevelObjectNames\", \"topLevelObjectKinds\": [\"a\", \"b\"]}");
    // The, for each of them, it asks for a list of attributes, and then for a list of relations.
    // Start with "a":
    expectWrite("{\"command\":\"getKindAttributes\",\"kindName\":\"a\"}");
    expectRead("{\"kindAttributes\": {\"bar\": \"int\", \"baz\": \"identifier\", \"foo\": \"string\", "
            "\"price\": \"double\"}, \"kindName\": \"a\", \"response\": \"getKindAttributes\"}");
    expectWrite("{\"command\":\"getKindRelations\",\"kindName\":\"a\"}");
    expectRead("{\"kindName\": \"a\", \"kindRelations\": [[\"IS_TEMPLATE\", \"b\"]], \"response\": \"getKindRelations\"}");
    // ...and move to "b":
    expectWrite("{\"command\":\"getKindAttributes\",\"kindName\":\"b\"}");
    expectRead("{\"kindAttributes\": {\"name\": \"string\", \"name_of_a\": \"identifier\"}, \"kindName\": \"b\", \"response\": \"getKindAttributes\"}");
    expectWrite("{\"command\":\"getKindRelations\",\"kindName\":\"b\"}");
    expectRead("{\"kindName\": \"b\", \"kindRelations\": [[\"TEMPLATIZED\", \"a\", \"name_of_a\"]], \"response\": \"getKindRelations\"}");

    // This is ugly, but in order to reuse the JsonApiTestFixture, we'll have to hack around this:
    delete j;
    j = new Deska::Db::CachingJsonApi();
    // FIXME: update streams here

    // Now, the first call to the API will request everything. Let's start with the kind names, for example.
    vector<Identifier> kindNames = j->kindNames();

    // Nothing else should be in the I/O queue
    expectEmpty();

    // Check that kindNames() works
    vector<Identifier> expectedKindNames;
    expectedKindNames.push_back("a");
    expectedKindNames.push_back("b");
    BOOST_CHECK_EQUAL_COLLECTIONS(kindNames.begin(), kindNames.end(), expectedKindNames.begin(), expectedKindNames.end());

    // Now, check the kindAttributes() for both top-level kinds
    // ...start with "a":
    vector<KindAttributeDataType> expectedAttrs;
    expectedAttrs.push_back(KindAttributeDataType("bar", TYPE_INT));
    expectedAttrs.push_back(KindAttributeDataType("baz", TYPE_IDENTIFIER));
    expectedAttrs.push_back(KindAttributeDataType("foo", TYPE_STRING));
    expectedAttrs.push_back(KindAttributeDataType("price", TYPE_DOUBLE));
    vector<KindAttributeDataType> resAttrs = j->kindAttributes("a");
    BOOST_CHECK_EQUAL_COLLECTIONS(resAttrs.begin(), resAttrs.end(), expectedAttrs.begin(), expectedAttrs.end());

    // ...and continue with "b":
    expectedAttrs.clear();
    expectedAttrs.push_back(KindAttributeDataType("name", TYPE_STRING));
    expectedAttrs.push_back(KindAttributeDataType("name_of_a", TYPE_IDENTIFIER));
    resAttrs = j->kindAttributes("b");
    BOOST_CHECK_EQUAL_COLLECTIONS(resAttrs.begin(), resAttrs.end(), expectedAttrs.begin(), expectedAttrs.end());

    // Now, let's check the relations. And start with checking "b" just for fun:
    vector<ObjectRelation> expectedRelations;
    expectedRelations.push_back(ObjectRelation::templatized("a", "name_of_a"));
    vector<ObjectRelation> resRelations = j->kindRelations("b");
    BOOST_CHECK_EQUAL_COLLECTIONS(resRelations.begin(), resRelations.end(), expectedRelations.begin(), expectedRelations.end());

    // ...and continue with "a":
    expectedRelations.clear();
    expectedRelations.push_back(ObjectRelation::isTemplate("b"));
    resRelations = j->kindRelations("a");
    BOOST_CHECK_EQUAL_COLLECTIONS(resRelations.begin(), resRelations.end(), expectedRelations.begin(), expectedRelations.end());

    // Why not verify that we can ask for something we already asked for, and still get the same result?
    kindNames = j->kindNames();
    BOOST_CHECK_EQUAL_COLLECTIONS(kindNames.begin(), kindNames.end(), expectedKindNames.begin(), expectedKindNames.end());

    // ...and let's repeat it also for soemthing else.
    resRelations = j->kindRelations("a");
    BOOST_CHECK_EQUAL_COLLECTIONS(resRelations.begin(), resRelations.end(), expectedRelations.begin(), expectedRelations.end());

    // The JsonApiTestFixture's destructor will free j.
}
