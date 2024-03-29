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

#include <boost/date_time/posix_time/time_formatters.hpp>
#include <boost/foreach.hpp>
#define BOOST_TEST_MODULE json_api_parser
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include "JsonApiTestFixture.h"
#include "deska/db/JsonApi.h"
#include "deska/db/JsonHandler.h"
#include "deska/db/AdditionalValueStreamOperators.h"
#include "FuzzyDeskaValue.h"

using std::vector;
using std::map;
using std::string;
using namespace Deska::Db;

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
            "\"price\": \"double\", \"ipv4\": \"ipv4address\", \"mac\": \"macaddress\", \"ipv6\": \"ipv6address\", "
            "\"timestamp\": \"timestamp\", \"date\": \"date\", \"flag\": \"bool\""
            "}, \"tag\": \"T\", \"response\": \"kindAttributes\"}\n");
    vector<KindAttributeDataType> expected;
    expected.push_back(KindAttributeDataType("bar", TYPE_INT));
    expected.push_back(KindAttributeDataType("baz", TYPE_IDENTIFIER));
    expected.push_back(KindAttributeDataType("foo", TYPE_STRING));
    expected.push_back(KindAttributeDataType("price", TYPE_DOUBLE));
    expected.push_back(KindAttributeDataType("ipv4", TYPE_IPV4_ADDRESS));
    expected.push_back(KindAttributeDataType("mac", TYPE_MAC_ADDRESS));
    expected.push_back(KindAttributeDataType("ipv6", TYPE_IPV6_ADDRESS));
    expected.push_back(KindAttributeDataType("timestamp", TYPE_TIMESTAMP));
    expected.push_back(KindAttributeDataType("date", TYPE_DATE));
    expected.push_back(KindAttributeDataType("flag", TYPE_BOOL));
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

/** @short Test that the parser throws an exception upon seeing an argument being returned back */
BOOST_FIXTURE_TEST_CASE(json_returned_argument, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"resumeChangeset\",\"tag\":\"T\",\"changeset\":\"tmp123\"}\n");
    expectRead("{\"response\": \"resumeChangeset\", \"tag\":\"T\", \"changeset\": \"tmp123\"}\n");
    BOOST_CHECK_THROW(j->resumeChangeset(TemporaryChangesetId(123)), JsonStructureError);
    expectEmpty();
}

/** @short Test that kindRelations() can fetch data */
BOOST_FIXTURE_TEST_CASE(json_kindRelations, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"kindRelations\",\"tag\":\"T\",\"kindName\":\"identifier\"}\n");
    expectRead("{\"kindRelations\": ["
            "{\"relation\": \"EMBED_INTO\", \"target\": \"hardware\", \"column\": \"machine\"}, "
            "{\"relation\": \"REFERS_TO\", \"target\": \"reference\", \"column\": \"link\"}, "
            "{\"relation\": \"TEMPLATIZED\", \"target\": \"by-which-kind\", \"column\": \"bar\"}, "
            "{\"relation\": \"CONTAINS\", \"target\": \"part\", \"column\": \"ref1\"}, "
            "{\"relation\": \"CONTAINABLE\", \"target\": \"into\", \"column\": \"ref2\"}"
            "], \"response\": \"kindRelations\",\"tag\":\"T\"}\n");
    vector<ObjectRelation> expected;
    expected.push_back(ObjectRelation::embedInto("hardware", "machine"));
    expected.push_back(ObjectRelation::refersTo("reference", "link"));
    expected.push_back(ObjectRelation::templatized("by-which-kind", "bar"));
    expected.push_back(ObjectRelation::contains("part", "ref1"));
    expected.push_back(ObjectRelation::containable("into", "ref2"));
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
    data.push_back(std::make_pair<string,string>("{\"rel\": \"EMBED_INTO\"}", "JSON field 'rel' is not allowed in this context (expecting one of: relation target column)."));
    data.push_back(std::make_pair<string,string>("{\"relation\": \"EMBED_INTO2\", \"target\": \"pwn\", \"column\": \"x\"}", "Invalid relation kind 'EMBED_INTO2'"));
    data.push_back(std::make_pair<string,string>("{\"relation\": \"EMBED_INTO\"}", "Mandatory field 'target' not present in the response"));
    data.push_back(std::make_pair<string,string>("{\"relation\": \"EMBED_INTO\", \"target\": \"x\"}", "Mandatory field 'column' not present in the response"));
    data.push_back(std::make_pair<string,string>("{\"relation\": \"EMBED_INTO\", \"target\": \"hardware\", \"column\": \"bar\", \"foo\": \"bar\"}",
                                                 "JSON field 'foo' is not allowed in this context (expecting one of: relation target column)."));
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
    vector<Identifier> res = j->kindInstances("blah", Filter(AttributeExpression(FILTER_COLUMN_NE, "kind", "attr", Value(std::string("foo")))), RevisionId(666));
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
    std::vector<Filter> subExpressions;
    subExpressions.push_back(AttributeExpression(FILTER_COLUMN_LT, "kind1", "attr1", Value(666)));
    subExpressions.push_back(AttributeExpression(FILTER_COLUMN_GE, "kind1", "attr2", Value(333)));
    vector<Identifier> res = j->kindInstances("blah", Filter(AndFilter(subExpressions)), RevisionId(666));
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
    std::vector<Filter> subExpressions;
    subExpressions.push_back(AttributeExpression(FILTER_COLUMN_GT, "kind1", "attr1", Value(666)));
    subExpressions.push_back(AttributeExpression(FILTER_COLUMN_LE, "kind1", "attr2", Value(333)));
    vector<Identifier> res = j->kindInstances("blah", Filter(OrFilter(subExpressions)), RevisionId(666));
    BOOST_CHECK_EQUAL_COLLECTIONS(res.begin(), res.end(), expected.begin(), expected.end());
    expectEmpty();
}

