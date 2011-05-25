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
    BOOST_CHECK_THROW(j->kindAttributes("some-object"), JsonStructureError);
    expectEmpty();
}

/** @short Test that kindRelations() can fetch data */
BOOST_FIXTURE_TEST_CASE(json_kindRelations, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"kindRelations\",\"kindName\":\"identifier\"}\n");
    expectRead("{\"kindName\": \"identifier\", \"kindRelations\": ["
            "{\"relation\": \"EMBED_INTO\", \"into\": \"hardware\"}, "
            "{\"relation\": \"MERGE_WITH\", \"targetTableName\": \"second-kind\", \"sourceAttribute\": \"my-attribute\"}, "
            "{\"relation\": \"IS_TEMPLATE\", \"toWhichKind\": \"target-kind\"}, "
            "{\"relation\": \"TEMPLATIZED\", \"byWhichKind\": \"by-which-kind\", \"sourceAttribute\": \"my-attribute\"}], \"response\": \"kindRelations\"}\n");
    vector<ObjectRelation> expected;
    expected.push_back(ObjectRelation::embedInto("hardware"));
    expected.push_back(ObjectRelation::mergeWith("second-kind", "my-attribute"));
    expected.push_back(ObjectRelation::isTemplate("target-kind"));
    expected.push_back(ObjectRelation::templatized("by-which-kind", "my-attribute"));
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
    data.push_back(std::make_pair<string,string>("{\"rel\": \"EMBED_INTO\"}", "Mandatory field 'relation' not present in the response"));
    data.push_back(std::make_pair<string,string>("{\"relation\": \"EMBED_INTO2\"}", "Invalid relation kind 'EMBED_INTO2'"));
    data.push_back(std::make_pair<string,string>("{\"relation\": \"EMBED_INTO\"}", "Mandatory field 'into' not present in the response"));
    data.push_back(std::make_pair<string,string>("{\"relation\": \"EMBED_INTO\", \"into\": \"hardware\", \"foo\": \"bar\"}",
                                                 "JSON field 'foo' is not allowed in this context (expecting one of: relation into)."));
    BOOST_FOREACH(const PairStringString &value, data) {
        expectWrite("{\"command\":\"kindRelations\",\"kindName\":\"identifier\"}\n");
        expectRead("{\"kindName\": \"identifier\", \"kindRelations\": [" + value.first + "], \"response\": \"kindRelations\"}\n");
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
    expectWrite("{\"command\":\"kindInstances\",\"kindName\":\"blah\",\"revision\":\"r666\"}\n");
    expectRead("{\"kindName\": \"blah\", \"kindInstances\": [\"foo\", \"bar\", \"ahoj\"], \"response\": \"kindInstances\", \"revision\": \"r666\"}\n");
    vector<Identifier> expected;
    expected.push_back("foo");
    expected.push_back("bar");
    expected.push_back("ahoj");
    vector<Identifier> res = j->kindInstances("blah", boost::optional<Filter>(), RevisionId(666));
    BOOST_CHECK_EQUAL_COLLECTIONS(res.begin(), res.end(), expected.begin(), expected.end());
    expectEmpty();
}

/** @short Test that kindInstances() fails when faced with wrong revision */
BOOST_FIXTURE_TEST_CASE(json_kindInstances_wrong_revision, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"kindInstances\",\"kindName\":\"blah\",\"revision\":\"r666\"}\n");
    expectRead("{\"kindName\": \"blah\", \"kindInstances\": [\"foo\", \"bar\", \"ahoj\"], \"response\": \"kindInstances\", \"revision\": \"r333\"}\n");
    BOOST_CHECK_THROW(j->kindInstances("blah", boost::optional<Filter>(), RevisionId(666)), JsonStructureError);
    expectEmpty();
}

/** @short Test that simple filter for IS NULL works fine */
BOOST_FIXTURE_TEST_CASE(json_kindInstances_filterEq, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"kindInstances\",\"kindName\":\"blah\",\"revision\":\"r666\",\"filter\":{\"condition\":\"columnEq\",\"column\":\"attr\",\"value\":null}}\n");
    expectRead("{\"kindName\": \"blah\", \"kindInstances\": [], \"response\": \"kindInstances\", \"revision\": \"r666\", "
               "\"filter\": {\"condition\": \"columnEq\", \"column\": \"attr\", \"value\": null}}\n");
    vector<Identifier> expected;
    vector<Identifier> res = j->kindInstances("blah", Filter(Expression(FILTER_COLUMN_EQ, "attr", Value())), RevisionId(666));
    BOOST_CHECK_EQUAL_COLLECTIONS(res.begin(), res.end(), expected.begin(), expected.end());
    expectEmpty();
}

