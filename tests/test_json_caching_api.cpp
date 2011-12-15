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
#define BOOST_TEST_MODULE json_caching_api
#include <boost/test/unit_test.hpp>
#include "JsonApiTestFixture.h"
#include "deska/db/CachingJsonApi.h"

using std::vector;
using std::map;
using namespace Deska::Db;

#define COMMON_INIT \
    /* At first, it has to find out what the top-level object types are */ \
    expectWrite("{\"command\":\"kindNames\",\"tag\":\"T\"}\n"); \
    expectRead("{\"response\": \"kindNames\", \"tag\":\"T\", \"kindNames\": [\"a\", \"b\"]}\n"); \
    /* The, for each of them, it asks for a list of attributes, and then for a list of relations. */ \
    /* Start with "a": */ \
    expectWrite("{\"command\":\"kindAttributes\",\"tag\":\"T\",\"kindName\":\"a\"}\n"); \
    expectRead("{\"kindAttributes\": {\"bar\": \"int\", \"baz\": \"identifier\", \"foo\": \"string\", " \
            "\"price\": \"double\"}, \"tag\":\"T\", \"response\": \"kindAttributes\"}\n"); \
    expectWrite("{\"command\":\"kindRelations\",\"tag\":\"T\",\"kindName\":\"a\"}\n"); \
    expectRead("{\"kindRelations\": [], \"response\": \"kindRelations\", \"tag\":\"T\"}\n"); \
    /* ...and move to "b": */ \
    expectWrite("{\"command\":\"kindAttributes\",\"tag\":\"T\",\"kindName\":\"b\"}\n"); \
    expectRead("{\"kindAttributes\": {\"name\": \"string\", \"name_of_a\": \"identifier\"}, \"tag\":\"T\", \"response\": \"kindAttributes\"}\n"); \
    expectWrite("{\"command\":\"kindRelations\",\"tag\":\"T\",\"kindName\":\"b\"}\n"); \
    expectRead("{\"kindRelations\": [{\"relation\":\"TEMPLATIZED\", \"target\":\"a\", \"column\": \"name_of_a\"}], \"tag\":\"T\", \"response\": \"kindRelations\"}\n"); \
    \
    /* This is ugly, but in order to reuse the JsonApiTestFixture, we'll have to hack around this: */ \
    delete j; \
    j = new Deska::Db::CachingJsonApi(); \
    bindStreams();