/** @short Test filter recursion */
BOOST_FIXTURE_TEST_CASE(json_kindInstances_recursive_filter, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"kindInstances\",\"tag\":\"T\",\"kindName\":\"blah\",\"revision\":\"r666\",\"filter\":"
                "{\"operator\":\"or\",\"operands\":["
                    "{\"condition\":\"columnGt\",\"kind\":\"kind1\",\"attribute\":\"attr1\",\"value\":666},"
                    "{\"operator\":\"and\",\"operands\":["
                        "{\"condition\":\"columnLe\",\"kind\":\"kind1\",\"attribute\":\"attr2\",\"value\":333},"
                        "{\"condition\":\"columnLe\",\"kind\":\"kind1\",\"attribute\":\"attr3\",\"value\":333666}"
                    "]}"
                "]}}\n");
    expectRead("{\"kindInstances\": [], \"response\": \"kindInstances\", \"tag\": \"T\"}\n");
    vector<Identifier> expected;
    std::vector<Filter> subExpressions1;
    subExpressions1.push_back(AttributeExpression(FILTER_COLUMN_GT, "kind1", "attr1", Value(666)));
    std::vector<Filter> subExpressions2;
    subExpressions2.push_back(AttributeExpression(FILTER_COLUMN_LE, "kind1", "attr2", Value(333)));
    subExpressions2.push_back(AttributeExpression(FILTER_COLUMN_LE, "kind1", "attr3", Value(333666)));
    subExpressions1.push_back(AndFilter(subExpressions2));
    vector<Identifier> res = j->kindInstances("blah", Filter(OrFilter(subExpressions1)), RevisionId(666));
    BOOST_CHECK_EQUAL_COLLECTIONS(res.begin(), res.end(), expected.begin(), expected.end());
    expectEmpty();
}

