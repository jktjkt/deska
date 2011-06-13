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
    expectWrite("{\"command\":\"kindNames\",\"tag\":\"T\"}\n");
    expectRead("{\"response\": \"kindNames\",\"tag\":\"T\", \"kindNames\": [\"z\", \"a\", \"b\", \"foo bar\"]}\n");
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
    expectWrite("{\"command\":\"kindAttributes\",\"tag\":\"T\",\"kindName\":\"some-object\"}\n");
    expectRead("{\"kindAttributes\": {\"bar\": \"int\", \"baz\": \"identifier\", \"foo\": \"string\", \n"
            "\"price\": \"double\"}, \"tag\": \"T\", \"response\": \"kindAttributes\"}\n");
    vector<KindAttributeDataType> expected;
    expected.push_back(KindAttributeDataType("bar", TYPE_INT));
    expected.push_back(KindAttributeDataType("baz", TYPE_IDENTIFIER));
    expected.push_back(KindAttributeDataType("foo", TYPE_STRING));
    expected.push_back(KindAttributeDataType("price", TYPE_DOUBLE));
    vector<KindAttributeDataType> res = j->kindAttributes("some-object");
    BOOST_CHECK_EQUAL_COLLECTIONS(res.begin(), res.end(), expected.begin(), expected.end());
    expectEmpty();
}

/** @short Test that kindAtttributes() screams loudly when seeing a different response */
BOOST_FIXTURE_TEST_CASE(json_kindAttributes_wrong_response, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"kindAttributes\",\"tag\":\"T\",\"kindName\":\"some-object\"}\n");
    expectRead("{\"kindAttributes\": {\"bar\": \"int\", \"baz\": \"identifier\", \"foo\": \"string\", "
            "\"price\": \"double\"}, \"response\": \"kindAttributesBlah\",\"tag\":\"T\"}\n");
    BOOST_CHECK_THROW(j->kindAttributes("some-object"), JsonStructureError);
    expectEmpty();
}

/** @short Test that kindAtttributes() screams loudly when seeing a different tag */
BOOST_FIXTURE_TEST_CASE(json_kindAttributes_wrong_tag, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"kindAttributes\",\"tag\":\"T\",\"kindName\":\"some-object\"}\n");
    expectRead("{\"kindAttributes\": {\"bar\": \"int\", \"baz\": \"identifier\", \"foo\": \"string\", "
            "\"price\": \"double\"}, \"response\": \"kindAttributes\",\"tag\":\"T2\"}\n");
    BOOST_CHECK_THROW(j->kindAttributes("some-object"), JsonStructureError);
    expectEmpty();
}

/** @short Test that kindRelations() can fetch data */
BOOST_FIXTURE_TEST_CASE(json_kindRelations, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"kindRelations\",\"tag\":\"T\",\"kindName\":\"identifier\"}\n");
    expectRead("{\"kindRelations\": ["
            "{\"relation\": \"EMBED_INTO\", \"target\": \"hardware\"}, "
            "{\"relation\": \"MERGE_WITH\", \"target\": \"second-kind\"}, "
            "{\"relation\": \"IS_TEMPLATE\", \"target\": \"target-kind\"}, "
            "{\"relation\": \"TEMPLATIZED\", \"target\": \"by-which-kind\"}], \"response\": \"kindRelations\",\"tag\":\"T\"}\n");
    vector<ObjectRelation> expected;
    expected.push_back(ObjectRelation::embedInto("hardware"));
    expected.push_back(ObjectRelation::mergeWith("second-kind"));
    expected.push_back(ObjectRelation::isTemplate("target-kind"));
    expected.push_back(ObjectRelation::templatized("by-which-kind"));
    vector<ObjectRelation> res = j->kindRelations("identifier");
    BOOST_CHECK_EQUAL_COLLECTIONS(res.begin(), res.end(), expected.begin(), expected.end());
    expectEmpty();
}

/** @short Test how kindRelations() handles errors */
BOOST_FIXTURE_TEST_CASE(json_kindRelations_errors, JsonApiTestFixtureFailOnStreamThrow)
{
    using std::string;
    using std::make_pair;
    // We want to test various different error scenarios here
    typedef std::pair<string,string> PairStringString;
    vector<PairStringString> data;
    data.push_back(std::make_pair<string,string>("{\"rel\": \"EMBED_INTO\"}", "JSON field 'rel' is not allowed in this context (expecting one of: relation target)."));
    data.push_back(std::make_pair<string,string>("{\"relation\": \"EMBED_INTO2\", \"target\": \"pwn\"}", "Invalid relation kind 'EMBED_INTO2'"));
    data.push_back(std::make_pair<string,string>("{\"relation\": \"EMBED_INTO\"}", "Mandatory field 'target' not present in the response"));
    data.push_back(std::make_pair<string,string>("{\"relation\": \"EMBED_INTO\", \"target\": \"hardware\", \"foo\": \"bar\"}",
                                                 "JSON field 'foo' is not allowed in this context (expecting one of: relation target)."));
    BOOST_FOREACH(const PairStringString &value, data) {
        expectWrite("{\"command\":\"kindRelations\",\"tag\":\"T\",\"kindName\":\"identifier\"}\n");
        expectRead("{\"kindRelations\": [" + value.first + "],\"tag\":\"T\", \"response\": \"kindRelations\"}\n");
        try {
            j->kindRelations("identifier");
            BOOST_ERROR(string("Passing '" + value.first + "' should have thrown an exception."));
        } catch (JsonStructureError &e) {
            BOOST_CHECK_EQUAL(value.second, e.what());
        }
        expectEmpty();
    }
}

