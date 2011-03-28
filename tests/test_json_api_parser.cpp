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
#include <boost/test/floating_point_comparison.hpp>
#include "JsonApiTestFixture.h"
#include "deska/db/JsonApi.h"
#include "deska/db/JsonHandler.h"

using std::vector;
using std::map;
using namespace Deska::Db;

/** @short Fuzzy comparator for Deska::Value which can dela with floating point values

@see json_objectData
*/
struct FuzzyTestCompareDeskaValue: public boost::static_visitor<>
{
    template <typename T, typename U> void operator()(const T &, const U &) const
    {
        BOOST_ERROR("Cannot compare different types for equality");
    }

    template <typename T> void operator()(const T &a, const T &b) const
    {
        BOOST_CHECK_EQUAL(a, b);
    }

    void operator()(const double &a, const double &b) const
    {
        BOOST_CHECK_CLOSE(a, b, 0.01);
    }

    void operator()(const int &a, const double &b) const
    {
        BOOST_CHECK_CLOSE(static_cast<double>(a), b, 0.01);
    }

    void operator()(const double &a, const int &b) const
    {
        BOOST_CHECK_CLOSE(a, static_cast<double>(b), 0.01);
    }
};

/** @short Test that kindNames() can retrieve data */
BOOST_FIXTURE_TEST_CASE(json_kindNames, JsonApiTestFixture)
{
    expectWrite("{\"command\":\"getTopLevelObjectNames\"}");
    expectRead("{\"response\": \"getTopLevelObjectNames\", \"topLevelObjectKinds\": [\"z\", \"a\", \"b\", \"foo bar\"]}");
    vector<Identifier> expected;
    expected.push_back("z");
    expected.push_back("a");
    expected.push_back("b");
    expected.push_back("foo bar");
    vector<Identifier> res = j->kindNames();
    BOOST_CHECK_EQUAL_COLLECTIONS(res.begin(), res.end(), expected.begin(), expected.end());
    expectEmpty();
}

/** @short Test that kindAttributes() retrieves data */
BOOST_FIXTURE_TEST_CASE(json_kindAttributes, JsonApiTestFixture)
{
    expectWrite("{\"command\":\"getKindAttributes\",\"kindName\":\"some-object\"}");
    expectRead("{\"kindAttributes\": {\"bar\": \"int\", \"baz\": \"identifier\", \"foo\": \"string\", "
            "\"price\": \"double\"}, \"kindName\": \"some-object\", \"response\": \"getKindAttributes\"}");
    vector<KindAttributeDataType> expected;
    expected.push_back(KindAttributeDataType("bar", TYPE_INT));
    expected.push_back(KindAttributeDataType("baz", TYPE_IDENTIFIER));
    expected.push_back(KindAttributeDataType("foo", TYPE_STRING));
    expected.push_back(KindAttributeDataType("price", TYPE_DOUBLE));
    vector<KindAttributeDataType> res = j->kindAttributes("some-object");
    BOOST_CHECK_EQUAL_COLLECTIONS(res.begin(), res.end(), expected.begin(), expected.end());
    expectEmpty();
}

/** @short Test that kindAtttributes() can catch wrong referenced objects */
BOOST_FIXTURE_TEST_CASE(json_kindAttributes_wrong_object, JsonApiTestFixture)
{
    expectWrite("{\"command\":\"getKindAttributes\",\"kindName\":\"some-object\"}");
    expectRead("{\"kindAttributes\": {\"bar\": \"int\", \"baz\": \"identifier\", \"foo\": \"string\", "
            "\"price\": \"double\"}, \"kindName\": \"some-object-2\", \"response\": \"getKindAttributes\"}");
    BOOST_CHECK_THROW(j->kindAttributes("some-object"), JsonParseError);
    expectEmpty();
}

/** @short Test that kindRelations() can fetch data */
BOOST_FIXTURE_TEST_CASE(json_kindRelations, JsonApiTestFixture)
{
    expectWrite("{\"command\":\"getKindRelations\",\"kindName\":\"identifier\"}");
    expectRead("{\"kindName\": \"identifier\", \"kindRelations\": [[\"EMBED_INTO\", \"hardware\"], "
            "[\"MERGE_WITH\", \"second-kind\", \"my-attribute\"], [\"IS_TEMPLATE\", \"target-kind\"], "
            "[\"TEMPLATIZED\", \"by-which-kind\", \"my-attribute\"]], \"response\": \"getKindRelations\"}");
    vector<ObjectRelation> expected;
    expected.push_back(ObjectRelation::embedInto("hardware"));
    expected.push_back(ObjectRelation::mergeWith("second-kind", "my-attribute"));
    expected.push_back(ObjectRelation::isTemplate("target-kind"));
    expected.push_back(ObjectRelation::templatized("by-which-kind", "my-attribute"));
    vector<ObjectRelation> res = j->kindRelations("identifier");
    BOOST_CHECK_EQUAL_COLLECTIONS(res.begin(), res.end(), expected.begin(), expected.end());
    expectEmpty();
}