/** @short Basic test for objectData() */
BOOST_FIXTURE_TEST_CASE(json_objectData, JsonApiTestFixtureFailOnStreamThrow)
{
    // The JsonApiParser needs to know type information for the individual object kinds
    expectWrite("{\"command\":\"kindAttributes\",\"tag\":\"T\",\"kindName\":\"kk\"}\n");
    expectRead("{\"kindAttributes\": {\"int\": \"int\", \"baz\": \"identifier\", \"foo\": \"string\", \n"
               "\"real\": \"double\", \"price\": \"double\", \"funnyfloat\": \"double\", \"inherit\": \"identifier\", \"anotherKind\": \"identifier\", "
               "\"ipv4\": \"ipv4address\", \"mac\": \"macaddress\", \"ipv6\": \"ipv6address\", \"timestamp\": \"timestamp\", \"date\": \"date\", "
               "\"role\": \"identifier_set\"}, "
               "\"tag\":\"T\", \"response\": \"kindAttributes\"}\n");
    // ... as well as relation information for proper filtering
    expectWrite("{\"command\":\"kindRelations\",\"tag\":\"T\",\"kindName\":\"kk\"}\n");
    expectRead("{\"kindRelations\": ["
               "{\"relation\": \"TEMPLATIZED\", \"target\": \"by-which-kind\", \"column\": \"defaults\"}, "
               "{\"relation\": \"CONTAINS\", \"target\": \"anotherKind\", \"column\": \"anotherKind\"}"
               "], \"tag\":\"T\", \"response\": \"kindRelations\"}\n");

    expectWrite("{\"command\":\"objectData\",\"tag\":\"T\",\"kindName\":\"kk\",\"objectName\":\"oo\",\"revision\":\"r3\"}\n");
    expectRead("{\"tag\":\"T\", \"objectData\": {\"foo\": \"bar\", \"baz\": \"id\", \"int\": 10, \"real\": 100.666, \"price\": 666, \"funnyfloat\": 70.0, "
            "\"ipv4\": \"127.0.0.1\", \"mac\": \"00:16:3e:37:53:2B\", \"ipv6\": \"::1\", \"date\": \"2011-06-20\", \"timestamp\": \"2011-04-07 17:22:33\","
            "\"inherit\": \"bleh\", \"anotherKind\": \"foo_ref\", \"role\": [\"a\", \"b\", \"cc\"]}, \"response\": \"objectData\"}\n");
    map<Identifier,Value> expected;
    expected["foo"] = std::string("bar");
    expected["int"] = 10;
    expected["real"] = 100.666;
    // Yes, check int-to-float comparison here
    expected["price"] = 666.0;
    expected["funnyfloat"] = 70.0;
    expected["baz"] = string("id");
    expected["mac"] = Deska::Db::MacAddress(0x00, 0x16, 0x3e, 0x37, 0x53, 0x2b);
    expected["ipv4"] = boost::asio::ip::address_v4::from_string("127.0.0.1");
    expected["ipv6"] = boost::asio::ip::address_v6::from_string("::1");
    expected["date"] = boost::gregorian::date(2011, 6, 20);
    expected["timestamp"] = boost::posix_time::ptime(boost::gregorian::date(2011, 4, 7), boost::posix_time::time_duration(17, 22, 33));
    expected["inherit"] = string("bleh");
    expected["anotherKind"] = string("foo_ref");
    std::set<Identifier> role;
    role.insert("a");
    role.insert("b");
    role.insert("cc");
    expected["role"] = role;
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
    expectWrite("{\"command\":\"kindAttributes\",\"tag\":\"T\",\"kindName\":\"kk\"}\n");
    expectRead("{\"kindAttributes\": {\"baz\": \"int\", \"foo\": \"string\"}, \n"
               "\"tag\":\"T\", \"response\": \"kindAttributes\"}\n");
    expectWrite("{\"command\":\"kindRelations\",\"tag\":\"T\",\"kindName\":\"kk\"}\n");
    expectRead("{\"response\":\"kindRelations\", \"tag\":\"T\", \"kindRelations\": []}\n");

    expectWrite("{\"command\":\"resolvedObjectData\",\"tag\":\"T\",\"kindName\":\"kk\",\"objectName\":\"oo\"}\n");
    expectRead("{\"resolvedObjectData\": "
            "{\"foo\": \"bar\", \"baz\": 666}, \"tag\":\"T\", \"response\": \"resolvedObjectData\"}\n");
    map<Identifier, Value> expected;
    expected["foo"] = string("bar");
    expected["baz"] = 666;
    map<Identifier, Value> res = j->resolvedObjectData("kk", "oo");
    // In this case, we limit ourselves to string comparisons. There's a map invloved here, which means that
    // BOOST_CHECK_EQUAL_COLLECTIONS is worthless, and there isn't much point in duplicating the whole logic from json_objectData
    // at yet another place. Let's stick with strings and don't expect to see detailed error reporting here.
    BOOST_REQUIRE_EQUAL(res.size(), expected.size());
    BOOST_CHECK(std::equal(res.begin(), res.end(), expected.begin()));
    expectEmpty();
}
/** @short Basic functionality of multipleObjectData() */
BOOST_FIXTURE_TEST_CASE(json_multipleObjectData, JsonApiTestFixtureFailOnStreamThrow)
{
    // The JsonApiParser needs to know type information for the individual object kinds
    expectWrite("{\"command\":\"kindAttributes\",\"tag\":\"T\",\"kindName\":\"kk\"}\n");
    expectRead("{\"kindAttributes\": {\"int\": \"int\", \"baz\": \"identifier\", \"foo\": \"string\", \n"
               "\"template\": \"identifier\", \"anotherKind\": \"identifier\"}, \"tag\":\"T\", \"response\": \"kindAttributes\"}\n");
    // ... as well as relation information for proper filtering
    expectWrite("{\"command\":\"kindRelations\",\"tag\":\"T\",\"kindName\":\"kk\"}\n");
    expectRead("{\"kindRelations\": ["
               "{\"relation\": \"TEMPLATIZED\", \"target\": \"by-which-kind\", \"column\": \"baz\"}, "
               "{\"relation\": \"CONTAINS\", \"target\": \"anotherKind\", \"column\": \"template\"}"
               "],\"tag\":\"T\", \"response\": \"kindRelations\"}\n");

    expectWrite("{\"command\":\"multipleObjectData\",\"tag\":\"T\",\"kindName\":\"kk\",\"filter\":{\"condition\":\"columnNe\",\"kind\":\"kind\",\"attribute\":\"int\",\"value\":666}}\n");
    expectRead("{\"multipleObjectData\": {"
               "\"a\": {\"foo\": \"barA\", \"baz\": \"idA\", \"int\": 10, \"template\": null, \"anotherKind\": \"a\"}, "
               "\"b\": {\"foo\": \"barB\", \"baz\": \"idB\", \"int\": 20, \"template\": null, \"anotherKind\": \"b\"} "
               "}, \"tag\":\"T\", \"response\": \"multipleObjectData\"}\n");
    map<Identifier, map<Identifier,Value> > expected;
    expected["a"]["foo"] = string("barA");
    expected["a"]["int"] = 10;
    expected["a"]["baz"] = string("idA");
    expected["a"]["template"] = Value();
    expected["a"]["anotherKind"] = string("a");
    expected["b"]["foo"] = string("barB");
    expected["b"]["int"] = 20;
    expected["b"]["baz"] = string("idB");
    expected["b"]["template"] = Value();
    expected["b"]["anotherKind"] = string("b");
    // Check just the interesting items
    map<Identifier, map<Identifier,Value> > res = j->multipleObjectData("kk", Filter(AttributeExpression(FILTER_COLUMN_NE, "kind", "int", Value(666))));
    BOOST_CHECK(std::equal(res.begin(), res.end(), expected.begin()));
    expectEmpty();
}

