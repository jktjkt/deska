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
}

/** @short Test stream dumping of the filters */
BOOST_AUTO_TEST_CASE(filter_dumping)
{
    MetadataExpression e1(FILTER_COLUMN_EQ, "revision", RevisionId(123));
    MetadataExpression e2(FILTER_COLUMN_NE, "changeset", TemporaryChangesetId(666));
    MetadataExpression e3(FILTER_COLUMN_LT, "state", PendingChangeset::ATTACH_DETACHED);
    MetadataExpression e4(FILTER_COLUMN_LE, "author", Value("foo"));
    AttributeExpression e5(FILTER_COLUMN_GT, "kind", "attr", Value(123));
    verifyFilterObject("MetadataExpression(revision == MetadataValue<RevisionId>(r123))", e1);
    verifyFilterObject("MetadataExpression(changeset != MetadataValue<TemporaryChangesetId>(tmp666))", e2);
    verifyFilterObject("MetadataExpression(state < MetadataValue<PendingChangeset::AttachStatus>(DETACHED))", e3);
    verifyFilterObject("MetadataExpression(author <= MetadataValue<Value>(Value<string>(foo)))", e4);
    verifyFilterObject("AttributeExpression(kind.attr > Value<int>(123))", e5);
}