/** @short Test that kindInstances() returns a reasonable result */
BOOST_FIXTURE_TEST_CASE(json_kindInstances, JsonApiTestFixture)
{
    expectWrite("{\"command\":\"getKindInstances\",\"kindName\":\"blah\",\"revision\":\"r666\"}");
    expectRead("{\"kindName\": \"blah\", \"objectInstances\": [\"foo\", \"bar\", \"ahoj\"], \"response\": \"getKindInstances\", \"revision\": \"r666\"}");
    vector<Identifier> expected;
    expected.push_back("foo");
    expected.push_back("bar");
    expected.push_back("ahoj");
    vector<Identifier> res = j->kindInstances("blah", RevisionId(666));
    BOOST_CHECK_EQUAL_COLLECTIONS(res.begin(), res.end(), expected.begin(), expected.end());
    expectEmpty();
}

/** @short Test that kindInstances() fails when faced with wrong revision */
BOOST_FIXTURE_TEST_CASE(json_kindInstances_wrong_revision, JsonApiTestFixture)
{
    expectWrite("{\"command\":\"getKindInstances\",\"kindName\":\"blah\",\"revision\":\"r666\"}");
    expectRead("{\"kindName\": \"blah\", \"objectInstances\": [\"foo\", \"bar\", \"ahoj\"], \"response\": \"getKindInstances\", \"revision\": \"r333\"}");
    BOOST_CHECK_THROW(j->kindInstances("blah", RevisionId(666)), JsonParseError);
    expectEmpty();
}

/** @short Basic test for objectData() */
BOOST_FIXTURE_TEST_CASE(json_objectData, JsonApiTestFixture)
{
    expectWrite("{\"command\":\"getObjectData\",\"kindName\":\"kk\",\"objectName\":\"oo\",\"revision\":\"r3\"}");
    expectRead("{\"kindName\": \"kk\", \"objectData\": {\"foo\": \"bar\", \"int\": 10, \"real\": 100.666, \"price\": 666}, "
            "\"objectName\": \"oo\", \"response\": \"getObjectData\", \"revision\": \"r3\"}");
    map<Identifier,Value> expected;
    expected["foo"] = "bar";
    expected["int"] = 10;
    expected["real"] = 100.666;
    // Yes, check int-to-float comparison here
    expected["price"] = 666.0;
    map<Identifier,Value> res = j->objectData("kk", "oo", RevisionId(3));
    // This won't work on floats...
    //BOOST_CHECK(std::equal(res.begin(), res.end(), expected.begin()));
    // ...which is why we have to resort to implicit iteration here:
    BOOST_REQUIRE_EQUAL(res.size(), expected.size());
    map<Identifier,Value>::iterator i1 = expected.begin(), i2 = res.begin();
    while (i1 != expected.end()) {
        BOOST_REQUIRE_EQUAL(i1->first, i2->first);
        boost::apply_visitor(FuzzyTestCompareDeskaValue(), i1->second, i2->second);
        ++i1;
        ++i2;
    }
    expectEmpty();
}

/** @short Basic test for resolvedObjectData() */
BOOST_FIXTURE_TEST_CASE(json_resolvedObjectData, JsonApiTestFixture)
{
    expectWrite("{\"command\":\"getResolvedObjectData\",\"kindName\":\"kk\",\"objectName\":\"oo\",\"revision\":\"r0\"}");
    expectRead("{\"kindName\": \"kk\", \"objectName\": \"oo\", \"resolvedObjectData\": "
            "{\"foo\": [\"obj-defining-this\", \"bar\"], \"baz\": [\"this-obj\", \"666\"]}, \"response\": \"getResolvedObjectData\", \"revision\": \"r0\"}");
    map<Identifier, std::pair<Identifier,Value> > expected;
    expected["foo"] = std::make_pair("obj-defining-this", "bar");
    expected["baz"] = std::make_pair("this-obj", "666");
    map<Identifier, std::pair<Identifier,Value> > res = j->resolvedObjectData("kk", "oo");
    // In this case, we limit ourselves to string comparisons. There's a map invloved here, which means that
    // BOOST_CHECK_EQUAL_COLLECTIONS is worthless, and there isn't much point in duplicating the whole logic from json_objectData
    // at yet another place. Let's stick with strings and don't expect to see detailed error reporting here.
    BOOST_REQUIRE_EQUAL(res.size(), expected.size());
    BOOST_CHECK(std::equal(res.begin(), res.end(), expected.begin()));
    expectEmpty();
}