/** @short Basic test for resolvedObjectDataWithOrigin() */
BOOST_FIXTURE_TEST_CASE(json_resolvedObjectDataWithOrigin, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"kindAttributes\",\"tag\":\"T\",\"kindName\":\"kk\"}\n");
    expectRead("{\"kindAttributes\": {\"baz\": \"int\", \"foo\": \"string\", \"smrt\": \"timestamp\"}, \n"
               "\"tag\":\"T\", \"response\": \"kindAttributes\"}\n");
    expectWrite("{\"command\":\"kindRelations\",\"tag\":\"T\",\"kindName\":\"kk\"}\n");
    expectRead("{\"response\":\"kindRelations\", \"tag\":\"T\", \"kindRelations\": []}\n");

    expectWrite("{\"command\":\"resolvedObjectDataWithOrigin\",\"tag\":\"T\",\"kindName\":\"kk\",\"objectName\":\"oo\"}\n");
    expectRead("{\"resolvedObjectDataWithOrigin\": "
               "{\"foo\": [\"obj-defining-this\", \"bar\"], \"baz\": [\"this-obj\", 666], \"smrt\": [null, null]}, \"tag\":\"T\", \"response\": \"resolvedObjectDataWithOrigin\"}\n");
    map<Identifier, std::pair<boost::optional<Identifier>,Value> > expected;
    expected["foo"] = std::make_pair(Identifier("obj-defining-this"), string("bar"));
    expected["baz"] = std::make_pair(Identifier("this-obj"), 666);
    expected["smrt"] = std::make_pair(boost::optional<Identifier>(), Value());
    map<Identifier, std::pair<boost::optional<Identifier>,Value> > res = j->resolvedObjectDataWithOrigin("kk", "oo");
    // In this case, we limit ourselves to string comparisons. There's a map invloved here, which means that
    // BOOST_CHECK_EQUAL_COLLECTIONS is worthless, and there isn't much point in duplicating the whole logic from json_objectData
    // at yet another place. Let's stick with strings and don't expect to see detailed error reporting here.
    BOOST_REQUIRE_EQUAL(res.size(), expected.size());
    BOOST_CHECK(std::equal(res.begin(), res.end(), expected.begin()));
    expectEmpty();
}