/** @short Test that kindInstances() returns a reasonable result */
BOOST_FIXTURE_TEST_CASE(json_kindInstances, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"kindInstances\",\"tag\":\"T\",\"kindName\":\"blah\",\"revision\":\"r666\"}\n");
    expectRead("{\"kindInstances\": [\"foo\", \"bar\", \"ahoj\"], \"tag\":\"T\", \"response\": \"kindInstances\"}\n");
    vector<Identifier> expected;
    expected.push_back("foo");
    expected.push_back("bar");
    expected.push_back("ahoj");
    vector<Identifier> res = j->kindInstances("blah", boost::optional<Filter>(), RevisionId(666));
    BOOST_CHECK_EQUAL_COLLECTIONS(res.begin(), res.end(), expected.begin(), expected.end());
    expectEmpty();
}

/** @short Test that simple filter for IS NULL works fine */
BOOST_FIXTURE_TEST_CASE(json_kindInstances_filterEq, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"kindInstances\",\"tag\":\"T\",\"kindName\":\"blah\",\"revision\":\"r666\",\"filter\":{\"condition\":\"columnEq\",\"kind\":\"kind\",\"attribute\":\"attr\",\"value\":null}}\n");
    expectRead("{\"kindInstances\": [], \"response\": \"kindInstances\",\"tag\":\"T\"}\n");
    vector<Identifier> expected;
    vector<Identifier> res = j->kindInstances("blah", Filter(AttributeExpression(FILTER_COLUMN_EQ, "kind", "attr", Value())), RevisionId(666));
    BOOST_CHECK_EQUAL_COLLECTIONS(res.begin(), res.end(), expected.begin(), expected.end());
    expectEmpty();
}

/** @short Test that simple filter for != string */
BOOST_FIXTURE_TEST_CASE(json_kindInstances_filterNe, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"kindInstances\",\"tag\":\"T\",\"kindName\":\"blah\",\"revision\":\"r666\",\"filter\":{\"condition\":\"columnNe\",\"kind\":\"kind\",\"attribute\":\"attr\",\"value\":\"foo\"}}\n");
    expectRead("{\"kindInstances\": [], \"response\": \"kindInstances\", \"tag\":\"T\"}\n");
    vector<Identifier> expected;
    vector<Identifier> res = j->kindInstances("blah", Filter(AttributeExpression(FILTER_COLUMN_NE, "kind", "attr", Value("foo"))), RevisionId(666));
    BOOST_CHECK_EQUAL_COLLECTIONS(res.begin(), res.end(), expected.begin(), expected.end());
    expectEmpty();
}

/** @short Test for AND, less-than and greater-or-equal */
BOOST_FIXTURE_TEST_CASE(json_kindInstances_filter_and_lt_ge, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"kindInstances\",\"tag\":\"T\",\"kindName\":\"blah\",\"revision\":\"r666\",\"filter\":"
                "{\"operator\":\"and\",\"operands\":[{\"condition\":\"columnLt\",\"kind\":\"kind1\",\"attribute\":\"attr1\",\"value\":666},"
                "{\"condition\":\"columnGe\",\"kind\":\"kind1\",\"attribute\":\"attr2\",\"value\":333}]}}\n");
    expectRead("{\"kindInstances\": [], \"tag\":\"T\", \"response\": \"kindInstances\"}\n");
    vector<Identifier> expected;
    std::vector<Expression> expressions;
    expressions.push_back(AttributeExpression(FILTER_COLUMN_LT, "kind1", "attr1", Value(666)));
    expressions.push_back(AttributeExpression(FILTER_COLUMN_GE, "kind1", "attr2", Value(333)));
    vector<Identifier> res = j->kindInstances("blah", Filter(AndFilter(expressions)), RevisionId(666));
    BOOST_CHECK_EQUAL_COLLECTIONS(res.begin(), res.end(), expected.begin(), expected.end());
    expectEmpty();
}

/** @short Test for OR, greater-than and less-or-equal */
BOOST_FIXTURE_TEST_CASE(json_kindInstances_filter_or_gt_le, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"kindInstances\",\"tag\":\"T\",\"kindName\":\"blah\",\"revision\":\"r666\",\"filter\":"
                "{\"operator\":\"or\",\"operands\":[{\"condition\":\"columnGt\",\"kind\":\"kind1\",\"attribute\":\"attr1\",\"value\":666},"
                "{\"condition\":\"columnLe\",\"kind\":\"kind1\",\"attribute\":\"attr2\",\"value\":333}]}}\n");
    expectRead("{\"kindInstances\": [], \"response\": \"kindInstances\", \"tag\":\"T\"}\n");
    vector<Identifier> expected;
    std::vector<Expression> expressions;
    expressions.push_back(AttributeExpression(FILTER_COLUMN_GT, "kind1", "attr1", Value(666)));
    expressions.push_back(AttributeExpression(FILTER_COLUMN_LE, "kind1", "attr2", Value(333)));
    vector<Identifier> res = j->kindInstances("blah", Filter(OrFilter(expressions)), RevisionId(666));
    BOOST_CHECK_EQUAL_COLLECTIONS(res.begin(), res.end(), expected.begin(), expected.end());
    expectEmpty();
}


