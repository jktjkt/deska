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

#include <queue>
#define BOOST_TEST_MODULE example
#include <boost/test/unit_test.hpp>
#include <boost/signals/trackable.hpp>
#include <boost/bind.hpp>

#include "deska/db/FakeApi.h"
#include "deska/cli/Parser.h"

/** @short Helper class representing a signal emitted by the Parser being tested */
class MockParserEvent
{
public:
    /** @short The categoryEntered() signal */
    static MockParserEvent categoryEntered(const Deska::Identifier &kind, const Deska::Identifier &name)
    {
        MockParserEvent res(EVENT_ENTER_CONTEXT);
        res.i1 = kind;
        res.i2 = name;
        return res;
    }

    /** @short The categoryLeft() signal */
    static MockParserEvent categoryLeft()
    {
        return MockParserEvent(EVENT_LEAVE_CONTEXT);
    }

    /** @short The setAttr() signal */
    static MockParserEvent setAttr(const Deska::Identifier &name, const Deska::Value &val)
    {
        MockParserEvent res(EVENT_SET_ATTR);
        res.i1 = name;
        res.v1 = val;
        return res;
    }

    /** @short An empty event for debug printing */
    static MockParserEvent invalid()
    {
        return MockParserEvent(EVENT_INVALID);
    }

    bool operator==(const MockParserEvent &other) const
    {
        return eventKind == other.eventKind && i1 == other.i1 && i2 == other.i2 && v1 == other.v1;
    }

    friend std::ostream& operator<<(std::ostream &out, const MockParserEvent &m);

private:
    typedef enum {
        /** @short Handler for the categoryEntered() signal */
        EVENT_ENTER_CONTEXT,
        /** @short handler for categoryLeft() */
        EVENT_LEAVE_CONTEXT,
        /** @short Handler for setAttribute() */
        EVENT_SET_ATTR,
        /** @short Fake, invalid event */
        EVENT_INVALID
    } Event;

    MockParserEvent(Event e): eventKind(e) {}

    Event eventKind;
    Deska::Identifier i1, i2;
    Deska::Value v1;
};

std::ostream& operator<<(std::ostream &out, const MockParserEvent &m)
{
    switch (m.eventKind) {
    case MockParserEvent::EVENT_ENTER_CONTEXT:
        out << "categoryEntered( " << m.i1 << ", " << m.i2 << " )";
        break;
    case MockParserEvent::EVENT_LEAVE_CONTEXT:
        out << "categoryLeft()";
        break;
    case MockParserEvent::EVENT_SET_ATTR:
        out << "setAttr( " << m.i1 << ", " << m.v1 << " )";
        break;
    case MockParserEvent::EVENT_INVALID:
        out << "[no event]";
        break;
    }
    return out;
}

struct F: public boost::signals::trackable
{
    F()
    {
        using namespace Deska;
        Deska::FakeApi *fake = new FakeApi();
        fake->attrs["hardware"].push_back( KindAttributeDataType( "id", "integer" ) );
        fake->attrs["hardware"].push_back( KindAttributeDataType( "name", "string" ) );
        fake->attrs["hardware"].push_back( KindAttributeDataType( "price", "integer" ) );
        fake->attrs["interface"].push_back( KindAttributeDataType( "ip", "string" ) );
        fake->attrs["interface"].push_back( KindAttributeDataType( "mac", "string" ) );
        fake->attrs["host"].push_back( KindAttributeDataType( "hardware", "identifier" ) );
        fake->attrs["host"].push_back( KindAttributeDataType( "name", "string" ) );

        fake->relations["interface"].push_back( ObjectRelation::embedInto("host") );
        db = fake;
    }
    ~F()
    {
        delete db;
    }

    /** @short Connect Parser's signals to slots in F */
    void connectSignalsFromParser(Deska::CLI::Parser &parser)
    {
        parser.categoryEntered.connect(boost::bind(&F::slotParserCategoryEntered, this, _1, _2));
        parser.categoryLeft.connect(boost::bind(&F::slotParserCategoryLeft, this));
        parser.attributeSet.connect(boost::bind(&F::slotParserSetAttr, this, _1, _2));
    }

    /** @short Handler for Parser's categoryEntered signal */
    void slotParserCategoryEntered(const Deska::Identifier &kind, const Deska::Identifier &name)
    {
        parserEvents.push(MockParserEvent::categoryEntered(kind, name));
    }

    /** @short Handler for Parser's categoryLeft() */
    void slotParserCategoryLeft()
    {
        parserEvents.push(MockParserEvent::categoryLeft());
    }

    /** @short Handler for Parser's setAttr() signal */
    void slotParserSetAttr(const Deska::Identifier &name, const Deska::Value &val)
    {
        parserEvents.push(MockParserEvent::setAttr(name, val));
    }

    /** @short Call this function to verify that no more events were logged */
    void expectNothingElse()
    {
        BOOST_CHECK_MESSAGE(parserEvents.empty(), "Expected no more emitted signals");
    }

    /** @short Verify that the first signal which wasn't checked yet was the categoryEntered and that its argument match */
    void expectCategoryEntered(const Deska::Identifier &kind, const Deska::Identifier &name)
    {
        expectHelper(MockParserEvent::categoryEntered(kind, name));
    }

    /** @short Check for categoryLeft

    @see expectCategoryEntered()
    */
    void expectCategoryLeft()
    {
        expectHelper(MockParserEvent::categoryLeft());
    }

    /** @short Check that the first signal which was not checked yet was the setAttr()

    @see expectCategoryEntered();
    */
    void expectSetAttr(const Deska::Identifier &name, const Deska::Value &val)
    {
        expectHelper(MockParserEvent::setAttr(name, val));
    }

    /** @short Helper for various expect* functions */
    void expectHelper(const MockParserEvent &e)
    {
        // We would like to continue with the test suite after hitting the first error, and
        // BOOST_REQUIRE doesn't allow masking via BOOST_AUTO_TEST_CASE_EXPECTED_FAILURES...
        BOOST_CHECK( ! parserEvents.empty() );
        bool shouldPop = ! parserEvents.empty();
        MockParserEvent other = shouldPop ? parserEvents.front() : MockParserEvent::invalid();
        BOOST_CHECK_EQUAL(e, other);
        if ( shouldPop )
            parserEvents.pop();
    }

    Deska::Api *db;
    std::queue<MockParserEvent> parserEvents;
};


/** @short Verify that the mock object setup works properly

In this test case, we check our mock facility for obvious errors.
*/
BOOST_FIXTURE_TEST_CASE( test_mock_objects, F )
{
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
}

BOOST_AUTO_TEST_CASE_EXPECTED_FAILURES(parsing_top_level_objects, 4)
BOOST_FIXTURE_TEST_CASE( parsing_top_level_objects, F )
{
    Deska::CLI::Parser parser(db);
    connectSignalsFromParser(parser);

    // start a new context
    parser.parseLine("hardware \"hov2\"\r\n");
    expectCategoryEntered("hardware", "hpv2");
    expectNothingElse();

    parser.parseLine("end\r\n");
    expectCategoryLeft();
    expectNothingElse();

}