/** @short Test that simple filter for != string */
BOOST_FIXTURE_TEST_CASE(json_kindInstances_filterNe, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"kindInstances\",\"kindName\":\"blah\",\"revision\":\"r666\",\"filter\":{\"condition\":\"columnNe\",\"column\":\"attr\",\"value\":\"foo\"}}\n");
    expectRead("{\"kindName\": \"blah\", \"kindInstances\": [], \"response\": \"kindInstances\", \"revision\": \"r666\", "
               "\"filter\": {\"condition\": \"columnNe\", \"column\": \"attr\", \"value\": \"foo\"}}\n");
    vector<Identifier> expected;
    vector<Identifier> res = j->kindInstances("blah", Filter(Expression(FILTER_COLUMN_NE, "attr", Value("foo"))), RevisionId(666));
    BOOST_CHECK_EQUAL_COLLECTIONS(res.begin(), res.end(), expected.begin(), expected.end());
    expectEmpty();
}

/** @short Test for AND, less-than and greater-or-equal */
BOOST_FIXTURE_TEST_CASE(json_kindInstances_filter_and_lt_ge, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"kindInstances\",\"kindName\":\"blah\",\"revision\":\"r666\",\"filter\":"
                "{\"operator\":\"and\",\"operands\":[{\"condition\":\"columnLt\",\"column\":\"attr1\",\"value\":666},"
                "{\"condition\":\"columnGe\",\"column\":\"attr2\",\"value\":333}]}}\n");
    expectRead("{\"kindName\": \"blah\", \"kindInstances\": [], \"response\": \"kindInstances\", \"revision\": \"r666\", "
               "\"filter\": {\"operator\":\"and\",\"operands\":[{\"condition\":\"columnLt\",\"column\":\"attr1\",\"value\":666},"
               "{\"condition\":\"columnGe\",\"column\":\"attr2\",\"value\":333}]}}\n");
    vector<Identifier> expected;
    std::vector<Expression> expressions;
    expressions.push_back(Expression(FILTER_COLUMN_LT, "attr1", Value(666)));
    expressions.push_back(Expression(FILTER_COLUMN_GE, "attr2", Value(333)));
    vector<Identifier> res = j->kindInstances("blah", Filter(AndFilter(expressions)), RevisionId(666));
    BOOST_CHECK_EQUAL_COLLECTIONS(res.begin(), res.end(), expected.begin(), expected.end());
    expectEmpty();
}

/** @short Test for OR, greater-than and less-or-equal */
BOOST_FIXTURE_TEST_CASE(json_kindInstances_filter_or_gt_le, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"kindInstances\",\"kindName\":\"blah\",\"revision\":\"r666\",\"filter\":"
                "{\"operator\":\"or\",\"operands\":[{\"condition\":\"columnGt\",\"column\":\"attr1\",\"value\":666},"
                "{\"condition\":\"columnLe\",\"column\":\"attr2\",\"value\":333}]}}\n");
    expectRead("{\"kindName\": \"blah\", \"kindInstances\": [], \"response\": \"kindInstances\", \"revision\": \"r666\", "
               "\"filter\": {\"operator\":\"or\",\"operands\":[{\"condition\":\"columnGt\",\"column\":\"attr1\",\"value\":666},"
               "{\"condition\":\"columnLe\",\"column\":\"attr2\",\"value\":333}]}}\n");
    vector<Identifier> expected;
    std::vector<Expression> expressions;
    expressions.push_back(Expression(FILTER_COLUMN_GT, "attr1", Value(666)));
    expressions.push_back(Expression(FILTER_COLUMN_LE, "attr2", Value(333)));
    vector<Identifier> res = j->kindInstances("blah", Filter(OrFilter(expressions)), RevisionId(666));
    BOOST_CHECK_EQUAL_COLLECTIONS(res.begin(), res.end(), expected.begin(), expected.end());
    expectEmpty();
}


