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
#include "JsonApiTestFixture.h"
#include "deska/db/JsonApi.h"

using std::vector;
using namespace Deska;

BOOST_FIXTURE_TEST_CASE(json_kindNames, JsonApiTestFixture)
{
    jsonDbInput = "{\"command\":\"getTopLevelObjectNames\"}";
    jsonDbOutput = "{\"response\": \"getTopLevelObjectNames\", \"topLevelObjectKinds\": [\"z\", \"a\", \"b\", \"foo bar\"]}";
    vector<Identifier> expected;
    expected.push_back("z");
    expected.push_back("a");
    expected.push_back("b");
    expected.push_back("foo bar");
    vector<Identifier> res = j->kindNames();
    BOOST_CHECK_EQUAL_COLLECTIONS(res.begin(), res.end(), expected.begin(), expected.end());
}

BOOST_FIXTURE_TEST_CASE(json_kindAttributes, JsonApiTestFixture)
{
    jsonDbInput = "{\"command\":\"getKindAttributes\",\"kindName\":\"some-object\"}";
    jsonDbOutput = "{\"kindAttributes\": {\"bar\": \"int\", \"baz\": \"identifier\", \"foo\": \"string\", "
            "\"price\": \"double\"}, \"kindName\": \"some-object\", \"response\": \"getKindAttributes\"}";
    vector<KindAttributeDataType> expected;
    expected.push_back(KindAttributeDataType("bar", TYPE_INT));
    expected.push_back(KindAttributeDataType("baz", TYPE_IDENTIFIER));
    expected.push_back(KindAttributeDataType("foo", TYPE_STRING));
    expected.push_back(KindAttributeDataType("price", TYPE_DOUBLE));
    vector<KindAttributeDataType> res = j->kindAttributes("some-object");
    BOOST_CHECK_EQUAL_COLLECTIONS(res.begin(), res.end(), expected.begin(), expected.end());
}

BOOST_FIXTURE_TEST_CASE(json_kindAttributes_wrong_object, JsonApiTestFixture)
{
    jsonDbInput = "{\"command\":\"getKindAttributes\",\"kindName\":\"some-object\"}";
    jsonDbOutput = "{\"kindAttributes\": {\"bar\": \"int\", \"baz\": \"identifier\", \"foo\": \"string\", "
            "\"price\": \"double\"}, \"kindName\": \"some-object-2\", \"response\": \"getKindAttributes\"}";
    BOOST_CHECK_THROW(j->kindAttributes("some-object"), JsonParseError);
}

BOOST_FIXTURE_TEST_CASE(json_kindRelations, JsonApiTestFixture)
{
    jsonDbInput = "{\"command\":\"getKindRelations\",\"kindName\":\"identifier\"}";
    jsonDbOutput = "{\"kindName\": \"identifier\", \"kindRelations\": [[\"EMBED_INTO\", \"hardware\"], "
            "[\"MERGE_WITH\", \"second-kind\", \"my-attribute\"], [\"IS_TEMPLATE\", \"target-kind\"], "
            "[\"TEMPLATIZED\", \"by-which-kind\", \"my-attribute\"]], \"response\": \"getKindRelations\"}";
    vector<ObjectRelation> expected;
    expected.push_back(ObjectRelation::embedInto("hardware"));
    expected.push_back(ObjectRelation::mergeWith("second-kind", "my-attribute"));
    expected.push_back(ObjectRelation::isTemplate("target-kind"));
    expected.push_back(ObjectRelation::templatized("by-which-kind", "my-attribute"));
    vector<ObjectRelation> res = j->kindRelations("identifier");
    BOOST_CHECK_EQUAL_COLLECTIONS(res.begin(), res.end(), expected.begin(), expected.end());
}