/** @short Basic functionality of multipleResolvedObjectDataWithOrigin() */
BOOST_FIXTURE_TEST_CASE(json_multipleResolvedObjectDataWithOrigin, JsonApiTestFixtureFailOnStreamThrow)
{
    // The JsonApiParser needs to know type information for the individual object kinds
    expectWrite("{\"command\":\"kindAttributes\",\"tag\":\"T\",\"kindName\":\"kk\"}\n");
    expectRead("{\"kindAttributes\": {\"int\": \"int\", \"baz\": \"identifier\", \"foo\": \"string\", \n"
               "\"template\": \"identifier\", \"anotherKind\": \"identifier\", \"smrt\": \"identifier_set\"}, "
               "\"tag\":\"T\", \"response\": \"kindAttributes\"}\n");
    // ... as well as relation information for proper filtering
    expectWrite("{\"command\":\"kindRelations\",\"tag\":\"T\",\"kindName\":\"kk\"}\n");
    expectRead("{\"kindRelations\": ["
               "{\"relation\": \"TEMPLATIZED\", \"target\": \"by-which-kind\", \"column\": \"template\"}, "
               "{\"relation\": \"CONTAINS\", \"target\": \"anotherKind\", \"column\": \"anotherKind\"}"
               "], \"tag\":\"T\", \"response\": \"kindRelations\"}\n");

    expectWrite("{\"command\":\"multipleResolvedObjectDataWithOrigin\",\"tag\":\"T\",\"kindName\":\"kk\",\"filter\":{\"condition\":\"columnNe\",\"kind\":\"kind1\",\"attribute\":\"int\",\"value\":666}}\n");
    expectRead("{\"multipleResolvedObjectDataWithOrigin\": {"
               "\"a\": {\"foo\": [\"1\", \"barA\"], \"baz\": [\"1\", \"idA\"], \"int\": [\"11\", 10], \"template\": [\"a\", null], \"anotherKind\": [\"a\", \"a\"], \"smrt\": [null, null]}, "
               "\"b\": {\"foo\": [\"1\", \"barB\"], \"baz\": [\"2\", \"idB\"], \"int\": [\"22\", 20], \"template\": [\"b\", \"22\"], \"anotherKind\": [\"b\", \"b\"], \"smrt\": [null, null]} "
               "}, \"tag\":\"T\", \"response\": \"multipleResolvedObjectDataWithOrigin\"}\n");
    map<Identifier, map<Identifier,std::pair<boost::optional<Identifier>, Value> > > expected;
    expected["a"]["foo"] = std::make_pair(Identifier("1"), string("barA"));
    expected["a"]["baz"] = std::make_pair(Identifier("1"), string("idA"));
    expected["a"]["int"] = std::make_pair(Identifier("11"), 10);
    expected["a"]["template"] = std::make_pair(Identifier("a"), Value());
    expected["a"]["anotherKind"] = std::make_pair(Identifier("a"), string("a"));
    expected["a"]["smrt"] = std::make_pair(boost::optional<Identifier>(), Value());
    expected["b"]["foo"] = std::make_pair(Identifier("1"), string("barB"));
    expected["b"]["baz"] = std::make_pair(Identifier("2"), string("idB"));
    expected["b"]["int"] = std::make_pair(Identifier("22"), 20);
    expected["b"]["template"] = std::make_pair(Identifier("b"), string("22"));
    expected["b"]["anotherKind"] = std::make_pair(Identifier("b"), string("b"));
    expected["b"]["smrt"] = std::make_pair(boost::optional<Identifier>(), Value());
    map<Identifier, map<Identifier, std::pair<boost::optional<Identifier>, Value> > > res = j->multipleResolvedObjectDataWithOrigin("kk", Filter(AttributeExpression(FILTER_COLUMN_NE, "kind1", "int", Value(666))));
    BOOST_CHECK(std::equal(res.begin(), res.end(), expected.begin()));
    expectEmpty();
}

