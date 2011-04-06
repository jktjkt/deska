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
BOOST_FIXTURE_TEST_CASE(json_kindNames, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"kindNames\"}\n");
    expectRead("{\"response\": \"kindNames\", \"kindNames\": [\"z\", \"a\", \"b\", \"foo bar\"]}\n");
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
BOOST_FIXTURE_TEST_CASE(json_kindAttributes, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"kindAttributes\",\"kindName\":\"some-object\"}\n");
    expectRead("{\"kindAttributes\": {\"bar\": \"int\", \"baz\": \"identifier\", \"foo\": \"string\", \n"
            "\"price\": \"double\"}, \"kindName\": \"some-object\", \"response\": \"kindAttributes\"}\n");
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
BOOST_FIXTURE_TEST_CASE(json_kindAttributes_wrong_object, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"kindAttributes\",\"kindName\":\"some-object\"}\n");
    expectRead("{\"kindAttributes\": {\"bar\": \"int\", \"baz\": \"identifier\", \"foo\": \"string\", "
            "\"price\": \"double\"}, \"kindName\": \"some-object-2\", \"response\": \"kindAttributes\"}\n");
    BOOST_CHECK_THROW(j->kindAttributes("some-object"), JsonParseError);
    expectEmpty();
}

/** @short Test that kindRelations() can fetch data */
BOOST_FIXTURE_TEST_CASE(json_kindRelations, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"kindRelations\",\"kindName\":\"identifier\"}\n");
    expectRead("{\"kindName\": \"identifier\", \"kindRelations\": [[\"EMBED_INTO\", \"hardware\"], "
            "[\"MERGE_WITH\", \"second-kind\", \"my-attribute\"], [\"IS_TEMPLATE\", \"target-kind\"], "
            "[\"TEMPLATIZED\", \"by-which-kind\", \"my-attribute\"]], \"response\": \"kindRelations\"}\n");
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
BOOST_FIXTURE_TEST_CASE(json_kindInstances, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"kindInstances\",\"kindName\":\"blah\",\"revision\":\"r666\"}\n");
    expectRead("{\"kindName\": \"blah\", \"kindInstances\": [\"foo\", \"bar\", \"ahoj\"], \"response\": \"kindInstances\", \"revision\": \"r666\"}\n");
    vector<Identifier> expected;
    expected.push_back("foo");
    expected.push_back("bar");
    expected.push_back("ahoj");
    vector<Identifier> res = j->kindInstances("blah", RevisionId(666));
    BOOST_CHECK_EQUAL_COLLECTIONS(res.begin(), res.end(), expected.begin(), expected.end());
    expectEmpty();
}

/** @short Test that kindInstances() fails when faced with wrong revision */
BOOST_FIXTURE_TEST_CASE(json_kindInstances_wrong_revision, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"kindInstances\",\"kindName\":\"blah\",\"revision\":\"r666\"}\n");
    expectRead("{\"kindName\": \"blah\", \"kindInstances\": [\"foo\", \"bar\", \"ahoj\"], \"response\": \"kindInstances\", \"revision\": \"r333\"}\n");
    BOOST_CHECK_THROW(j->kindInstances("blah", RevisionId(666)), JsonParseError);
    expectEmpty();
}