/** @short Basic test for findOverridenAttrs() */
BOOST_FIXTURE_TEST_CASE(json_findOverridenAttrs, JsonApiTestFixture)
{
    expectWrite("{\"command\":\"getObjectsOverridingAttribute\",\"kindName\":\"k\",\"objectName\":\"o\",\"attributeName\":\"aa\"}");
    expectRead("{\"attributeName\": \"aa\", \"kindName\": \"k\", "
            "\"objectInstances\": [\"z\", \"a\", \"aaa\"], \"objectName\": \"o\", \"response\": \"getObjectsOverridingAttribute\"}");
    vector<Identifier> expected;
    expected.push_back("z");
    expected.push_back("a");
    expected.push_back("aaa");
    vector<Identifier> res = j->findOverriddenAttrs("k", "o", "aa");
    BOOST_CHECK_EQUAL_COLLECTIONS(res.begin(), res.end(), expected.begin(), expected.end());
    expectEmpty();
}

/** @short Basic test for findNonOverridenAttrs() */
BOOST_FIXTURE_TEST_CASE(json_findNonOverridenAttrs, JsonApiTestFixture)
{
    expectWrite("{\"command\":\"getObjectsNotOverridingAttribute\",\"kindName\":\"k\",\"objectName\":\"o\",\"attributeName\":\"aa\"}");
    expectRead("{\"attributeName\": \"aa\", \"kindName\": \"k\", "
            "\"objectInstances\": [\"d\", \"e\", \"aaaaa\"], \"objectName\": \"o\", \"response\": \"getObjectsNotOverridingAttribute\"}");
    vector<Identifier> expected;
    expected.push_back("d");
    expected.push_back("e");
    expected.push_back("aaaaa");
    vector<Identifier> res = j->findNonOverriddenAttrs("k", "o", "aa");
    BOOST_CHECK_EQUAL_COLLECTIONS(res.begin(), res.end(), expected.begin(), expected.end());
    expectEmpty();
}

/** @short Basic test for createObject() */
BOOST_FIXTURE_TEST_CASE(json_createObject, JsonApiTestFixture)
{
    expectWrite("{\"command\":\"createObject\",\"kindName\":\"k\",\"objectName\":\"o\"}");
    expectRead("{\"kindName\": \"k\", \"objectName\": \"o\", \"response\": \"createObject\", \"result\": true}");
    j->createObject("k", "o");
    expectEmpty();
}

/** @short Basic test for deleteObject() */
BOOST_FIXTURE_TEST_CASE(json_deleteObject, JsonApiTestFixture)
{
    expectWrite("{\"command\":\"deleteObject\",\"kindName\":\"k\",\"objectName\":\"o\"}");
    expectRead("{\"kindName\": \"k\", \"objectName\": \"o\", \"response\": \"deleteObject\", \"result\": true}");
    j->deleteObject("k", "o");
    expectEmpty();
}

/** @short Basic test for renameObject() */
BOOST_FIXTURE_TEST_CASE(json_renameObject, JsonApiTestFixture)
{
    expectWrite("{\"command\":\"renameObject\",\"kindName\":\"kind\",\"objectName\":\"ooooold\",\"newObjectName\":\"new\"}");
    expectRead("{\"kindName\": \"kind\", \"newObjectName\": \"new\", \"objectName\": \"ooooold\", \"response\": \"renameObject\", \"result\": true}");
    j->renameObject("kind", "ooooold", "new");
    expectEmpty();
}