/** @short Basic functionality of multipleResolvedObjectData() */
BOOST_FIXTURE_TEST_CASE(json_multipleResolvedObjectData, JsonApiTestFixtureFailOnStreamThrow)
{
    // The JsonApiParser needs to know type information for the individual object kinds
    expectWrite("{\"command\":\"kindAttributes\",\"tag\":\"T\",\"kindName\":\"kk\"}\n");
    expectRead("{\"kindAttributes\": {\"int\": \"int\", \"baz\": \"identifier\", \"foo\": \"string\", \n"
               "\"template\": \"identifier\", \"anotherKind\": \"identifier\"}, "
               "\"tag\":\"T\", \"response\": \"kindAttributes\"}\n");
    // ... as well as relation information for proper filtering
    expectWrite("{\"command\":\"kindRelations\",\"tag\":\"T\",\"kindName\":\"kk\"}\n");
    expectRead("{\"kindRelations\": ["
               "{\"relation\": \"TEMPLATIZED\", \"target\": \"by-which-kind\", \"column\": \"template\"}, "
               "{\"relation\": \"CONTAINS\", \"target\": \"anotherKind\", \"column\": \"anotherKind\"}"
               "], \"tag\":\"T\", \"response\": \"kindRelations\"}\n");

    expectWrite("{\"command\":\"multipleResolvedObjectData\",\"tag\":\"T\",\"kindName\":\"kk\",\"filter\":{\"condition\":\"columnNe\",\"kind\":\"kind1\",\"attribute\":\"int\",\"value\":666}}\n");
    expectRead("{\"multipleResolvedObjectData\": {"
               "\"a\": {\"foo\": \"barA\", \"baz\": \"idA\", \"int\": 10, \"template\": null, \"anotherKind\": \"a\"}, "
               "\"b\": {\"foo\": \"barB\", \"baz\": \"idB\", \"int\": 20, \"template\": \"22\", \"anotherKind\": \"b\"} "
               "}, \"tag\":\"T\", \"response\": \"multipleResolvedObjectData\"}\n");
    map<Identifier, map<Identifier,Value> > expected;
    expected["a"]["foo"] = string("barA");
    expected["a"]["baz"] = string("idA");
    expected["a"]["int"] = 10;
    expected["a"]["template"] = Value();
    expected["a"]["anotherKind"] = string("a");
    expected["b"]["foo"] = string("barB");
    expected["b"]["baz"] = string("idB");
    expected["b"]["int"] = 20;
    expected["b"]["template"] = string("22");
    expected["b"]["anotherKind"] = string("b");
    map<Identifier, map<Identifier, Value> > res = j->multipleResolvedObjectData("kk", Filter(AttributeExpression(FILTER_COLUMN_NE, "kind1", "int", Value(666))));
    BOOST_CHECK(std::equal(res.begin(), res.end(), expected.begin()));
    expectEmpty();
}

/** @short Basic test for createObject() */
BOOST_FIXTURE_TEST_CASE(json_createObject, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"createObject\",\"tag\":\"T\",\"kindName\":\"k\",\"objectName\":\"o\"}\n");
    expectRead("{\"response\": \"createObject\",\"tag\":\"T\",\"createObject\":\"blah\"}\n");
    BOOST_CHECK_EQUAL(j->createObject("k", "o"), "blah");
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
    // string or identifier
    data.push_back(SetAttrTestData(jsonInputPrefix + "\"some string\"}\n", jsonOutput, Deska::Db::Value(string("some string"))));
    // set of identifiers
    std::set<std::string> stringset;
    stringset.insert("first");
    stringset.insert("second");
    data.push_back(SetAttrTestData(jsonInputPrefix + "[\"first\",\"second\"]}\n", jsonOutput, Deska::Db::Value(stringset)));
    // int
    data.push_back(SetAttrTestData(jsonInputPrefix + "123}\n", jsonOutput, Deska::Db::Value(123)));
    // bool
    data.push_back(SetAttrTestData(jsonInputPrefix + "true}\n", jsonOutput, Deska::Db::Value(true)));
    data.push_back(SetAttrTestData(jsonInputPrefix + "false}\n", jsonOutput, Deska::Db::Value(false)));
    // double
    data.push_back(SetAttrTestData(jsonInputPrefix + "333.666}\n", jsonOutput, Deska::Db::Value(333.666)));
    data.push_back(SetAttrTestData(jsonInputPrefix + "70.0}\n", jsonOutput, Deska::Db::Value(70.0)));
    data.push_back(SetAttrTestData(jsonInputPrefix + "0.0}\n", jsonOutput, Deska::Db::Value(0.0)));
    data.push_back(SetAttrTestData(jsonInputPrefix + "-1.0}\n", jsonOutput, Deska::Db::Value(-1.0)));
    // null
    data.push_back(SetAttrTestData(jsonInputPrefix + "null}\n", jsonOutput, Deska::Db::Value()));
    // IPv4 address
    data.push_back(SetAttrTestData(jsonInputPrefix + "\"203.0.113.66\"}\n", jsonOutput,
                                   Deska::Db::Value(boost::asio::ip::address_v4::from_string("203.0.113.66"))));
    // IPv6 address
    data.push_back(SetAttrTestData(jsonInputPrefix + "\"2001:db8:666:333::\"}\n", jsonOutput,
                                   Deska::Db::Value(boost::asio::ip::address_v6::from_string("2001:DB8:666:333::"))));
    // Ethernet MAC address
    data.push_back(SetAttrTestData(jsonInputPrefix + "\"06:00:00:00:00:0a\"}\n", jsonOutput,
                                   Deska::Db::Value(MacAddress(6, 0, 0, 0, 0, 0x0a))));
    // date
    data.push_back(SetAttrTestData(jsonInputPrefix + "\"2011-08-09\"}\n", jsonOutput,
                                   Deska::Db::Value(boost::gregorian::date(2011, 8, 9))));
    // timestamp
    data.push_back(SetAttrTestData(jsonInputPrefix + "\"2011-04-07 17:22:33\"}\n", jsonOutput,
                                   Deska::Db::Value(boost::posix_time::ptime(boost::gregorian::date(2011, 4, 7), boost::posix_time::time_duration(17, 22, 33)))));

    BOOST_FOREACH(const SetAttrTestData &value, data) {
        expectWrite(value.jsonIn);
        expectRead(value.jsonOut);
        j->setAttribute("k", "o", "a", value.v);
        expectEmpty();
    }
}

