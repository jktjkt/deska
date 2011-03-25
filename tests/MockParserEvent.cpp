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

#include <iostream>
#include "MockParserEvent.h"

MockParserEvent MockParserEvent::categoryEntered(const Deska::Db::Identifier &kind, const Deska::Db::Identifier &name)
{
    MockParserEvent res(EVENT_ENTER_CONTEXT);
    res.i1 = kind;
    res.i2 = name;
    return res;
}

MockParserEvent MockParserEvent::categoryLeft()
{
    return MockParserEvent(EVENT_LEAVE_CONTEXT);
}

MockParserEvent MockParserEvent::setAttr(const Deska::Db::Identifier &name, const Deska::Db::Value &val)
{
    MockParserEvent res(EVENT_SET_ATTR);
    res.i1 = name;
    res.v1 = val;
    return res;
}

MockParserEvent MockParserEvent::invalid()
{
    return MockParserEvent(EVENT_INVALID);
}

MockParserEvent MockParserEvent::parserError(const Deska::CLI::ParserException &err)
{
    MockParserEvent res(EVENT_PARSE_ERROR);
    res.message = err.dump();
    return res;
}

bool MockParserEvent::operator==(const MockParserEvent &other) const
{
    return eventKind == other.eventKind && i1 == other.i1 && i2 == other.i2 && v1 == other.v1 && message == other.message;
}


MockParserEvent::MockParserEvent(Event e): eventKind(e)
{
}

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
    case MockParserEvent::EVENT_PARSE_ERROR:
        out << "parseError(" << m.message << ")";
        break;
    case MockParserEvent::EVENT_INVALID:
        out << "[no event]";
        break;
    }
    return out;
}