/** @short Basic test for objectData() */
BOOST_FIXTURE_TEST_CASE(json_objectData, JsonApiTestFixtureFailOnStreamThrow)
{
    // The JsonApiParser needs to know type information for the individual object kinds
    expectWrite("{\"command\":\"kindAttributes\",\"tag\":\"T\",\"kindName\":\"kk\"}\n");
    expectRead("{\"kindAttributes\": {\"int\": \"int\", \"baz\": \"identifier\", \"foo\": \"string\", \n"
               "\"real\": \"double\", \"price\": \"double\", \"template\": \"int\", \"anotherKind\": \"int\"}, "
               "\"tag\":\"T\", \"response\": \"kindAttributes\"}\n");
    // ... as well as relation information for proper filtering
    expectWrite("{\"command\":\"kindRelations\",\"tag\":\"T\",\"kindName\":\"kk\"}\n");
    expectRead("{\"kindRelations\": ["
               "{\"relation\": \"TEMPLATIZED\", \"target\": \"by-which-kind\"}, "
               "{\"relation\": \"MERGE_WITH\", \"target\": \"anotherKind\"}"
               "], \"tag\":\"T\", \"response\": \"kindRelations\"}\n");

    expectWrite("{\"command\":\"objectData\",\"tag\":\"T\",\"kindName\":\"kk\",\"objectName\":\"oo\",\"revision\":\"r3\"}\n");
    expectRead("{\"tag\":\"T\", \"objectData\": {\"foo\": \"bar\", \"baz\": \"id\", \"int\": 10, \"real\": 100.666, \"price\": 666}, "
            "\"response\": \"objectData\"}\n");
    map<Identifier,Value> expected;
    expected["foo"] = "bar";
    expected["int"] = 10;
    expected["real"] = 100.666;
    // Yes, check int-to-float comparison here
    expected["price"] = 666.0;
    expected["baz"] = "id";
    map<Identifier,Value> res = j->objectData("kk", "oo", RevisionId(3));
    // This won't work on floats...
    //BOOST_CHECK(std::equal(res.begin(), res.end(), expected.begin()));
    // ...which is why we have to resort to implicit iteration here:
    BOOST_REQUIRE_EQUAL(res.size(), expected.size());
    map<Identifier,Value>::iterator i1 = expected.begin(), i2 = res.begin();
    while (i1 != expected.end()) {
        BOOST_REQUIRE_EQUAL(i1->first, i2->first);
        if (!i1->second || !i2->second) {
            // compare against a null
            BOOST_REQUIRE_EQUAL(i1->second, i2->second);
        } else {
            boost::apply_visitor(FuzzyTestCompareDeskaValue(), *(i1->second), *(i2->second));
        }
        ++i1;
        ++i2;
    }
    expectEmpty();
}

/** @short Basic functionality of multipleObjectData() */
BOOST_FIXTURE_TEST_CASE(json_multipleObjectData, JsonApiTestFixtureFailOnStreamThrow)
{
    // The JsonApiParser needs to know type information for the individual object kinds
    expectWrite("{\"command\":\"kindAttributes\",\"tag\":\"T\",\"kindName\":\"kk\"}\n");
    expectRead("{\"kindAttributes\": {\"int\": \"int\", \"baz\": \"identifier\", \"foo\": \"string\", \n"
               "\"template\": \"int\", \"anotherKind\": \"int\"}, \"tag\":\"T\", \"response\": \"kindAttributes\"}\n");
    // ... as well as relation information for proper filtering
    expectWrite("{\"command\":\"kindRelations\",\"tag\":\"T\",\"kindName\":\"kk\"}\n");
    expectRead("{\"kindRelations\": ["
               "{\"relation\": \"TEMPLATIZED\", \"target\": \"by-which-kind\"}, "
               "{\"relation\": \"MERGE_WITH\", \"target\": \"anotherKind\"}"
               "],\"tag\":\"T\", \"response\": \"kindRelations\"}\n");

    expectWrite("{\"command\":\"multipleObjectData\",\"tag\":\"T\",\"kindName\":\"kk\",\"filter\":{\"condition\":\"columnNe\",\"kind\":\"kind\",\"attribute\":\"int\",\"value\":666}}\n");
    expectRead("{\"multipleObjectData\": {"
               "\"a\": {\"foo\": \"barA\", \"baz\": \"idA\", \"int\": 10}, "
               "\"b\": {\"foo\": \"barB\", \"baz\": \"idB\", \"int\": 20} "
               "}, \"tag\":\"T\", \"response\": \"multipleObjectData\"}\n");
    map<Identifier, map<Identifier,Value> > expected;
    expected["a"]["foo"] = "barA";
    expected["a"]["int"] = 10;
    expected["a"]["baz"] = "idA";
    expected["b"]["foo"] = "barB";
    expected["b"]["int"] = 20;
    expected["b"]["baz"] = "idB";
    // Check just the interesting items
    map<Identifier, map<Identifier,Value> > res = j->multipleObjectData("kk", AttributeExpression(FILTER_COLUMN_NE, "kind", "int", Value(666)));
    BOOST_CHECK(std::equal(res.begin(), res.end(), expected.begin()));
    expectEmpty();
}

