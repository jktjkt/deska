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

#ifndef DESKA_TEST_MOCKPARSEREVENT_H
#define DESKA_TEST_MOCKPARSEREVENT_H

#include <iosfwd>
#include "deska/cli/Exceptions.h"
#include "deska/db/Api.h"

/** @short Helper class representing a signal emitted by the Parser being tested */
class MockParserEvent
{
public:
    /** @short The categoryEntered() signal */
    static MockParserEvent categoryEntered(const Deska::Identifier &kind, const Deska::Identifier &name);

    /** @short The categoryLeft() signal */
    static MockParserEvent categoryLeft();

    /** @short The setAttr() signal */
    static MockParserEvent setAttr(const Deska::Identifier &name, const Deska::Value &val);

    /** @short An empty event for debug printing */
    static MockParserEvent invalid();

    /** @short Parser error */
    static MockParserEvent parserError(const Deska::CLI::ParserException &err);

    bool operator==(const MockParserEvent &other) const;

    friend std::ostream& operator<<(std::ostream &out, const MockParserEvent &m);

private:
    typedef enum {
        /** @short Handler for the categoryEntered() signal */
        EVENT_ENTER_CONTEXT,
        /** @short handler for categoryLeft() */
        EVENT_LEAVE_CONTEXT,
        /** @short Handler for setAttribute() */
        EVENT_SET_ATTR,
        /** @short Handler for parseError() */
        EVENT_PARSE_ERROR,
        /** @short Fake, invalid event */
        EVENT_INVALID
    } Event;

    MockParserEvent(Event e);

    Event eventKind;
    Deska::Identifier i1, i2;
    Deska::Value v1;
    Deska::CLI::ParserExceptionPtr e;
};

std::ostream& operator<<(std::ostream &out, const MockParserEvent &m);

#endif // DESKA_TEST_MOCKPARSEREVENT_H
