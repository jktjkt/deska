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
#include <boost/signals2/shared_connection_block.hpp>
#include <boost/test/unit_test.hpp>
#include "ParserTestFixture.h"


/** @short Verify that the mock object setup works properly

In this test case, we check our mock facility for obvious errors.
*/
BOOST_FIXTURE_TEST_CASE( test_mock_objects, F )
{
    // There's no parser, so we have to skip testing of the proper nesting
    boost::signals2::shared_connection_block(attrCheckContextConnection);

    // At first, nothing should be present in there
    expectNothingElse();

    // Simulate slots triggered by the Parser
    slotParserCategoryEntered("a", "b");
    slotParserSetAttr("foo", "bar");
    slotParserCategoryLeft();

    // ...and verify that we indeed received them
    expectCategoryEntered("a", "b");
    expectSetAttr("foo", "bar");
    expectCategoryLeft();

    // The shouldn't be anything else at this point
    expectNothingElse();
    verifyEmptyStack();
}