/** @short Basic test for objectData() */
BOOST_FIXTURE_TEST_CASE(json_objectData, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"objectData\",\"kindName\":\"kk\",\"objectName\":\"oo\",\"revision\":\"r3\"}\n");
    expectRead("{\"kindName\": \"kk\", \"objectData\": {\"foo\": \"bar\", \"int\": 10, \"real\": 100.666, \"price\": 666}, "
            "\"objectName\": \"oo\", \"response\": \"objectData\", \"revision\": \"r3\"}\n");
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
BOOST_FIXTURE_TEST_CASE(json_resolvedObjectData, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"resolvedObjectData\",\"kindName\":\"kk\",\"objectName\":\"oo\",\"revision\":\"r0\"}\n");
    expectRead("{\"kindName\": \"kk\", \"objectName\": \"oo\", \"resolvedObjectData\": "
            "{\"foo\": [\"obj-defining-this\", \"bar\"], \"baz\": [\"this-obj\", \"666\"]}, \"response\": \"resolvedObjectData\", \"revision\": \"r0\"}\n");
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
BOOST_FIXTURE_TEST_CASE(json_findOverridenAttrs, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"findOverriddenAttrs\",\"kindName\":\"k\",\"objectName\":\"o\",\"attributeName\":\"aa\"}\n");
    expectRead("{\"attributeName\": \"aa\", \"kindName\": \"k\", "
            "\"objectInstances\": [\"z\", \"a\", \"aaa\"], \"objectName\": \"o\", \"response\": \"findOverriddenAttrs\"}\n");
    vector<Identifier> expected;
    expected.push_back("z");
    expected.push_back("a");
    expected.push_back("aaa");
    vector<Identifier> res = j->findOverriddenAttrs("k", "o", "aa");
    BOOST_CHECK_EQUAL_COLLECTIONS(res.begin(), res.end(), expected.begin(), expected.end());
    expectEmpty();
}

/** @short Basic test for findNonOverridenAttrs() */
BOOST_FIXTURE_TEST_CASE(json_findNonOverridenAttrs, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"findNonOverriddenAttrs\",\"kindName\":\"k\",\"objectName\":\"o\",\"attributeName\":\"aa\"}\n");
    expectRead("{\"attributeName\": \"aa\", \"kindName\": \"k\", "
            "\"objectInstances\": [\"d\", \"e\", \"aaaaa\"], \"objectName\": \"o\", \"response\": \"findNonOverriddenAttrs\"}\n");
    vector<Identifier> expected;
    expected.push_back("d");
    expected.push_back("e");
    expected.push_back("aaaaa");
    vector<Identifier> res = j->findNonOverriddenAttrs("k", "o", "aa");
    BOOST_CHECK_EQUAL_COLLECTIONS(res.begin(), res.end(), expected.begin(), expected.end());
    expectEmpty();
}

/** @short Basic test for createObject() */
BOOST_FIXTURE_TEST_CASE(json_createObject, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"createObject\",\"kindName\":\"k\",\"objectName\":\"o\"}\n");
    expectRead("{\"kindName\": \"k\", \"objectName\": \"o\", \"response\": \"createObject\", \"result\": true}\n");
    j->createObject("k", "o");
    expectEmpty();
}

/** @short Basic test for deleteObject() */
BOOST_FIXTURE_TEST_CASE(json_deleteObject, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"deleteObject\",\"kindName\":\"k\",\"objectName\":\"o\"}\n");
    expectRead("{\"kindName\": \"k\", \"objectName\": \"o\", \"response\": \"deleteObject\", \"result\": true}\n");
    j->deleteObject("k", "o");
    expectEmpty();
}

/** @short Basic test for renameObject() */
BOOST_FIXTURE_TEST_CASE(json_renameObject, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"renameObject\",\"kindName\":\"kind\",\"objectName\":\"ooooold\",\"newObjectName\":\"new\"}\n");
    expectRead("{\"kindName\": \"kind\", \"newObjectName\": \"new\", \"objectName\": \"ooooold\", \"response\": \"renameObject\", \"result\": true}\n");
    j->renameObject("kind", "ooooold", "new");
    expectEmpty();
}