/** @short Basic test for removeAttribute() */
BOOST_FIXTURE_TEST_CASE(json_removeAttribute, JsonApiTestFixture)
{
    expectWrite("{\"command\":\"removeObjectAttribute\",\"kindName\":\"kind\",\"objectName\":\"obj\",\"attributeName\":\"fancyAttr\"}");
    expectRead("{\"attributeName\": \"fancyAttr\", \"kindName\": \"kind\", \"objectName\": \"obj\", \"response\": \"removeObjectAttribute\", \"result\": true}");
    j->removeAttribute("kind", "obj", "fancyAttr");
    expectEmpty();
}

/** @short A three-member tuple for holding JSON input/output and the corresponding Deska::Value */
struct SetAttrTestData {
    std::string jsonIn;
    std::string jsonOut;
    Value v;
    SetAttrTestData() {};
    SetAttrTestData(const std::string &ji, const std::string &jo, const Value &val): jsonIn(ji), jsonOut(jo), v(val) {}
};

/** @short Basic test for setAttribute() */
BOOST_FIXTURE_TEST_CASE(json_setAttribute, JsonApiTestFixture)
{
    // We want to test all of the supported data types, that's why we use a vector and some rather complicated boilerplate code here
    vector<SetAttrTestData> data;
    std::string jsonInputPrefix = "{\"command\":\"setObjectAttribute\",\"kindName\":\"k\",\"objectName\":\"o\",\"attributeName\":\"a\",\"attributeData\":";
    std::string jsonOutputSuffix = "{\"attributeName\": \"a\", \"kindName\": \"k\", \"objectName\": \"o\", "
            "\"response\": \"setObjectAttribute\", \"result\": true, \"attributeData\": ";
    data.push_back(SetAttrTestData(jsonInputPrefix + "\"some string\"}", jsonOutputSuffix + "\"some string\"}", "some string"));
    data.push_back(SetAttrTestData(jsonInputPrefix + "123}", jsonOutputSuffix + " 123}", 123));
    data.push_back(SetAttrTestData(jsonInputPrefix + "333.666}", jsonOutputSuffix + " 333.666}", 333.666));
    BOOST_FOREACH(const SetAttrTestData &value, data) {
        expectWrite(value.jsonIn);
        expectRead(value.jsonOut);
        j->setAttribute("k", "o", "a", value.v);
        expectEmpty();
    }
}

/** @short Basic test for startChangeset() */
BOOST_FIXTURE_TEST_CASE(json_startChangeset, JsonApiTestFixture)
{
    expectWrite("{\"command\":\"vcsStartChangeset\"}");
    expectRead("{\"response\": \"vcsStartChangeset\", \"revision\": \"tmp333\"}");
    BOOST_CHECK_EQUAL(j->startChangeset(), TemporaryChangesetId(333));
    expectEmpty();
}

/** @short Basic test for commitChangeset() */
BOOST_FIXTURE_TEST_CASE(json_commitChangeset, JsonApiTestFixture)
{
    expectWrite("{\"command\":\"vcsCommitChangeset\"}");
    expectRead("{\"response\": \"vcsCommitChangeset\", \"revision\": \"r666\"}");
    BOOST_CHECK_EQUAL(j->commitChangeset(), RevisionId(666));
    expectEmpty();
}

/** @short Basic test for reabseChangeset() */
BOOST_FIXTURE_TEST_CASE(json_rebaseChangeset, JsonApiTestFixture)
{
    expectWrite("{\"command\":\"vcsRebaseChangeset\",\"currentRevision\":\"r666\"}");
    expectRead("{\"response\": \"vcsRebaseChangeset\", \"currentRevision\": \"r666\", \"revision\": \"tmp333666\" }");
    BOOST_CHECK_EQUAL(j->rebaseChangeset(RevisionId(666)), TemporaryChangesetId(333666));
    expectEmpty();
}

/** @short Basic test for pendingChangesetsByMyself() */
BOOST_FIXTURE_TEST_CASE(json_pendingChangesetsByMyself, JsonApiTestFixture)
{
    expectWrite("{\"command\":\"vcsGetPendingChangesetsByMyself\"}");
    expectRead("{\"response\": \"vcsGetPendingChangesetsByMyself\", \"revisions\": [\"tmp1\", \"tmp2\", \"tmp3\"]}");
    vector<TemporaryChangesetId> expected;
    expected.push_back(TemporaryChangesetId(1));
    expected.push_back(TemporaryChangesetId(2));
    expected.push_back(TemporaryChangesetId(3));
    vector<TemporaryChangesetId> res = j->pendingChangesetsByMyself();
    BOOST_CHECK_EQUAL_COLLECTIONS(res.begin(), res.end(), expected.begin(), expected.end());
    expectEmpty();
}