/** @short Basic test for objectData() */
BOOST_FIXTURE_TEST_CASE(json_objectData, JsonApiTestFixtureFailOnStreamThrow)
{
    // The JsonApiParser needs to know type information for the individual object kinds
    expectWrite("{\"command\":\"kindAttributes\",\"kindName\":\"kk\"}\n");
    expectRead("{\"kindAttributes\": {\"int\": \"int\", \"baz\": \"identifier\", \"foo\": \"string\", \n"
            "\"real\": \"double\", \"price\": \"double\"}, \"kindName\": \"kk\", \"response\": \"kindAttributes\"}\n");

    expectWrite("{\"command\":\"objectData\",\"kindName\":\"kk\",\"objectName\":\"oo\",\"revision\":\"r3\"}\n");
    expectRead("{\"kindName\": \"kk\", \"objectData\": {\"foo\": \"bar\", \"baz\": \"id\", \"int\": 10, \"real\": 100.666, \"price\": 666}, "
            "\"objectName\": \"oo\", \"response\": \"objectData\", \"revision\": \"r3\"}\n");
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

/** @short Basic test for resolvedObjectData() */
BOOST_FIXTURE_TEST_CASE(json_resolvedObjectData, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"kindAttributes\",\"kindName\":\"kk\"}\n");
    expectRead("{\"kindAttributes\": {\"baz\": \"int\", \"foo\": \"string\"}, \n"
               "\"kindName\": \"kk\", \"response\": \"kindAttributes\"}\n");

    expectWrite("{\"command\":\"resolvedObjectData\",\"kindName\":\"kk\",\"objectName\":\"oo\"}\n");
    expectRead("{\"kindName\": \"kk\", \"objectName\": \"oo\", \"resolvedObjectData\": "
            "{\"foo\": [\"obj-defining-this\", \"bar\"], \"baz\": [\"this-obj\", 666]}, \"response\": \"resolvedObjectData\"}\n");
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

/** @short Basic test for createObject() */
BOOST_FIXTURE_TEST_CASE(json_createObject, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"createObject\",\"kindName\":\"k\",\"objectName\":\"o\"}\n");
    expectRead("{\"kindName\": \"k\", \"objectName\": \"o\", \"response\": \"createObject\"}\n");
    j->createObject("k", "o");
    expectEmpty();
}

/** @short Basic test for deleteObject() */
BOOST_FIXTURE_TEST_CASE(json_deleteObject, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"deleteObject\",\"kindName\":\"k\",\"objectName\":\"o\"}\n");
    expectRead("{\"kindName\": \"k\", \"objectName\": \"o\", \"response\": \"deleteObject\"}\n");
    j->deleteObject("k", "o");
    expectEmpty();
}

/** @short Basic test for renameObject() */
BOOST_FIXTURE_TEST_CASE(json_renameObject, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"renameObject\",\"kindName\":\"kind\",\"objectName\":\"ooooold\",\"newObjectName\":\"new\"}\n");
    expectRead("{\"kindName\": \"kind\", \"newObjectName\": \"new\", \"objectName\": \"ooooold\", \"response\": \"renameObject\"}\n");
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
    std::string jsonInputPrefix = "{\"command\":\"setAttribute\",\"kindName\":\"k\",\"objectName\":\"o\",\"attributeName\":\"a\",\"attributeData\":";
    std::string jsonOutputSuffix = "{\"attributeName\": \"a\", \"kindName\": \"k\", \"objectName\": \"o\", "
            "\"response\": \"setAttribute\", \"attributeData\": ";
    data.push_back(SetAttrTestData(jsonInputPrefix + "\"some string\"}\n", jsonOutputSuffix + "\"some string\"}\n", Deska::Db::Value("some string")));
    data.push_back(SetAttrTestData(jsonInputPrefix + "123}\n", jsonOutputSuffix + " 123}\n", Deska::Db::Value(123)));
    data.push_back(SetAttrTestData(jsonInputPrefix + "333.666}\n", jsonOutputSuffix + " 333.666}\n", Deska::Db::Value(333.666)));
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
    expectRead("{\"response\": \"startChangeset\", \"startChangeset\": \"tmp333\"}\n");
    BOOST_CHECK_EQUAL(j->startChangeset(), TemporaryChangesetId(333));
    expectEmpty();
}