/** @short Basic test for removeAttribute() */
BOOST_FIXTURE_TEST_CASE(json_removeAttribute, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"removeAttribute\",\"kindName\":\"kind\",\"objectName\":\"obj\",\"attributeName\":\"fancyAttr\"}\n");
    expectRead("{\"attributeName\": \"fancyAttr\", \"kindName\": \"kind\", \"objectName\": \"obj\", \"response\": \"removeAttribute\", \"result\": true}\n");
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
BOOST_FIXTURE_TEST_CASE(json_setAttribute, JsonApiTestFixtureFailOnStreamThrow)
{
    // We want to test all of the supported data types, that's why we use a vector and some rather complicated boilerplate code here
    vector<SetAttrTestData> data;
    std::string jsonInputPrefix = "{\"command\":\"setAttribute\",\"kindName\":\"k\",\"objectName\":\"o\",\"attributeName\":\"a\",\"attributeData\":";
    std::string jsonOutputSuffix = "{\"attributeName\": \"a\", \"kindName\": \"k\", \"objectName\": \"o\", "
            "\"response\": \"setAttribute\", \"result\": true, \"attributeData\": ";
    data.push_back(SetAttrTestData(jsonInputPrefix + "\"some string\"}\n", jsonOutputSuffix + "\"some string\"}\n", "some string"));
    data.push_back(SetAttrTestData(jsonInputPrefix + "123}\n", jsonOutputSuffix + " 123}\n", 123));
    data.push_back(SetAttrTestData(jsonInputPrefix + "333.666}\n", jsonOutputSuffix + " 333.666}\n", 333.666));
    BOOST_FOREACH(const SetAttrTestData &value, data) {
        expectWrite(value.jsonIn);
        expectRead(value.jsonOut);
        j->setAttribute("k", "o", "a", value.v);
        expectEmpty();
    }
}

/** @short Basic test for startChangeset() */
BOOST_FIXTURE_TEST_CASE(json_startChangeset, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"startChangeset\"}\n");
    expectRead("{\"response\": \"startChangeset\", \"revision\": \"tmp333\"}\n");
    BOOST_CHECK_EQUAL(j->startChangeset(), TemporaryChangesetId(333));
    expectEmpty();
}

/** @short Basic test for commitChangeset() */
BOOST_FIXTURE_TEST_CASE(json_commitChangeset, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"commitChangeset\",\"commitMessage\":\"description\"}\n");
    expectRead("{\"response\": \"commitChangeset\", \"revision\": \"r666\", \"commitMessage\":\"description\"}\n");
    BOOST_CHECK_EQUAL(j->commitChangeset("description"), RevisionId(666));
    expectEmpty();
}

/** @short Basic test for reabseChangeset() */
BOOST_FIXTURE_TEST_CASE(json_rebaseChangeset, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"rebaseChangeset\",\"currentRevision\":\"r666\"}\n");
    expectRead("{\"response\": \"rebaseChangeset\", \"currentRevision\": \"r666\", \"revision\": \"tmp333666\" }\n");
    BOOST_CHECK_EQUAL(j->rebaseChangeset(RevisionId(666)), TemporaryChangesetId(333666));
    expectEmpty();
}

/** @short Basic test for pendingChangesetsByMyself() */
BOOST_FIXTURE_TEST_CASE(json_pendingChangesetsByMyself, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"pendingChangesetsByMyself\"}\n");
    expectRead("{\"response\": \"pendingChangesetsByMyself\", \"pendingChangesetsByMyself\": [\"tmp1\", \"tmp2\", \"tmp3\"]}\n");
    vector<TemporaryChangesetId> expected;
    expected.push_back(TemporaryChangesetId(1));
    expected.push_back(TemporaryChangesetId(2));
    expected.push_back(TemporaryChangesetId(3));
    vector<TemporaryChangesetId> res = j->pendingChangesetsByMyself();
    BOOST_CHECK_EQUAL_COLLECTIONS(res.begin(), res.end(), expected.begin(), expected.end());
    expectEmpty();
}

/** @short Basic test for resumeChangeset() */
BOOST_FIXTURE_TEST_CASE(json_resumeChangeset, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"resumeChangeset\",\"revision\":\"tmp123\"}\n");
    expectRead("{\"response\": \"resumeChangeset\", \"revision\": \"tmp123\"}\n");
    j->resumeChangeset(TemporaryChangesetId(123));
    expectEmpty();
}