/** @short Basic test for the setAttributeInsert() */
BOOST_FIXTURE_TEST_CASE(json_setAttributeInsert, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"setAttributeInsert\",\"tag\":\"T\",\"kindName\":\"k\",\"objectName\":\"o\",\"attributeName\":\"a\",\"attributeData\":\"123\"}\n");
    expectRead("{\"response\": \"setAttributeInsert\", \"tag\":\"T\"}\n");
    j->setAttributeInsert("k", "o", "a", "123");
    expectEmpty();
}

/** @short Basic test for the setAttributeRemove() */
BOOST_FIXTURE_TEST_CASE(json_setAttributeRemove, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"setAttributeRemove\",\"tag\":\"T\",\"kindName\":\"k\",\"objectName\":\"o\",\"attributeName\":\"a\",\"attributeData\":\"123\"}\n");
    expectRead("{\"response\": \"setAttributeRemove\", \"tag\":\"T\"}\n");
    j->setAttributeRemove("k", "o", "a", "123");
    expectEmpty();
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

/** @short Basic test for restoringCommit() */
BOOST_FIXTURE_TEST_CASE(json_restoringCommit, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"restoringCommit\",\"tag\":\"T\",\"commitMessage\":\"description\",\"author\":\"user\",\"timestamp\":\"2011-04-07 17:22:33\"}\n");
    expectRead("{\"response\": \"restoringCommit\", \"tag\":\"T\", \"restoringCommit\": \"r333\"}\n");
    BOOST_CHECK_EQUAL(j->restoringCommit("description", "user", boost::posix_time::ptime(boost::gregorian::date(2011, 4, 7), boost::posix_time::time_duration(17, 22, 33))), RevisionId(333));
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
    expectRead("{\"response\": \"resumeChangeset\", \"tag\":\"T\"}\n");
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

/** @short Basic test for lockCurrentChangeset() */
BOOST_FIXTURE_TEST_CASE(json_lockCurrentChangeset, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"lockCurrentChangeset\",\"tag\":\"T\"}\n");
    expectRead("{\"response\": \"lockCurrentChangeset\",\"tag\":\"T\"}\n");
    j->lockCurrentChangeset();
    expectEmpty();
}

/** @short Basic test for unlockCurrentChangeset() */
BOOST_FIXTURE_TEST_CASE(json_unlockCurrentChangeset, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"unlockCurrentChangeset\",\"tag\":\"T\"}\n");
    expectRead("{\"response\": \"unlockCurrentChangeset\",\"tag\":\"T\"}\n");
    j->unlockCurrentChangeset();
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