/** @short Make sure that CachingJsonApi's caching really works

This tests tries to verify that when we call any metadata-querying method of the API, the caching layer
will rmember the result and use it for further inquiries.
*/
BOOST_FIXTURE_TEST_CASE(json_kindNames, JsonApiTestFixtureFailOnStreamThrow)
{
    COMMON_INIT

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

BOOST_FIXTURE_TEST_CASE(json_caching_modifications_same_kind, JsonApiTestFixtureFailOnStreamThrow)
{
    COMMON_INIT
    j->kindNames();

    j->setCommandBatching(Deska::Db::CACHE_SAME_KIND);
    // The first two commands shall be batched together
    j->setAttribute("a", "o1", "bar", Deska::Db::Value(10));
    j->setAttribute("a", "o2", "price", Deska::Db::Value(0.0));
    expectWrite("{\"command\":\"applyBatchedChanges\",\"tag\":\"T\",\"modifications\":["
                "{\"command\":\"setAttribute\",\"kindName\":\"a\",\"objectName\":\"o1\",\"attributeName\":\"bar\",\"attributeData\":10},"
                "{\"command\":\"setAttribute\",\"kindName\":\"a\",\"objectName\":\"o2\",\"attributeName\":\"price\",\"attributeData\":0.0}"
                "]}\n");
    expectRead("{\"response\":\"applyBatchedChanges\", \"tag\":\"T\"}\n");
    // But this one is for a different kind, and therefore will not be batched with them
    j->setAttribute("b", "o3", "name", Deska::Db::Value(std::string("666")));
    // It will remain cached until we flush the cache explicitly. This command will not send the modification further:
    expectWrite("{\"command\":\"listRevisions\",\"tag\":\"T\"}\n");
    expectRead("{\"response\": \"listRevisions\", \"listRevisions\": [], \"tag\":\"T\"}\n");
    j->listRevisions();
    // Only an explicit cache flush will send the cached modification for o3.name, along with the rest of the command for this kind:
    BOOST_CHECK_EQUAL(j->createObject("b", "o4"), std::string("o4"));
    j->deleteObject("b", "o5");
    j->renameObject("b", "o4", "o6");
    expectWrite("{\"command\":\"applyBatchedChanges\",\"tag\":\"T\",\"modifications\":["
                "{\"command\":\"setAttribute\",\"kindName\":\"b\",\"objectName\":\"o3\",\"attributeName\":\"name\",\"attributeData\":\"666\"},"
                "{\"command\":\"createObject\",\"kindName\":\"b\",\"objectName\":\"o4\"},"
                "{\"command\":\"deleteObject\",\"kindName\":\"b\",\"objectName\":\"o5\"},"
                "{\"command\":\"renameObject\",\"kindName\":\"b\",\"oldObjectName\":\"o4\",\"newObjectName\":\"o6\"}"
                "]}\n");
    expectRead("{\"response\":\"applyBatchedChanges\", \"tag\":\"T\"}\n");
    j->setCommandBatching(Deska::Db::SEND_IMMEDIATELY);

    // Enable caching again and check that createObject with an empty name is a special case and flushes the cache
    j->setCommandBatching(Deska::Db::CACHE_SAME_KIND);
    // File the cache with junk data
    j->setAttribute("a", "o1", "bar", Deska::Db::Value(10));
    // Now the createObject() will automaitcally flush the cache and add a standalone command
    expectWrite("{\"command\":\"applyBatchedChanges\",\"tag\":\"T\",\"modifications\":["
                "{\"command\":\"setAttribute\",\"kindName\":\"a\",\"objectName\":\"o1\",\"attributeName\":\"bar\",\"attributeData\":10}"
                "]}\n");
    expectRead("{\"response\":\"applyBatchedChanges\", \"tag\":\"T\"}\n");
    expectWrite("{\"command\":\"createObject\",\"tag\":\"T\",\"kindName\":\"a\",\"objectName\":\"\"}\n");
    expectRead("{\"response\":\"createObject\",\"tag\":\"T\",\"createObject\":\"o7\"}\n");
    BOOST_CHECK_EQUAL(j->createObject("a", ""), std::string("o7"));

    // There are no pending commands, so this shall be a NOP
    j->setCommandBatching(Deska::Db::SEND_IMMEDIATELY);

    // The JsonApiTestFixture's destructor will free j.
}

BOOST_FIXTURE_TEST_CASE(json_caching_modifications_same_object, JsonApiTestFixtureFailOnStreamThrow)
{
    COMMON_INIT
    j->kindNames();

    j->setCommandBatching(Deska::Db::CACHE_SAME_OBJECT);

    // These two commands will not be combined together
    j->setAttribute("a", "o1", "bar", Deska::Db::Value(10));
    expectWrite("{\"command\":\"applyBatchedChanges\",\"tag\":\"T\",\"modifications\":["
                "{\"command\":\"setAttribute\",\"kindName\":\"a\",\"objectName\":\"o1\",\"attributeName\":\"bar\",\"attributeData\":10}"
                "]}\n");
    expectRead("{\"response\":\"applyBatchedChanges\", \"tag\":\"T\"}\n");
    j->setAttribute("a", "o2", "price", Deska::Db::Value(0.0));

    // Accessing a different kind shall indeed flush the command
    expectWrite("{\"command\":\"applyBatchedChanges\",\"tag\":\"T\",\"modifications\":["
                "{\"command\":\"setAttribute\",\"kindName\":\"a\",\"objectName\":\"o2\",\"attributeName\":\"price\",\"attributeData\":0.0}"
                "]}\n");
    expectRead("{\"response\":\"applyBatchedChanges\", \"tag\":\"T\"}\n");
    j->setAttribute("b", "o3", "name", Deska::Db::Value(std::string("666")));

    // Accessing functions which do not modify the DB does not flush the cache
    expectWrite("{\"command\":\"listRevisions\",\"tag\":\"T\"}\n");
    expectRead("{\"response\": \"listRevisions\", \"listRevisions\": [], \"tag\":\"T\"}\n");
    j->listRevisions();

    // Modifying the same object will not flush the cache
    j->setAttribute("b", "o3", "pwn", Deska::Db::Value(std::string("333")));
    j->renameObject("b", "o3", "o4");
    j->setAttribute("b", "o4", "xyz", Deska::Db::Value(std::string("abc")));
    j->deleteObject("b", "o4");
    expectWrite("{\"command\":\"applyBatchedChanges\",\"tag\":\"T\",\"modifications\":["
                // this one is from before the listRevisions
                "{\"command\":\"setAttribute\",\"kindName\":\"b\",\"objectName\":\"o3\",\"attributeName\":\"name\",\"attributeData\":\"666\"},"
                // these four are from the code block just above
                "{\"command\":\"setAttribute\",\"kindName\":\"b\",\"objectName\":\"o3\",\"attributeName\":\"pwn\",\"attributeData\":\"333\"},"
                "{\"command\":\"renameObject\",\"kindName\":\"b\",\"oldObjectName\":\"o3\",\"newObjectName\":\"o4\"},"
                "{\"command\":\"setAttribute\",\"kindName\":\"b\",\"objectName\":\"o4\",\"attributeName\":\"xyz\",\"attributeData\":\"abc\"},"
                "{\"command\":\"deleteObject\",\"kindName\":\"b\",\"objectName\":\"o4\"}"
                "]}\n");
    expectRead("{\"response\":\"applyBatchedChanges\", \"tag\":\"T\"}\n");
    // On the other hand, creating an object (no matter what its name is) shall flush the cache
    BOOST_CHECK_EQUAL(j->createObject("b", "o4"), std::string("o4"));
    // Flush the cache explicitly now
    expectWrite("{\"command\":\"applyBatchedChanges\",\"tag\":\"T\",\"modifications\":["
                "{\"command\":\"createObject\",\"kindName\":\"b\",\"objectName\":\"o4\"}"
                "]}\n");
    expectRead("{\"response\":\"applyBatchedChanges\", \"tag\":\"T\"}\n");
    j->setCommandBatching(Deska::Db::SEND_IMMEDIATELY);

    // There are no pending commands, so this shall be a NOP
    j->setCommandBatching(Deska::Db::CACHE_SAME_OBJECT);
    j->setCommandBatching(Deska::Db::SEND_IMMEDIATELY);

    // The JsonApiTestFixture's destructor will free j.
}