/** @short Basic test for resumeChangeset() */
BOOST_FIXTURE_TEST_CASE(json_resumeChangeset, JsonApiTestFixture)
{
    expectWrite("{\"command\":\"vcsResumePendingChangeset\",\"revision\":\"tmp123\"}");
    expectRead("{\"response\": \"vcsResumePendingChangeset\", \"revision\": \"tmp123\"}");
    j->resumeChangeset(TemporaryChangesetId(123));
    expectEmpty();
}

/** @short Basic test for detachFromCurrentChangeset() */
BOOST_FIXTURE_TEST_CASE(json_detachFromCurrentChangeset, JsonApiTestFixture)
{
    expectWrite("{\"command\":\"vcsDetachFromCurrentChangeset\",\"message\":\"foobar\"}");
    expectRead("{\"response\": \"vcsDetachFromCurrentChangeset\",\"message\":\"foobar\"}");
    j->detachFromCurrentChangeset("foobar");
    expectEmpty();
}

/** @short Basic test for abortCurrentChangeset() */
BOOST_FIXTURE_TEST_CASE(json_abortCurrentChangeset, JsonApiTestFixture)
{
    expectWrite("{\"command\":\"vcsAbortCurrentChangeset\"}");
    expectRead("{\"response\": \"vcsAbortCurrentChangeset\"}");
    j->abortCurrentChangeset();
    expectEmpty();
}

/** @short Verify correctness of parsing of revisions from JSON */
BOOST_FIXTURE_TEST_CASE(json_revision_parsing_ok, JsonApiTestFixture)
{
    expectWrite("{\"command\":\"c\",\"r\":\"tmp123\"}");
    expectRead("{\"response\": \"c\", \"r\": \"tmp3\"}");
    TemporaryChangesetId r(123);
    JsonHandler h(j, "c");
    h.write("r", r).extract(&r).valueShouldMatch = false;
    h.work();
    BOOST_CHECK_EQUAL(r, TemporaryChangesetId(3));
    expectEmpty();
}

/** @short Make sure we scream loudly when faced with invalid JSON data */
BOOST_FIXTURE_TEST_CASE(json_malformed_json, JsonApiTestFixture)
{
    std::vector<std::string> data;
    data.push_back("");
    data.push_back("{");
    data.push_back("{\"command\":");
    data.push_back("{\"command\":\"c");
    data.push_back("{\"command\":\"c\"");
    data.push_back("{\"command\":\"c\",");
    BOOST_FOREACH(const std::string &line, data) {
        expectWrite("{\"command\":\"vcsAbortCurrentChangeset\"}");
        expectRead(line);
        // FIXME: distinguish between "JSON parsing error" and "data error in a well-formed JSON"
        BOOST_CHECK_THROW(j->abortCurrentChangeset(), JsonParseError);
        expectEmpty();
    }
}

/** @short Verify that parsing of TemporaryChangesetId from JSON is satisfied exclusively by valid TemporaryChangesetId representation */
BOOST_FIXTURE_TEST_CASE(json_revision_parsing_kind_mismatch, JsonApiTestFixture)
{
    std::vector<std::string> offendingItems;
    offendingItems.push_back("r3");
    offendingItems.push_back("r");
    offendingItems.push_back("tm");
    offendingItems.push_back("tmp");
    offendingItems.push_back("tmpabc");
    offendingItems.push_back("foo");
    offendingItems.push_back("666");
    offendingItems.push_back("");

    BOOST_FOREACH(const std::string &s, offendingItems) {
        expectWrite("{\"command\":\"c\",\"r\":\"tmp123\"}");
        expectRead("{\"response\": \"c\", \"r\": \"" + s + "\"}");

        // Got to duplicate the behavior of how a JsonApiParser would use the JsonHandler
        TemporaryChangesetId r(123);
        JsonHandler h(j, "c");
        // Make sure to explicitly request extraction, but also don't request comparison of JSON serialization
        h.write("r", r).extract(&r).valueShouldMatch = false;
        // ...if we didn't use the valueShouldMatch here, the error would get caught much earlier, even before
        // the actual extraction happens.
        try {
            h.work();
            BOOST_ERROR(std::string("Passing '" + s + "' should have thrown an exception."));
        } catch (std::domain_error &e) {
            // this is actually what we want
            //std::cerr << "OK: " << e.what() << std::endl;
        }
        expectEmpty();
    }
}