/** @short Basic test for resolvedObjectData() */
BOOST_FIXTURE_TEST_CASE(json_resolvedObjectData, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"kindAttributes\",\"tag\":\"T\",\"kindName\":\"kk\"}\n");
    expectRead("{\"kindAttributes\": {\"baz\": \"int\", \"foo\": \"string\"}, \n"
               "\"tag\":\"T\", \"response\": \"kindAttributes\"}\n");
    expectWrite("{\"command\":\"kindRelations\",\"tag\":\"T\",\"kindName\":\"kk\"}\n");
    expectRead("{\"response\":\"kindRelations\", \"tag\":\"T\", \"kindRelations\": []}\n");

    expectWrite("{\"command\":\"resolvedObjectData\",\"tag\":\"T\",\"kindName\":\"kk\",\"objectName\":\"oo\"}\n");
    expectRead("{\"resolvedObjectData\": "
            "{\"foo\": [\"obj-defining-this\", \"bar\"], \"baz\": [\"this-obj\", 666]}, \"tag\":\"T\", \"response\": \"resolvedObjectData\"}\n");
    map<Identifier, std::pair<Identifier,Value> > expected;
    expected["foo"] = std::make_pair("obj-defining-this", "bar");
    expected["baz"] = std::make_pair("this-obj", 666);
    map<Identifier, std::pair<Identifier,Value> > res = j->resolvedObjectData("kk", "oo");
    // In this case, we limit ourselves to string comparisons. There's a map invloved here, which means that
    // BOOST_CHECK_EQUAL_COLLECTIONS is worthless, and there isn't much point in duplicating the whole logic from json_objectData
    // at yet another place. Let's stick with strings and don't expect to see detailed error reporting here.
    BOOST_REQUIRE_EQUAL(res.size(), expected.size());
    BOOST_CHECK(std::equal(res.begin(), res.end(), expected.begin()));
    expectEmpty();
}

/** @short Basic functionality of multipleResolvedObjectData() */
BOOST_FIXTURE_TEST_CASE(json_multipleResolvedObjectData, JsonApiTestFixtureFailOnStreamThrow)
{
    // The JsonApiParser needs to know type information for the individual object kinds
    expectWrite("{\"command\":\"kindAttributes\",\"tag\":\"T\",\"kindName\":\"kk\"}\n");
    expectRead("{\"kindAttributes\": {\"int\": \"int\", \"baz\": \"identifier\", \"foo\": \"string\", \n"
               "\"template\": \"int\", \"anotherKind\": \"int\"}, "
               "\"tag\":\"T\", \"response\": \"kindAttributes\"}\n");
    // ... as well as relation information for proper filtering
    expectWrite("{\"command\":\"kindRelations\",\"tag\":\"T\",\"kindName\":\"kk\"}\n");
    expectRead("{\"kindRelations\": ["
               "{\"relation\": \"TEMPLATIZED\", \"target\": \"by-which-kind\"}, "
               "{\"relation\": \"MERGE_WITH\", \"target\": \"anotherKind\"}"
               "], \"tag\":\"T\", \"response\": \"kindRelations\"}\n");

    expectWrite("{\"command\":\"multipleResolvedObjectData\",\"tag\":\"T\",\"kindName\":\"kk\",\"filter\":{\"condition\":\"columnNe\",\"kind\":\"kind1\",\"attribute\":\"int\",\"value\":666}}\n");
    expectRead("{\"multipleResolvedObjectData\": {"
               "\"a\": {\"foo\": [\"1\", \"barA\"], \"baz\": [\"1\", \"idA\"], \"int\": [\"11\", 10]}, "
               "\"b\": {\"foo\": [\"1\", \"barB\"], \"baz\": [\"2\", \"idB\"], \"int\": [\"22\", 20]} "
               "}, \"tag\":\"T\", \"response\": \"multipleResolvedObjectData\"}\n");
    map<Identifier, map<Identifier,std::pair<Identifier, Value> > > expected;
    expected["a"]["foo"] = std::make_pair("1", "barA");
    expected["a"]["baz"] = std::make_pair("1", "idA");
    expected["a"]["int"] = std::make_pair("11", 10);
    expected["b"]["foo"] = std::make_pair("1", "barB");
    expected["b"]["baz"] = std::make_pair("2", "idB");
    expected["b"]["int"] = std::make_pair("22", 20);
    map<Identifier, map<Identifier, std::pair<Identifier, Value> > > res = j->multipleResolvedObjectData("kk", AttributeExpression(FILTER_COLUMN_NE, "kind1", "int", Value(666)));
    BOOST_CHECK(std::equal(res.begin(), res.end(), expected.begin()));
    expectEmpty();
}

/** @short Basic test for createObject() */
BOOST_FIXTURE_TEST_CASE(json_createObject, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"createObject\",\"tag\":\"T\",\"kindName\":\"k\",\"objectName\":\"o\"}\n");
    expectRead("{\"response\": \"createObject\",\"tag\":\"T\"}\n");
    j->createObject("k", "o");
    expectEmpty();
}

/** @short Basic test for deleteObject() */
BOOST_FIXTURE_TEST_CASE(json_deleteObject, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"deleteObject\",\"tag\":\"T\",\"kindName\":\"k\",\"objectName\":\"o\"}\n");
    expectRead("{\"response\": \"deleteObject\",\"tag\":\"T\"}\n");
    j->deleteObject("k", "o");
    expectEmpty();
}

