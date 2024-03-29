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



vector<Identifier> FakeApi::kindInstances(const Identifier &kindName, const boost::optional<Filter> &filter, const boost::optional<RevisionId> &revision) const
{
    vector<Identifier> empty;
    return empty;
}



map<Identifier, Value> FakeApi::objectData(const Identifier &kindName, const Identifier &objectName, const boost::optional<RevisionId> &revision)
{
    throw Deska::Db::NotFoundError("Function objectData does not work in fake API");
}

map<Identifier, Value> FakeApi::resolvedObjectData(const Identifier &kindName, const Identifier &objectName, const boost::optional<RevisionId> &revision)
{
    map<Identifier, Value> empty;
    return empty;
}

std::map<Identifier, std::map<Identifier, Value> > FakeApi::multipleObjectData(
        const Identifier &kindName, const boost::optional<Filter> &filter, const boost::optional<RevisionId> &revision)
{
    return map<Identifier, map<Identifier, Value> >();
}

std::map<Identifier, std::map<Identifier, Value> > FakeApi::multipleResolvedObjectData(
        const Identifier &kindName, const boost::optional<Filter> &filter, const boost::optional<RevisionId> &revision)
{
    return map<Identifier, map<Identifier, Value> >();
}

map<Identifier, pair<boost::optional<Identifier>, Value> > FakeApi::resolvedObjectDataWithOrigin(const Identifier &kindName,
                                                                      const Identifier &objectName, const boost::optional<RevisionId> &revision)
{
    map<Identifier, pair<boost::optional<Identifier>, Value> > empty;
    return empty;
}

std::map<Identifier, std::map<Identifier, std::pair<boost::optional<Identifier>, Value> > > FakeApi::multipleResolvedObjectDataWithOrigin(
    const Identifier &kindName, const boost::optional<Filter> &filter, const boost::optional<RevisionId> &revision)
{
    return map<Identifier, map<Identifier, pair<boost::optional<Identifier>, Value> > >();
}


void FakeApi::deleteObject(const Identifier &kindName, const Identifier &objectName)
{
}

void FakeApi::restoreDeletedObject(const Identifier &kindName, const Identifier &objectName)
{
}


Identifier FakeApi::createObject(const Identifier &kindName, const Identifier &objectName)
{
    return objectName;
}



void FakeApi::renameObject(const Identifier &kindName, const Identifier &oldObjectName, const Identifier &newObjectName)
{
}



void FakeApi::setAttribute(const Identifier &kindName, const Identifier &objectName, const Identifier &attributeName,
                           const Value &attributeData)
{
}

void FakeApi::setAttributeInsert(const Identifier &kindName, const Identifier &objectName, const Identifier &attributeName, const Identifier &attributeData)
{
}

void FakeApi::setAttributeRemove(const Identifier &kindName, const Identifier &objectName, const Identifier &attributeName, const Identifier &attributeData)
{
}

void FakeApi::applyBatchedChanges(const std::vector<ObjectModificationCommand> &modifications)
{
}

TemporaryChangesetId FakeApi::startChangeset()
{
    return TemporaryChangesetId(666);
}



RevisionId FakeApi::commitChangeset(const std::string &commitMessage)
{
    return RevisionId(666);
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

void FakeApi::lockCurrentChangeset()
{
}

void FakeApi::unlockCurrentChangeset()
{
}

void FakeApi::freezeView()
{
}

void FakeApi::unFreezeView()
{
}

RevisionId FakeApi::restoringCommit(const std::string &commitMessage, const std::string &author, const boost::posix_time::ptime &timestamp)
{
    return RevisionId(789);
}

std::vector<RevisionMetadata> FakeApi::listRevisions(const boost::optional<Filter> &filter) const
{
    return std::vector<RevisionMetadata>();
}

std::vector<ObjectModificationResult> FakeApi::dataDifference(const RevisionId revisionA, const RevisionId revisionB, const boost::optional<Filter> &filter) const
{
    return std::vector<ObjectModificationResult>();
}

std::vector<ObjectModificationResult> FakeApi::resolvedDataDifference(const RevisionId revisionA, const RevisionId revisionB, const boost::optional<Filter> &filter) const
{
    return std::vector<ObjectModificationResult>();
}

std::vector<ObjectModificationResult> FakeApi::dataDifferenceInTemporaryChangeset(const TemporaryChangesetId changeset, const boost::optional<Filter> &filter) const
{
    return std::vector<ObjectModificationResult>();
}

std::vector<ObjectModificationResult> FakeApi::resolvedDataDifferenceInTemporaryChangeset(const TemporaryChangesetId changeset, const boost::optional<Filter> &filter) const
{
    return std::vector<ObjectModificationResult>();
}

std::string FakeApi::showConfigDiff(const ConfigGeneratingMode forceRegenerate)
{
    return std::string();
}

}
}