/** @short Basic test for detachFromCurrentChangeset() */
BOOST_FIXTURE_TEST_CASE(json_detachFromCurrentChangeset, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"detachFromCurrentChangeset\",\"message\":\"foobar\"}\n");
    expectRead("{\"response\": \"detachFromCurrentChangeset\",\"message\":\"foobar\"}\n");
    j->detachFromCurrentChangeset("foobar");
    expectEmpty();
}

/** @short Basic test for abortCurrentChangeset() */
BOOST_FIXTURE_TEST_CASE(json_abortCurrentChangeset, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"abortCurrentChangeset\"}\n");
    expectRead("{\"response\": \"abortCurrentChangeset\"}\n");
    j->abortCurrentChangeset();
    expectEmpty();
}

/** @short Verify correctness of parsing of revisions from JSON */
BOOST_FIXTURE_TEST_CASE(json_revision_parsing_ok, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"c\",\"r\":\"tmp123\"}\n");
    expectRead("{\"response\": \"c\", \"r\": \"tmp3\"}\n");
    TemporaryChangesetId r(123);
    JsonHandler h(j, "c");
    h.write("r", r).extract(&r).valueShouldMatch = false;
    h.work();
    BOOST_CHECK_EQUAL(r, TemporaryChangesetId(3));
    expectEmpty();
}


/** @short Data-driven testing for JSON parsing errors

boost::test doesn't really support data-driven testing with re-initialization of the fixture afetr each test case
but that reinitialization is exactly what we need here, as we do want to check the state of the streams.
That's why we create a custom fixture here.
*/
struct MalformedJsonFixture: public JsonApiTestFixtureFailOnStreamThrow {
    std::string line;
    ~MalformedJsonFixture()
    {
        expectWrite("{\"command\":\"abortCurrentChangeset\"}\n");
        if (!line.empty())
            expectRead(line);
        mockBuffer.expectReadEof();
        // FIXME: distinguish between "JSON parsing error" and "data error in a well-formed JSON"
        BOOST_CHECK_THROW(j->abortCurrentChangeset(), JsonParseError);
        BOOST_CHECK(readStream.eof());
        BOOST_CHECK(!readStream.bad());
        BOOST_CHECK(writeStream.good());
        expectEmpty();
    }
};

/** @short Make sure we scream loudly when faced with invalid JSON data */
BOOST_FIXTURE_TEST_CASE(json_malformed_json_empty, MalformedJsonFixture)
{
    line = "";
}

BOOST_FIXTURE_TEST_CASE(json_malformed_json_open_bace, MalformedJsonFixture)
{
    line = "{";
}

BOOST_FIXTURE_TEST_CASE(json_malformed_json_just_key, MalformedJsonFixture)
{
    line = "{\"command\":";
}
BOOST_FIXTURE_TEST_CASE(json_malformed_json_unterminated_value, MalformedJsonFixture)
{
    line = "{\"command\":\"c";
}

BOOST_FIXTURE_TEST_CASE(json_malformed_json_missing_close_brace, MalformedJsonFixture)
{
    line = "{\"command\":\"c\"";
}

BOOST_FIXTURE_TEST_CASE(json_malformed_json_missing_next_command, MalformedJsonFixture)
{
    line = "{\"command\":\"c\",";
}

/** @short Verify that parsing of TemporaryChangesetId from JSON is satisfied exclusively by valid TemporaryChangesetId representation */
BOOST_FIXTURE_TEST_CASE(json_revision_parsing_kind_mismatch, JsonApiTestFixtureFailOnStreamThrow)
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
        expectWrite("{\"command\":\"c\",\"r\":\"tmp123\"}\n");
        expectRead("{\"response\": \"c\", \"r\": \"" + s + "\"}\n");

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