/** @short Basic test for restoreDeletedObject() */
BOOST_FIXTURE_TEST_CASE(json_restoreDeletedObject, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"restoreDeletedObject\",\"tag\":\"T\",\"kindName\":\"k\",\"objectName\":\"o\"}\n");
    expectRead("{\"response\": \"restoreDeletedObject\",\"tag\":\"T\"}\n");
    j->restoreDeletedObject("k", "o");
    expectEmpty();
}

/** @short Basic test for renameObject() */
BOOST_FIXTURE_TEST_CASE(json_renameObject, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"renameObject\",\"tag\":\"T\",\"kindName\":\"kind\",\"oldObjectName\":\"ooooold\",\"newObjectName\":\"new\"}\n");
    expectRead("{\"response\": \"renameObject\",\"tag\":\"T\"}\n");
    j->renameObject("kind", "ooooold", "new");
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
    std::string jsonInputPrefix = "{\"command\":\"setAttribute\",\"tag\":\"T\",\"kindName\":\"k\",\"objectName\":\"o\",\"attributeName\":\"a\",\"attributeData\":";
    std::string jsonOutput = "{\"response\": \"setAttribute\", \"tag\":\"T\"}\n";
    data.push_back(SetAttrTestData(jsonInputPrefix + "\"some string\"}\n", jsonOutput, Deska::Db::Value("some string")));
    data.push_back(SetAttrTestData(jsonInputPrefix + "123}\n", jsonOutput, Deska::Db::Value(123)));
    data.push_back(SetAttrTestData(jsonInputPrefix + "333.666}\n", jsonOutput, Deska::Db::Value(333.666)));
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
    expectWrite("{\"command\":\"startChangeset\",\"tag\":\"T\"}\n");
    expectRead("{\"response\": \"startChangeset\", \"tag\":\"T\", \"startChangeset\": \"tmp333\"}\n");
    BOOST_CHECK_EQUAL(j->startChangeset(), TemporaryChangesetId(333));
    expectEmpty();
}

/** @short Basic test for commitChangeset() */
BOOST_FIXTURE_TEST_CASE(json_commitChangeset, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"commitChangeset\",\"tag\":\"T\",\"commitMessage\":\"description\"}\n");
    expectRead("{\"response\": \"commitChangeset\", \"tag\":\"T\", \"commitChangeset\": \"r666\"}\n");
    BOOST_CHECK_EQUAL(j->commitChangeset("description"), RevisionId(666));
    expectEmpty();
}

/** @short Basic test for reabseChangeset() */
BOOST_FIXTURE_TEST_CASE(json_rebaseChangeset, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"rebaseChangeset\",\"tag\":\"T\",\"parentRevision\":\"r666\"}\n");
    expectRead("{\"response\": \"rebaseChangeset\", \"tag\":\"T\", \"parentRevision\": \"r666\"}\n");
    j->rebaseChangeset(RevisionId(666));
    expectEmpty();
}

/** @short Basic test for pendingChangesets() */
BOOST_FIXTURE_TEST_CASE(json_pendingChangesets, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"pendingChangesets\",\"tag\":\"T\"}\n");
    expectRead("{\"response\": \"pendingChangesets\", \"tag\":\"T\", \"pendingChangesets\": ["
               "{\"changeset\":\"tmp123\", \"author\": \"user\", \"status\": \"DETACHED\", "
                   "\"timestamp\": \"2011-04-07 17:22:33\", \"parentRevision\": \"r666\", \"message\": \"message\"}, "
               // The next one is the same, except that we use JSON's null here
               "{\"changeset\":\"tmp123\", \"author\": \"user\", \"status\": \"DETACHED\", "
                   "\"timestamp\": \"2011-04-07 17:22:33\", \"parentRevision\": \"r666\", \"message\": \"message\", "
                   "\"activeConnectionInfo\": null}, "
               "{\"changeset\":\"tmp333\", \"author\": \"bar\", \"status\": \"INPROGRESS\", "
                   "\"timestamp\": \"2011-04-07 17:22:33\", \"parentRevision\": \"r666\", \"message\": \"foo\", "
                   "\"activeConnectionInfo\": \"unspecified connection\"}"
               "]}\n");
    vector<PendingChangeset> expected;
    expected.push_back(PendingChangeset(
                           TemporaryChangesetId(123), "user",
                           boost::posix_time::ptime(boost::gregorian::date(2011, 4, 7), boost::posix_time::time_duration(17, 22, 33)),
                           RevisionId(666), "message", PendingChangeset::ATTACH_DETACHED, boost::optional<std::string>()));
    // repeat the first line once again
    expected.push_back(expected.front());
    expected.push_back(PendingChangeset(
                           TemporaryChangesetId(333), "bar",
                           boost::posix_time::ptime(boost::gregorian::date(2011, 4, 7), boost::posix_time::time_duration(17, 22, 33)),
                           RevisionId(666),"foo", PendingChangeset::ATTACH_IN_PROGRESS,
                           boost::optional<std::string>("unspecified connection")));
    vector<PendingChangeset> res = j->pendingChangesets();
    BOOST_CHECK_EQUAL_COLLECTIONS(res.begin(), res.end(), expected.begin(), expected.end());
    expectEmpty();
}