/** @short Basic test for commitChangeset() */
BOOST_FIXTURE_TEST_CASE(json_commitChangeset, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"commitChangeset\",\"commitMessage\":\"description\"}\n");
    expectRead("{\"response\": \"commitChangeset\", \"commitChangeset\": \"r666\", \"commitMessage\":\"description\"}\n");
    BOOST_CHECK_EQUAL(j->commitChangeset("description"), RevisionId(666));
    expectEmpty();
}

/** @short Basic test for reabseChangeset() */
BOOST_FIXTURE_TEST_CASE(json_rebaseChangeset, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"rebaseChangeset\",\"parentRevision\":\"r666\"}\n");
    expectRead("{\"response\": \"rebaseChangeset\", \"parentRevision\": \"r666\"}\n");
    j->rebaseChangeset(RevisionId(666));
    expectEmpty();
}

/** @short Basic test for pendingChangesets() */
BOOST_FIXTURE_TEST_CASE(json_pendingChangesets, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"pendingChangesets\"}\n");
    expectRead("{\"response\": \"pendingChangesets\", \"pendingChangesets\": ["
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

/** @short Test that simple filter for pendingChangesets works */
BOOST_FIXTURE_TEST_CASE(json_pendingChangesets_filterNe, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"pendingChangesets\",\"filter\":{\"condition\":\"columnNe\",\"column\":\"revision\",\"value\":\"tmp123\"}}\n");
    expectRead("{\"response\": \"pendingChangesets\", \"filter\":{\"condition\":\"columnNe\",\"column\":\"revision\",\"value\":\"tmp123\"}, \"pendingChangesets\": []}\n");
    std::vector<PendingChangeset> expected;
    std::vector<PendingChangeset> res = j->pendingChangesets(Filter(Expression(FILTER_COLUMN_NE, "revision", TemporaryChangesetId(123))));
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

/** @short Test listRevisions() from JSON */
BOOST_FIXTURE_TEST_CASE(json_listRevisions, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"listRevisions\"}\n");
    expectRead("{\"response\": \"listRevisions\", \"listRevisions\": ["
               "{\"revision\": \"r123\", \"author\": \"user\", \"timestamp\": \"2011-04-07 17:22:33\", \"commitMessage\": \"message\"}"
               "]}\n");
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
    expectWrite("{\"command\":\"listRevisions\",\"filter\":{\"condition\":\"columnEq\",\"column\":\"revision\",\"value\":\"r123\"}}\n");
    expectRead("{\"response\": \"listRevisions\", \"filter\":{\"condition\":\"columnEq\",\"column\":\"revision\",\"value\":\"r123\"}, \"listRevisions\": []}\n");
    std::vector<RevisionMetadata> expected;
    std::vector<RevisionMetadata> res = j->listRevisions(Filter(Expression(FILTER_COLUMN_EQ, "revision", RevisionId(123))));
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
    f.expectWrite("{\"command\":\"kindNames\"}\n");
    f.expectRead("{\"response\": \"kindNames\", \"kindNames\": [\"k1\", \"k2\", \"k3\", \"k4\", \"k5\"]}\n");

    f.expectWrite("{\"command\":\"kindAttributes\",\"kindName\":\"k1\"}\n");
    f.expectRead("{\"kindAttributes\": {}, \"kindName\": \"k1\", \"response\": \"kindAttributes\"}\n");
    f.expectWrite("{\"command\":\"kindAttributes\",\"kindName\":\"k2\"}\n");
    f.expectRead("{\"kindAttributes\": {}, \"kindName\": \"k2\", \"response\": \"kindAttributes\"}\n");
    f.expectWrite("{\"command\":\"kindAttributes\",\"kindName\":\"k3\"}\n");
    f.expectRead("{\"kindAttributes\": {}, \"kindName\": \"k3\", \"response\": \"kindAttributes\"}\n");
    f.expectWrite("{\"command\":\"kindAttributes\",\"kindName\":\"k4\"}\n");
    f.expectRead("{\"kindAttributes\": {\"fancyAttr\": \"int\"}, \"kindName\": \"k4\", \"response\": \"kindAttributes\"}\n");
    f.expectWrite("{\"command\":\"kindAttributes\",\"kindName\":\"k5\"}\n");
    f.expectRead("{\"kindAttributes\": {\"a5\": \"string\"}, \"kindName\": \"k5\", \"response\": \"kindAttributes\"}\n");

}

