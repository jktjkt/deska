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
#define BOOST_TEST_MODULE api_filters
#include <boost/test/unit_test.hpp>
#include "deska/db/Filter.h"
#include "deska/db/AdditionalValueStreamOperators.h"

using namespace std;
using namespace Deska::Db;

template<typename T>
void verifyFilterObject(const std::string &str, const T &value)
{
    std::ostringstream ss;
    ss << value;
    BOOST_CHECK_EQUAL(str, ss.str());

    ss.clear();
    ss.str("");
    ss << Filter(value);
    BOOST_CHECK_EQUAL(str, ss.str());
    BOOST_CHECK_EQUAL(Filter(value), Filter(value));
}

/** @short Test stream dumping of the filters */
BOOST_AUTO_TEST_CASE(filter_dumping)
{
    // We don't put these items into a liast because we really want to test without casting to a wrapping filter
    MetadataExpression e1(FILTER_COLUMN_EQ, "revision", RevisionId(123));
    string s1 = "MetadataExpression(revision == MetadataValue<RevisionId>(r123))";
    MetadataExpression e2(FILTER_COLUMN_NE, "changeset", TemporaryChangesetId(666));
    string s2 = "MetadataExpression(changeset != MetadataValue<TemporaryChangesetId>(tmp666))";
    MetadataExpression e3(FILTER_COLUMN_LT, "state", PendingChangeset::ATTACH_DETACHED);
    string s3 = "MetadataExpression(state < MetadataValue<PendingChangeset::AttachStatus>(DETACHED))";
    MetadataExpression e4(FILTER_COLUMN_LE, "author", Value("foo"));
    string s4 = "MetadataExpression(author <= MetadataValue<Value>(Value<string>(foo)))";
    AttributeExpression e5(FILTER_COLUMN_GT, "kind", "attr", Value(123));
    string s5 = "AttributeExpression(kind.attr > Value<int>(123))";
    vector<Filter> v1;
    v1.push_back(e1);
    v1.push_back(e2);
    AndFilter e6(v1);
    string s6 = "AndFilter([" + s1 + ", " + s2 + ", ])";
    vector<Filter> v2;
    v2.push_back(e3);
    v2.push_back(e4);
    OrFilter e7(v2);
    string s7 = "OrFilter([" + s3 + ", " + s4 + ", ])";
    vector<Filter> v3;
    v3.push_back(e5);
    v3.push_back(e6);
    v3.push_back(e7);
    AndFilter e8(v3);
    string s8 = "AndFilter([" + s5 + ", " + s6 + ", " + s7 + ", ])";

    verifyFilterObject(s1, e1);
    verifyFilterObject(s2, e2);
    verifyFilterObject(s3, e3);
    verifyFilterObject(s4, e4);
    verifyFilterObject(s5, e5);
    verifyFilterObject(s6, e6);
    verifyFilterObject(s7, e7);
    verifyFilterObject(s8, e8);
}