/** @short Test that simple filter for pendingChangesets against TemporaryChangesetId works */
BOOST_FIXTURE_TEST_CASE(json_pendingChangesets_filterNe, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"pendingChangesets\",\"tag\":\"T\",\"filter\":{\"condition\":\"columnNe\",\"metadata\":\"changeset\",\"value\":\"tmp123\"}}\n");
    expectRead("{\"response\": \"pendingChangesets\", \"tag\":\"T\", \"pendingChangesets\": []}\n");
    std::vector<PendingChangeset> expected;
    std::vector<PendingChangeset> res = j->pendingChangesets(Filter(MetadataExpression(FILTER_COLUMN_NE, "changeset", TemporaryChangesetId(123))));
    BOOST_CHECK_EQUAL_COLLECTIONS(res.begin(), res.end(), expected.begin(), expected.end());
    expectEmpty();
}

/** @short Test that simple filter for pendingChangesets against "being detached" works */
BOOST_FIXTURE_TEST_CASE(json_pendingChangesets_filterStatusDetached, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"pendingChangesets\",\"tag\":\"T\",\"filter\":{\"condition\":\"columnEq\",\"metadata\":\"status\",\"value\":\"DETACHED\"}}\n");
    expectRead("{\"response\": \"pendingChangesets\", \"tag\":\"T\", \"pendingChangesets\": []}\n");
    std::vector<PendingChangeset> expected;
    std::vector<PendingChangeset> res = j->pendingChangesets(Filter(MetadataExpression(FILTER_COLUMN_EQ, "status", PendingChangeset::ATTACH_DETACHED)));
    BOOST_CHECK_EQUAL_COLLECTIONS(res.begin(), res.end(), expected.begin(), expected.end());
    expectEmpty();
}

/** @short Test that simple filter for pendingChangesets against "being in progress" works */
BOOST_FIXTURE_TEST_CASE(json_pendingChangesets_filterStatusInProgress, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"pendingChangesets\",\"tag\":\"T\",\"filter\":{\"condition\":\"columnEq\",\"metadata\":\"status\",\"value\":\"INPROGRESS\"}}\n");
    expectRead("{\"response\": \"pendingChangesets\", \"tag\":\"T\", \"pendingChangesets\": []}\n");
    std::vector<PendingChangeset> expected;
    std::vector<PendingChangeset> res = j->pendingChangesets(Filter(MetadataExpression(FILTER_COLUMN_EQ, "status", PendingChangeset::ATTACH_IN_PROGRESS)));
    BOOST_CHECK_EQUAL_COLLECTIONS(res.begin(), res.end(), expected.begin(), expected.end());
    expectEmpty();
}

/** @short Basic test for resumeChangeset() */
BOOST_FIXTURE_TEST_CASE(json_resumeChangeset, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"resumeChangeset\",\"tag\":\"T\",\"changeset\":\"tmp123\"}\n");
    expectRead("{\"response\": \"resumeChangeset\", \"tag\":\"T\", \"changeset\": \"tmp123\"}\n");
    j->resumeChangeset(TemporaryChangesetId(123));
    expectEmpty();
}

/** @short Basic test for detachFromCurrentChangeset() */
BOOST_FIXTURE_TEST_CASE(json_detachFromCurrentChangeset, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"detachFromCurrentChangeset\",\"tag\":\"T\",\"message\":\"foobar\"}\n");
    expectRead("{\"response\": \"detachFromCurrentChangeset\", \"tag\":\"T\"}\n");
    j->detachFromCurrentChangeset("foobar");
    expectEmpty();
}

/** @short Basic test for abortCurrentChangeset() */
BOOST_FIXTURE_TEST_CASE(json_abortCurrentChangeset, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"abortCurrentChangeset\",\"tag\":\"T\"}\n");
    expectRead("{\"response\": \"abortCurrentChangeset\",\"tag\":\"T\"}\n");
    j->abortCurrentChangeset();
    expectEmpty();
}

/** @short Basic test for freezeView() */
BOOST_FIXTURE_TEST_CASE(json_freezeView, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"freezeView\",\"tag\":\"T\"}\n");
    expectRead("{\"response\": \"freezeView\", \"tag\":\"T\"}\n");
    j->freezeView();
    expectEmpty();
}

/** @short Basic test for unFreezeView() */
BOOST_FIXTURE_TEST_CASE(json_unFreezeView, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"unFreezeView\",\"tag\":\"T\"}\n");
    expectRead("{\"response\": \"unFreezeView\", \"tag\":\"T\"}\n");
    j->unFreezeView();
    expectEmpty();
}


/** @short Test listRevisions() from JSON */
BOOST_FIXTURE_TEST_CASE(json_listRevisions, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"listRevisions\",\"tag\":\"T\"}\n");
    expectRead("{\"response\": \"listRevisions\", \"listRevisions\": ["
               "{\"revision\": \"r123\", \"author\": \"user\", \"timestamp\": \"2011-04-07 17:22:33\", \"commitMessage\": \"message\"}"
               "], \"tag\":\"T\"}\n");
    std::vector<RevisionMetadata> expected;
    expected.push_back(RevisionMetadata(
                           RevisionId(123), "user",
                           boost::posix_time::ptime(boost::gregorian::date(2011, 4, 7), boost::posix_time::time_duration(17, 22, 33)),
                           "message"));
    std::vector<RevisionMetadata> res = j->listRevisions();
    BOOST_CHECK_EQUAL_COLLECTIONS(res.begin(), res.end(), expected.begin(), expected.end());
    expectEmpty();
}