/** @short Test dataDifference() from JSON */
BOOST_FIXTURE_TEST_CASE(json_dataDifference, JsonApiTestFixtureFailOnStreamThrow)
{
    schemeForDiff(*this);
    expectWrite("{\"command\":\"dataDifference\",\"revisionA\":\"r1\",\"revisionB\":\"r2\"}\n");
    expectRead("{\"response\": \"dataDifference\",\"revisionA\":\"r1\",\"revisionB\":\"r2\", \"dataDifference\": ["
               + exampleJsonDiff +
               "]}\n");
    std::vector<ObjectModification> expected = diffObjects();
    std::vector<ObjectModification> res = j->dataDifference(RevisionId(1), RevisionId(2));
    BOOST_CHECK_EQUAL_COLLECTIONS(res.begin(), res.end(), expected.begin(), expected.end());
    expectEmpty();
}

/** @short Test dataDifferenceInTemporaryChangeset() from JSON */
BOOST_FIXTURE_TEST_CASE(json_dataDifferenceInTemporaryChangeset, JsonApiTestFixtureFailOnStreamThrow)
{
    schemeForDiff(*this);
    expectWrite("{\"command\":\"dataDifferenceInTemporaryChangeset\",\"changeset\":\"tmp666\"}\n");
    expectRead("{\"response\": \"dataDifferenceInTemporaryChangeset\",\"changeset\":\"tmp666\", \"dataDifferenceInTemporaryChangeset\": ["
               + exampleJsonDiff +
               "]}\n");
    std::vector<ObjectModification> expected = diffObjects();
    std::vector<ObjectModification> res = j->dataDifferenceInTemporaryChangeset(TemporaryChangesetId(666));
    BOOST_CHECK_EQUAL_COLLECTIONS(res.begin(), res.end(), expected.begin(), expected.end());
    expectEmpty();
}

/** @short Test applyBatchedChanges() from JSON */
BOOST_FIXTURE_TEST_CASE(json_applyBatchedChanges, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"applyBatchedChanges\",\"modifications\":[" + exampleJsonDiff + "]}\n");
    expectRead("{\"response\": \"applyBatchedChanges\",\"modifications\": [" + exampleJsonDiff + "]}\n");
    j->applyBatchedChanges(diffObjects());
    expectEmpty();
}

/** @short Test that we catch reports of server-side exceptions */
BOOST_FIXTURE_TEST_CASE(json_exceptions, JsonApiTestFixtureFailOnStreamThrow)
{
#define JSON_ERR_TEST(X) \
    expectWrite("{\"command\":\"startChangeset\"}\n"); \
    expectRead("{\"dbException\": {\"type\":\"" #X "\",\"message\":\"x\"}}\n"); \
    BOOST_CHECK_THROW(j->startChangeset(), X); expectEmpty();

    JSON_ERR_TEST(NotFoundError);
    JSON_ERR_TEST(NoChangesetError);
    JSON_ERR_TEST(SqlError);
    JSON_ERR_TEST(ServerError);
#undef JSON_ERR_TEST
}


/** @short Verify correctness of parsing of revisions from JSON */
BOOST_FIXTURE_TEST_CASE(json_revision_parsing_ok, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"c\",\"r\":\"tmp123\"}\n");
    expectRead("{\"response\": \"c\", \"r\": \"tmp3\"}\n");
    TemporaryChangesetId r(123);
    JsonHandlerApiWrapper h(j, "c");
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
        JsonHandlerApiWrapper h(j, "c");
        // Make sure to explicitly request extraction, but also don't request comparison of JSON serialization
        h.write("r", r).extract(&r).valueShouldMatch = false;
        // ...if we didn't use the valueShouldMatch here, the error would get caught much earlier, even before
        // the actual extraction happens.
        try {
            h.work();
            BOOST_ERROR(std::string("Passing '" + s + "' should have thrown an exception."));
        } catch (JsonStructureError &e) {
            // this is actually what we want
            //std::cerr << "OK: " << e.what() << std::endl;
        }
        expectEmpty();
    }
}
