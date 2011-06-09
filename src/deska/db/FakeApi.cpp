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
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include "FakeApi.h"

using std::map;
using std::pair;
using std::string;
using std::vector;

namespace Deska {
namespace Db {

FakeApi::FakeApi()
{
}



FakeApi::~FakeApi()
{
}



vector<Identifier> FakeApi::kindNames() const
{
    using namespace boost::phoenix;
    using namespace arg_names;
    vector<Identifier> res;
    transform(attrs.begin(), attrs.end(), back_inserter(res),
               bind(&map<string, vector<KindAttributeDataType> >::value_type::first, arg1)
              );
    return res;
}



vector<KindAttributeDataType> FakeApi::kindAttributes(const Identifier &kindName) const
{
    map<string, vector<KindAttributeDataType> >::const_iterator it = attrs.find(kindName);
    if (it == attrs.end())
        return vector<KindAttributeDataType>();
    else
        return it->second;
}



vector<ObjectRelation> FakeApi::kindRelations(const Identifier &kindName) const
{
    map<string, vector<ObjectRelation> >::const_iterator it = relations.find(kindName);
    if (it == relations.end())
        return vector<ObjectRelation>();
    else
        return it->second;
}



vector<Identifier> FakeApi::kindInstances(const Identifier &kindName, const boost::optional<Filter> &filter, const RevisionId) const
{
    vector<Identifier> empty;
    return empty;
}



map<Identifier, Value> FakeApi::objectData(const Identifier &kindName, const Identifier &objectName, const RevisionId)
{
    map<Identifier, Value> empty;
    return empty;
}

std::map<Identifier, std::map<Identifier, Value> > FakeApi::multipleObjectData(
        const Identifier &kindName, const Filter &filter, const RevisionId)
{
    return map<Identifier, map<Identifier, Value> >();
}

map<Identifier, pair<Identifier, Value> > FakeApi::resolvedObjectData(const Identifier &kindName,
                                                                      const Identifier &objectName, const RevisionId)
{
    map<Identifier, pair<Identifier, Value> > empty;
    return empty;
}

std::map<Identifier, std::map<Identifier, std::pair<Identifier, Value> > > FakeApi::multipleResolvedObjectData(
    const Identifier &kindName, const Filter &filter, const RevisionId)
{
    return map<Identifier, map<Identifier, pair<Identifier, Value> > >();
}


void FakeApi::deleteObject(const Identifier &kindName, const Identifier &objectName)
{
}

void FakeApi::restoreDeletedObject(const Identifier &kindName, const Identifier &objectName)
{
}


void FakeApi::createObject(const Identifier &kindName, const Identifier &objectName)
{
}



void FakeApi::renameObject(const Identifier &kindName, const Identifier &oldObjectName, const Identifier &newObjectName)
{
}



void FakeApi::setAttribute(const Identifier &kindName, const Identifier &objectName, const Identifier &attributeName,
                           const Value &attributeData)
{
}

void FakeApi::applyBatchedChanges(const std::vector<ObjectModification> &modifications)
{
}

TemporaryChangesetId FakeApi::startChangeset()
{
    return TemporaryChangesetId::null;
}



RevisionId FakeApi::commitChangeset(const std::string &commitMessage)
{
    return RevisionId::null;
}



void FakeApi::rebaseChangeset(const RevisionId parentRevision)
{
}

std::vector<PendingChangeset> FakeApi::pendingChangesets(const boost::optional<Filter> &filter)
{
    return std::vector<PendingChangeset>();
}

void FakeApi::resumeChangeset(const TemporaryChangesetId changeset)
{
}

void FakeApi::detachFromCurrentChangeset(const std::string &message)
{
}

void FakeApi::abortCurrentChangeset()
{
}

std::vector<RevisionMetadata> FakeApi::listRevisions(const boost::optional<Filter> &filter) const
{
    return std::vector<RevisionMetadata>();
}

std::vector<ObjectModification> FakeApi::dataDifference(const RevisionId revisionA, const RevisionId revisionB, const boost::optional<Filter> &filter) const
{
    return std::vector<ObjectModification>();
}

std::vector<ObjectModification> FakeApi::resolvedDataDifference(const RevisionId revisionA, const RevisionId revisionB, const boost::optional<Filter> &filter) const
{
    return std::vector<ObjectModification>();
}

std::vector<ObjectModification> FakeApi::dataDifferenceInTemporaryChangeset(const TemporaryChangesetId changeset, const boost::optional<Filter> &filter) const
{
    return std::vector<ObjectModification>();
}

std::vector<ObjectModification> FakeApi::resolvedDataDifferenceInTemporaryChangeset(const TemporaryChangesetId changeset, const boost::optional<Filter> &filter) const
{
    return std::vector<ObjectModification>();
}

}
}