/** @short Test that simple filter for listRevisions works */
BOOST_FIXTURE_TEST_CASE(json_listRevisions_filterEq, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"listRevisions\",\"tag\":\"T\",\"filter\":{\"condition\":\"columnEq\",\"metadata\":\"revision\",\"value\":\"r123\"}}\n");
    expectRead("{\"response\": \"listRevisions\", \"tag\":\"T\", \"listRevisions\": []}\n");
    std::vector<RevisionMetadata> expected;
    std::vector<RevisionMetadata> res = j->listRevisions(Filter(MetadataExpression(FILTER_COLUMN_EQ, "revision", RevisionId(123))));
    BOOST_CHECK_EQUAL_COLLECTIONS(res.begin(), res.end(), expected.begin(), expected.end());
    expectEmpty();
}

namespace {
std::string exampleJsonDiff =
    "{\"command\":\"createObject\",\"kindName\":\"k1\",\"objectName\":\"o1\"},"
    "{\"command\":\"deleteObject\",\"kindName\":\"k2\",\"objectName\":\"o2\"},"
    "{\"command\":\"renameObject\",\"kindName\":\"k3\",\"oldObjectName\":\"ooooold\",\"newObjectName\":\"new\"},"
    "{\"command\":\"setAttribute\",\"kindName\":\"k5\",\"objectName\":\"o5\",\"attributeName\":\"a5\",\"attributeData\":\"new\",\"oldAttributeData\":\"old\"}";

std::vector<ObjectModification> diffObjects()
{
    std::vector<ObjectModification> res;
    res.push_back(CreateObjectModification("k1", "o1"));
    res.push_back(DeleteObjectModification("k2", "o2"));
    res.push_back(RenameObjectModification("k3", "ooooold", "new"));
    res.push_back(SetAttributeModification("k5", "o5", "a5", Deska::Db::Value("new"), Deska::Db::Value("old")));
    return res;
}
}

void schemeForDiff(JsonApiTestFixtureFailOnStreamThrow &f)
{
    f.expectWrite("{\"command\":\"kindNames\",\"tag\":\"T\"}\n");
    f.expectRead("{\"response\": \"kindNames\", \"tag\":\"T\", \"kindNames\": [\"k1\", \"k2\", \"k3\", \"k4\", \"k5\"]}\n");

    f.expectWrite("{\"command\":\"kindAttributes\",\"tag\":\"T\",\"kindName\":\"k1\"}\n");
    f.expectRead("{\"kindAttributes\": {}, \"tag\":\"T\", \"response\": \"kindAttributes\"}\n");
    f.expectWrite("{\"command\":\"kindAttributes\",\"tag\":\"T\",\"kindName\":\"k2\"}\n");
    f.expectRead("{\"kindAttributes\": {}, \"tag\":\"T\", \"response\": \"kindAttributes\"}\n");
    f.expectWrite("{\"command\":\"kindAttributes\",\"tag\":\"T\",\"kindName\":\"k3\"}\n");
    f.expectRead("{\"kindAttributes\": {}, \"tag\":\"T\", \"response\": \"kindAttributes\"}\n");
    f.expectWrite("{\"command\":\"kindAttributes\",\"tag\":\"T\",\"kindName\":\"k4\"}\n");
    f.expectRead("{\"kindAttributes\": {\"fancyAttr\": \"int\"}, \"tag\":\"T\", \"response\": \"kindAttributes\"}\n");
    f.expectWrite("{\"command\":\"kindAttributes\",\"tag\":\"T\",\"kindName\":\"k5\"}\n");
    f.expectRead("{\"kindAttributes\": {\"a5\": \"string\"}, \"tag\":\"T\", \"response\": \"kindAttributes\"}\n");

}

/** @short Test dataDifference() from JSON */
BOOST_FIXTURE_TEST_CASE(json_dataDifference, JsonApiTestFixtureFailOnStreamThrow)
{
    schemeForDiff(*this);
    expectWrite("{\"command\":\"dataDifference\",\"tag\":\"T\",\"revisionA\":\"r1\",\"revisionB\":\"r2\","
                "\"filter\":{\"condition\":\"columnEq\",\"kind\":\"kind1\",\"attribute\":\"attr1\",\"value\":null}}\n");
    expectRead("{\"response\": \"dataDifference\", \"tag\":\"T\", \"dataDifference\": ["
               + exampleJsonDiff + "]}\n");
    std::vector<ObjectModification> expected = diffObjects();
    std::vector<ObjectModification> res = j->dataDifference(RevisionId(1), RevisionId(2),
                                                            Filter(AttributeExpression(FILTER_COLUMN_EQ, "kind1", "attr1", Value())));
    BOOST_CHECK_EQUAL_COLLECTIONS(res.begin(), res.end(), expected.begin(), expected.end());
    expectEmpty();
}

