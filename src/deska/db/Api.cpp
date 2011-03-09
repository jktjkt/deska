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
#include "Api.h"

using namespace std;

namespace Deska
{

Api::~Api()
{
}

ObjectRelation::ObjectRelation(const ObjectRelationKind _kind, const Identifier &_targetTableName, const Identifier &_sourceAttribute):
    kind(_kind), targetTableName(_targetTableName), sourceAttribute(_sourceAttribute)
{
}

ObjectRelation ObjectRelation::mergeWith(const Identifier &targetTableName, const Identifier &sourceAttribute)
{
    ObjectRelation res;
    res.kind = RELATION_MERGE_WITH;
    res.targetTableName = targetTableName;
    res.sourceAttribute = sourceAttribute;
    return res;
}

ObjectRelation ObjectRelation::embedInto(const Identifier &into)
{
    ObjectRelation res;
    res.kind = RELATION_EMBED_INTO;
    res.targetTableName = into;
    return res;
}

ObjectRelation ObjectRelation::isTemplate(const Identifier &toWhichKind)
{
    ObjectRelation res;
    res.kind = RELATION_IS_TEMPLATE;
    res.targetTableName = toWhichKind;
    return res;
}

ObjectRelation ObjectRelation::templatized(const Identifier &byWhichKind, const Identifier &sourceAttribute)
{
    ObjectRelation res;
    res.kind = RELATION_TEMPLATIZED;
    res.targetTableName = byWhichKind;
    res.sourceAttribute = sourceAttribute;
    return res;
}

/** @short Private constructor for creating a half-baked object

This is very much needed for ObjectRelation::embedInto.
*/
ObjectRelation::ObjectRelation()
{
}

}
