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
#include <boost/optional.hpp>

#include "deska/cli/Exceptions.h"
#include "deska/db/Objects.h"
#include "deska/db/Filter.h"

/** @short Helper class representing a signal emitted by the Parser being tested */
class MockParserEvent
{
public:
    /** @short The createObject() signal */
    static MockParserEvent createObject(const Deska::Db::Identifier &kind, const Deska::Db::Identifier &name);

    /** @short The categoryEntered() signal */
    static MockParserEvent categoryEntered(const Deska::Db::Identifier &kind, const Deska::Db::Identifier &name);

    /** @short The categoryLeft() signal */
    static MockParserEvent categoryLeft();

    /** @short The setAttr() signal */
    static MockParserEvent setAttr(const Deska::Db::Identifier &kind, const Deska::Db::Identifier &name, const Deska::Db::Value &val);

    /** @short The setAttrInsert() signal */
    static MockParserEvent setAttrInsert(const Deska::Db::Identifier &kind, const Deska::Db::Identifier &name, const Deska::Db::Identifier &val);

    /** @short The setAttrRemove() signal */
    static MockParserEvent setAttrRemove(const Deska::Db::Identifier &kind, const Deska::Db::Identifier &name, const Deska::Db::Identifier &val);
    
    /** @short The removeAttr() signal */
    static MockParserEvent removeAttr(const Deska::Db::Identifier &kind, const Deska::Db::Identifier &name);

    /** @short The functionShow() signal */
    static MockParserEvent functionShow();
    
    /** @short The functionDelete() signal */
    static MockParserEvent functionDelete();

    /** @short The functionRename() signal */
    static MockParserEvent functionRename(const Deska::Db::Identifier &newName);

    /** @short The objectsFilter() signal */
    static MockParserEvent objectsFilter(const Deska::Db::Identifier &name, const Deska::Db::Filter &filter);

    /** @short Parser error */
    static MockParserEvent parserError(const Deska::Cli::ParserException &err);
    
    /** @short The parsingFinished() signal */
    static MockParserEvent parsingFinished();

    /** @short The parsingStarted() signal */
    static MockParserEvent parsingStarted();
    
    /** @short An empty event for debug printing */
    static MockParserEvent invalid();

    bool operator==(const MockParserEvent &other) const;

    friend std::ostream& operator<<(std::ostream &out, const MockParserEvent &m);

private:
    typedef enum {
        /** @short Handler for the createObject() signal */
        EVENT_CREATE_OBJECT,
        /** @short Handler for categoryEntered() */
        EVENT_ENTER_CONTEXT,
        /** @short Handler for categoryLeft() */
        EVENT_LEAVE_CONTEXT,
        /** @short Handler for setAttribute() */
        EVENT_SET_ATTR,
        /** @short Handler for setAttributeInsert() */
        EVENT_SET_ATTR_INSERT,
        /** @short Handler for setAttributeRemove() */
        EVENT_SET_ATTR_REMOVE,
        /** @short Handler for removeAttribute() */
        EVENT_REMOVE_ATTR,
        /** @short Handler for functionShow() */
        EVENT_FUNCTION_SHOW,
        /** @short Handler for functionDelete() */
        EVENT_FUNCTION_DELETE,
        /** @short Handler for functionRename() */
        EVENT_FUNCTION_RENAME,
        /** @short Handler for objectsFilter() */
        EVENT_OBJECTS_FILTER,
        /** @short Handler for parseError() */
        EVENT_PARSE_ERROR,
        /** @short Handler for parsingFinished() */
        EVENT_PARSING_FINISHED,
        /** @short Handler for parsingStarted() */
        EVENT_PARSING_STARTED,
        /** @short Fake, invalid event */
        EVENT_INVALID
    } Event;

    MockParserEvent(Event e);

    Event eventKind;
    Deska::Db::Identifier i1, i2;
    Deska::Db::Value v1;
    boost::optional<Deska::Db::Filter> f1;
    std::string message;
};

std::ostream& operator<<(std::ostream &out, const MockParserEvent &m);

#endif // DESKA_TEST_MOCKPARSEREVENT_H