/** @short Test dataDifferenceInTemporaryChangeset() from JSON */
BOOST_FIXTURE_TEST_CASE(json_dataDifferenceInTemporaryChangeset, JsonApiTestFixtureFailOnStreamThrow)
{
    schemeForDiff(*this);
    expectWrite("{\"command\":\"dataDifferenceInTemporaryChangeset\",\"tag\":\"T\",\"changeset\":\"tmp666\"}\n");
    expectRead("{\"response\": \"dataDifferenceInTemporaryChangeset\", \"tag\":\"T\", \"dataDifferenceInTemporaryChangeset\": ["
               + exampleJsonDiff +
               "]}\n");
    std::vector<ObjectModification> expected = diffObjects();
    std::vector<ObjectModification> res = j->dataDifferenceInTemporaryChangeset(TemporaryChangesetId(666));
    BOOST_CHECK_EQUAL_COLLECTIONS(res.begin(), res.end(), expected.begin(), expected.end());
    expectEmpty();
}

/** @short Test resolvedDataDifference() from JSON */
BOOST_FIXTURE_TEST_CASE(json_resolvedDataDifference, JsonApiTestFixtureFailOnStreamThrow)
{
    schemeForDiff(*this);
    expectWrite("{\"command\":\"resolvedDataDifference\",\"tag\":\"T\",\"revisionA\":\"r1\",\"revisionB\":\"r2\","
                "\"filter\":{\"condition\":\"columnEq\",\"kind\":\"kind1\",\"attribute\":\"attr1\",\"value\":null}}\n");
    expectRead("{\"response\": \"resolvedDataDifference\", \"tag\":\"T\", \"resolvedDataDifference\": ["
               + exampleJsonDiff +
               "], \"filter\":{\"condition\":\"columnEq\",\"kind\":\"kind1\",\"attribute\":\"attr1\",\"value\":null}}\n");
    std::vector<ObjectModification> expected = diffObjects();
    std::vector<ObjectModification> res = j->resolvedDataDifference(RevisionId(1), RevisionId(2),
                                                                    Filter(AttributeExpression(FILTER_COLUMN_EQ, "kind1", "attr1", Value())));
    BOOST_CHECK_EQUAL_COLLECTIONS(res.begin(), res.end(), expected.begin(), expected.end());
    expectEmpty();
}

/** @short Test resolvedDataDifferenceInTemporaryChangeset() from JSON */
BOOST_FIXTURE_TEST_CASE(json_resolvedDataDifferenceInTemporaryChangeset, JsonApiTestFixtureFailOnStreamThrow)
{
    schemeForDiff(*this);
    expectWrite("{\"command\":\"resolvedDataDifferenceInTemporaryChangeset\",\"tag\":\"T\",\"changeset\":\"tmp1\","
                "\"filter\":{\"condition\":\"columnEq\",\"kind\":\"kind1\",\"attribute\":\"attr1\",\"value\":null}}\n");
    expectRead("{\"response\": \"resolvedDataDifferenceInTemporaryChangeset\", \"tag\":\"T\", \"resolvedDataDifferenceInTemporaryChangeset\": ["
               + exampleJsonDiff + "]}\n");
    std::vector<ObjectModification> expected = diffObjects();
    std::vector<ObjectModification> res = j->resolvedDataDifferenceInTemporaryChangeset(
                TemporaryChangesetId(1), Filter(AttributeExpression(FILTER_COLUMN_EQ, "kind1", "attr1", Value())));
    BOOST_CHECK_EQUAL_COLLECTIONS(res.begin(), res.end(), expected.begin(), expected.end());
    expectEmpty();
}



/** @short Test applyBatchedChanges() from JSON */
BOOST_FIXTURE_TEST_CASE(json_applyBatchedChanges, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"applyBatchedChanges\",\"tag\":\"T\",\"modifications\":[" + exampleJsonDiff + "]}\n");
    expectRead("{\"response\": \"applyBatchedChanges\", \"tag\":\"T\"}\n");
    j->applyBatchedChanges(diffObjects());
    expectEmpty();
}

/** @short Test that we catch reports of server-side exceptions */
BOOST_FIXTURE_TEST_CASE(json_exceptions, JsonApiTestFixtureFailOnStreamThrow)
{
#define JSON_ERR_TEST(X) \
    expectWrite("{\"command\":\"startChangeset\",\"tag\":\"T\"}\n"); \
    expectRead("{\"dbException\": {\"type\":\"" #X "\",\"message\":\"x\"}}\n"); \
    BOOST_CHECK_THROW(j->startChangeset(), X); expectEmpty();

    JSON_ERR_TEST(NotFoundError);
    JSON_ERR_TEST(NoChangesetError);
    JSON_ERR_TEST(ChangesetAlreadyOpenError);
    JSON_ERR_TEST(FilterError);
    JSON_ERR_TEST(ReCreateObjectError);
    JSON_ERR_TEST(InvalidKindError);
    JSON_ERR_TEST(InvalidAttributeError);
    JSON_ERR_TEST(SqlError);
    JSON_ERR_TEST(ServerError);
#undef JSON_ERR_TEST
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
        expectWrite("{\"command\":\"abortCurrentChangeset\",\"tag\":\"T\"}\n");
        if (!line.empty())
            expectRead(line);
        mockBuffer.expectReadEof();
        BOOST_CHECK_THROW(j->abortCurrentChangeset(), JsonSyntaxError);
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
    line = "{\"command\":\"c\",\"tag\":\"T\"";
}

BOOST_FIXTURE_TEST_CASE(json_malformed_json_missing_next_command, MalformedJsonFixture)
{
    line = "{\"command\":\"c\",";
}

BOOST_FIXTURE_TEST_CASE(json_malformed_json_missing_comma, MalformedJsonFixture)
{
    line = "{\"command\":\"c\"";
}
