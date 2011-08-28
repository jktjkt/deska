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

MockParserEvent MockParserEvent::createObject(const Deska::Db::Identifier &kind, const Deska::Db::Identifier &name)
{
    MockParserEvent res(EVENT_CREATE_OBJECT);
    res.i1 = kind;
    res.i2 = name;
    return res;
}

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

MockParserEvent MockParserEvent::setAttr(const Deska::Db::Identifier &kind, const Deska::Db::Identifier &name, const Deska::Db::Value &val)
{
    MockParserEvent res(EVENT_SET_ATTR);
    res.i1 = kind;
    res.i2 = name;
    res.v1 = val;
    return res;
}

MockParserEvent MockParserEvent::setAttrInsert(const Deska::Db::Identifier &kind, const Deska::Db::Identifier &name, const Deska::Db::Identifier &val)
{
    MockParserEvent res(EVENT_SET_ATTR_INSERT);
    res.i1 = kind;
    res.i2 = name;
    res.i2 = val;
    return res;
}

MockParserEvent MockParserEvent::setAttrRemove(const Deska::Db::Identifier &kind, const Deska::Db::Identifier &name, const Deska::Db::Identifier &val)
{
    MockParserEvent res(EVENT_SET_ATTR_REMOVE);
    res.i1 = kind;
    res.i2 = name;
    res.i2 = val;
    return res;
}

MockParserEvent MockParserEvent::removeAttr(const Deska::Db::Identifier &kind, const Deska::Db::Identifier &name)
{
    MockParserEvent res(EVENT_REMOVE_ATTR);
    res.i1 = kind;
    res.i2 = name;
    return res;
}

MockParserEvent MockParserEvent::functionShow()
{
    return MockParserEvent(EVENT_FUNCTION_SHOW);
}

MockParserEvent MockParserEvent::functionDelete()
{
    return MockParserEvent(EVENT_FUNCTION_DELETE);
}

MockParserEvent MockParserEvent::functionRename(const Deska::Db::Identifier &newName)
{
    MockParserEvent res(EVENT_FUNCTION_RENAME);
    res.i1 = newName;
    return res;
}

MockParserEvent MockParserEvent::objectsFilter(const Deska::Db::Identifier &name, const Deska::Db::Filter &filter)
{
    MockParserEvent res(EVENT_OBJECTS_FILTER);
    res.i1 = name;
    res.f1 = filter;
    return res;
}

MockParserEvent MockParserEvent::parserError(const Deska::Cli::ParserException &err)
{
    MockParserEvent res(EVENT_PARSE_ERROR);
    res.message = err.dump();
    return res;
}

MockParserEvent MockParserEvent::parsingFinished()
{
    return MockParserEvent(EVENT_PARSING_FINISHED);
}

MockParserEvent MockParserEvent::parsingStarted()
{
    return MockParserEvent(EVENT_PARSING_STARTED);
}

MockParserEvent MockParserEvent::invalid()
{
    return MockParserEvent(EVENT_INVALID);
}

bool MockParserEvent::operator==(const MockParserEvent &other) const
{
    if (!(eventKind == other.eventKind && i1 == other.i1 && i2 == other.i2 && v1 == other.v1 && message == other.message))
        return false;

    if ((f1) && (other.f1))
        return *(f1) == *(other.f1);
    else if (!(f1) && !(other.f1))
        return true;
    else
        return false;
}


MockParserEvent::MockParserEvent(Event e): eventKind(e)
{
}

std::ostream& operator<<(std::ostream &out, const MockParserEvent &m)
{
    switch (m.eventKind) {
    case MockParserEvent::EVENT_CREATE_OBJECT:
        out << "createObject( " << m.i1 << ", " << m.i2 << " )";
        break;
    case MockParserEvent::EVENT_ENTER_CONTEXT:
        out << "categoryEntered( " << m.i1 << ", " << m.i2 << " )";
        break;
    case MockParserEvent::EVENT_LEAVE_CONTEXT:
        out << "categoryLeft()";
        break;
    case MockParserEvent::EVENT_SET_ATTR:
        out << "setAttr( " << m.i1 << ", " << m.i2 << ", " << *(m.v1) << " )";
        break;
    case MockParserEvent::EVENT_SET_ATTR_INSERT:
        out << "setAttrInsert( " << m.i1 << ", " << m.i2 << ", " << m.i2 << " )";
        break;
    case MockParserEvent::EVENT_SET_ATTR_REMOVE:
        out << "setAttrRemove( " << m.i1 << ", " << m.i2 << ", " << m.i2 << " )";
        break;
    case MockParserEvent::EVENT_REMOVE_ATTR:
        out << "removeAttr( " << m.i1 << ", " << m.i2 << " )";
        break;
    case MockParserEvent::EVENT_FUNCTION_SHOW:
        out << "functionShow()";
        break;
    case MockParserEvent::EVENT_FUNCTION_DELETE:
        out << "functionDelete()";
        break;
    case MockParserEvent::EVENT_FUNCTION_RENAME:
        out << "functionRename( " << m.i1 << " )";
        break;
    case MockParserEvent::EVENT_OBJECTS_FILTER:
        out << "objectsFilter( " << m.i1 << ", " << *(m.f1) << " )";
        break;
    case MockParserEvent::EVENT_PARSE_ERROR:
        out << "parseError(" << m.message << ")";
        break;
    case MockParserEvent::EVENT_PARSING_FINISHED:
        out << "parsingFinished()";
        break;
    case MockParserEvent::EVENT_PARSING_STARTED:
        out << "parsingStarted()";
        break;
    case MockParserEvent::EVENT_INVALID:
        out << "[no event]";
        break;
    }
    return out;
}