std::vector<ObjectModificationResult> diffObjects()
{
    std::vector<ObjectModificationResult> res;
    res.push_back(CreateObjectModification("k1", "o1"));
    res.push_back(DeleteObjectModification("k2", "o2"));
    res.push_back(RenameObjectModification("k3", "ooooold", "new"));
    res.push_back(SetAttributeModification("k5", "o5", "a5", Deska::Db::Value(string("new")), Deska::Db::Value(string("old"))));
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
    std::vector<ObjectModificationResult> expected = diffObjects();
    std::vector<ObjectModificationResult> res = j->dataDifference(RevisionId(1), RevisionId(2),
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
    std::vector<ObjectModificationResult> expected = diffObjects();
    std::vector<ObjectModificationResult> res = j->dataDifferenceInTemporaryChangeset(TemporaryChangesetId(666));
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
               + exampleJsonDiff + "]}\n");
    std::vector<ObjectModificationResult> expected = diffObjects();
    std::vector<ObjectModificationResult> res = j->resolvedDataDifference(RevisionId(1), RevisionId(2),
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
    std::vector<ObjectModificationResult> expected = diffObjects();
    std::vector<ObjectModificationResult> res = j->resolvedDataDifferenceInTemporaryChangeset(
                TemporaryChangesetId(1), Filter(AttributeExpression(FILTER_COLUMN_EQ, "kind1", "attr1", Value())));
    BOOST_CHECK_EQUAL_COLLECTIONS(res.begin(), res.end(), expected.begin(), expected.end());
    expectEmpty();
}



/** @short Test applyBatchedChanges() from JSON */
BOOST_FIXTURE_TEST_CASE(json_applyBatchedChanges, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"applyBatchedChanges\",\"tag\":\"T\",\"modifications\":[" +
                exampleJsonDiff +
                ",{\"command\":\"setAttribute\",\"kindName\":\"k6\",\"objectName\":\"o6\",\"attributeName\":\"a6\",\"attributeData\":\"new6\"}" +
                ",{\"command\":\"setAttribute\",\"kindName\":\"k7\",\"objectName\":\"o7\",\"attributeName\":\"a7\",\"attributeData\":\"new7\"}" +
                ",{\"command\":\"setAttributeInsert\",\"kindName\":\"k8\",\"objectName\":\"o8\",\"attributeName\":\"a8\",\"attributeData\":\"666\"}" +
                ",{\"command\":\"setAttributeRemove\",\"kindName\":\"k9\",\"objectName\":\"o9\",\"attributeName\":\"a9\",\"attributeData\":\"333\"}" +
                "]}\n");
    expectRead("{\"response\": \"applyBatchedChanges\", \"tag\":\"T\"}\n");
    std::vector<ObjectModificationResult> diff = diffObjects();
    std::vector<ObjectModificationCommand> modifications(diff.begin(), diff.end());
    modifications.push_back(SetAttributeModification("k6", "o6", "a6", Deska::Db::Value(string("new6"))));
    modifications.push_back(SetAttributeModification("k7", "o7", "a7", Deska::Db::Value(string("new7")), Deska::Db::Value()));
    modifications.push_back(SetAttributeInsertModification("k8", "o8", "a8", "666"));
    modifications.push_back(SetAttributeRemoveModification("k9", "o9", "a9", "333"));
    j->applyBatchedChanges(modifications);
    expectEmpty();
}

/** @short Test that showConfigDiff() returns a reasonable result */
BOOST_FIXTURE_TEST_CASE(json_showConfigDiff, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"showConfigDiff\",\"tag\":\"T\"}\n");
    expectRead("{\"showConfigDiff\": \"blabla\", \"tag\":\"T\", \"response\": \"showConfigDiff\"}\n");
    std::string expected = "blabla";
    std::string res = j->showConfigDiff();
    BOOST_CHECK_EQUAL(res, expected);
    expectEmpty();
}

/** @short Test that showConfigDiff() with a forced regen returns a reasonable result */
BOOST_FIXTURE_TEST_CASE(json_showConfigDiff_regen, JsonApiTestFixtureFailOnStreamThrow)
{
    expectWrite("{\"command\":\"showConfigDiff\",\"tag\":\"T\",\"forceRegenerate\":true}\n");
    expectRead("{\"showConfigDiff\": \"foo bar\", \"tag\":\"T\", \"response\": \"showConfigDiff\"}\n");
    std::string expected = "foo bar";
    std::string res = j->showConfigDiff(Deska::Db::Api::FORCE_REGENERATE);
    BOOST_CHECK_EQUAL(res, expected);
    expectEmpty();
}


/** @short Test that we catch reports of server-side exceptions */
BOOST_FIXTURE_TEST_CASE(json_exceptions, JsonApiTestFixtureFailOnStreamThrow)
{
#define JSON_ERR_TEST(X) \
    expectWrite("{\"command\":\"startChangeset\",\"tag\":\"T\"}\n"); \
    expectRead("{\"dbException\": {\"type\":\"" #X "\",\"message\":\"x\"}}\n"); \
    BOOST_CHECK_THROW(j->startChangeset(), X); expectEmpty();

    JSON_ERR_TEST(InvalidKindError);
    JSON_ERR_TEST(InvalidAttributeError);
    JSON_ERR_TEST(NotFoundError);
    JSON_ERR_TEST(NoChangesetError);
    JSON_ERR_TEST(ChangesetAlreadyOpenError);
    JSON_ERR_TEST(FreezingError);
    JSON_ERR_TEST(FilterError);
    JSON_ERR_TEST(ReCreateObjectError);
    JSON_ERR_TEST(AlreadyExistsError);
    JSON_ERR_TEST(RevisionParsingError);
    JSON_ERR_TEST(RevisionRangeError);
    JSON_ERR_TEST(ChangesetParsingError);
    JSON_ERR_TEST(ChangesetRangeError);
    JSON_ERR_TEST(ConstraintError);
    JSON_ERR_TEST(ObsoleteParentError);
    JSON_ERR_TEST(NotASetError);
    JSON_ERR_TEST(ChangesetLockingError);
    JSON_ERR_TEST(CfgGeneratingError);
    JSON_ERR_TEST(SpecialReadOnlyAttributeError);
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

