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

#include <algorithm>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/construct.hpp>
#include "FakeApi.h"

using namespace std;

namespace Deska
{

FakeApi::FakeApi()
{
}



FakeApi::~FakeApi()
{
}



vector<Identifier> FakeApi::kindNames() const
{
    using namespace boost::lambda;
    vector<Identifier> res;
    transform( attrs.begin(), attrs.end(), back_inserter(res),
               bind( &map<string, vector<KindAttributeDataType> >::value_type::first, _1 )
               );
    return res;
}



vector<KindAttributeDataType> FakeApi::kindAttributes( const Identifier &kindName ) const
{
    map<string, vector<KindAttributeDataType> >::const_iterator it = attrs.find( kindName );
    if ( it == attrs.end() )
        return vector<KindAttributeDataType>();
    else
        return it->second;
}



vector<ObjectRelation> FakeApi::kindRelations( const Identifier &kindName ) const
{
    map<string, vector<ObjectRelation> >::const_iterator it = relations.find( kindName );
    if ( it == relations.end() )
        return vector<ObjectRelation>();
    else
        return it->second;
}



vector<Identifier> FakeApi::kindInstances( const Identifier &kindName, const Revision ) const
{
    vector<Identifier> empty;
    return empty;
}



map<Identifier, Value> FakeApi::objectData( const Identifier &kindName, const Identifier &objectName, const Revision )
{
    map<Identifier, Value> empty;
    return empty;
}



map<Identifier, pair<Identifier, Value> > FakeApi::resolvedObjectData(const Identifier &kindName,
                                                                      const Identifier &objectName, const Revision)
{
    map<Identifier, pair<Identifier, Value> > empty;
    return empty;
}



vector<Identifier> FakeApi::findOverriddenAttrs(const Identifier &kindName, const Identifier &objectName,
                                                const Identifier &attrName)
{
    vector<Identifier> empty;
    return empty;
}



vector<Identifier> FakeApi::findNonOverriddenAttrs(const Identifier &kindName, const Identifier &objectName,
                                                   const Identifier &attrName)
{
    vector<Identifier> empty;
    return empty;
}



void FakeApi::deleteObject( const Identifier &kindName, const Identifier &objectName )
{
}



void FakeApi::createObject( const Identifier &kindName, const Identifier &objectname )
{
}



void FakeApi::renameObject( const Identifier &kindName, const Identifier &oldName, const Identifier &newName )
{
}

void FakeApi::removeAttribute(const Identifier &kindName, const Identifier &objectName, const Identifier &attributeName)
{
}



void FakeApi::setAttribute(const Identifier &kindName, const Identifier &objectName, const Identifier &attributeName,
                           const Value &value)
{
}



Revision FakeApi::startChangeset()
{
    return 0;
}



Revision FakeApi::commitChangeset()
{
    return 0;
}



Revision FakeApi::rebaseChangeset(const Revision oldRevision)
{
    return 0;
}

std::vector<Revision> FakeApi::pendingChangesetsByMyself()
{
    return std::vector<Revision>();
}

Revision FakeApi::resumeChangeset(const Revision oldRevision)
{
    return 0;
}

void FakeApi::detachFromActiveChangeset()
{
}

void FakeApi::abortChangeset(const Revision rev)
{
}

}
