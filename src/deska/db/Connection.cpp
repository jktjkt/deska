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

#include <boost/spirit/include/phoenix_bind.hpp>
#include <cstdlib>
#include "Connection.h"
#include "Connection_p.h"

namespace Deska {
namespace Db {

Connection::Connection()
{
    p = new Connection_p();
}

Connection::~Connection()
{
    delete p;
}

std::vector<Identifier> Connection::kindNames() const
{
    return p->kindNames();
}

std::vector<KindAttributeDataType> Connection::kindAttributes(const Identifier &kindName) const
{
    return p->kindAttributes(kindName);
}

std::vector<ObjectRelation> Connection::kindRelations(const Identifier &kindName) const
{
    return p->kindRelations(kindName);
}

std::vector<Identifier> Connection::kindInstances(const Identifier &kindName, const boost::optional<Filter> &filter, const boost::optional<RevisionId> &revision) const
{
    return p->kindInstances(kindName, filter, revision);
}

std::map<Identifier, Value> Connection::objectData(const Identifier &kindName, const Identifier &objectName, const boost::optional<RevisionId> &revision)
{
    return p->objectData(kindName, objectName, revision);
}

std::map<Identifier, Value> Connection::resolvedObjectData(const Identifier &kindName, const Identifier &objectName, const boost::optional<RevisionId> &revision)
{
    return p->resolvedObjectData(kindName, objectName, revision);
}

std::map<Identifier, std::map<Identifier, Value> > Connection::multipleObjectData(const Identifier &kindName, const Filter &filter, const boost::optional<RevisionId> &revision)
{
    return p->multipleObjectData(kindName, filter, revision);
}

std::map<Identifier, std::map<Identifier, Value> > Connection::multipleResolvedObjectData(const Identifier &kindName, const Filter &filter, const boost::optional<RevisionId> &revision)
{
    return p->multipleResolvedObjectData(kindName, filter, revision);
}

std::map<Identifier, std::pair<Identifier, Value> > Connection::resolvedObjectDataWithOrigin(const Identifier &kindName, const Identifier &objectName, const boost::optional<RevisionId> &revision)
{
    return p->resolvedObjectDataWithOrigin(kindName, objectName, revision);
}

std::map<Identifier, std::map<Identifier, std::pair<Identifier, Value> > > Connection::multipleResolvedObjectDataWithOrigin(const Identifier &kindName, const Filter &filter, const boost::optional<RevisionId> &revision)
{
    return p->multipleResolvedObjectDataWithOrigin(kindName, filter, revision);
}

void Connection::deleteObject(const Identifier &kindName, const Identifier &objectName)
{
    p->deleteObject(kindName, objectName);
}

void Connection::restoreDeletedObject(const Identifier &kindName, const Identifier &objectName)
{
    p->restoreDeletedObject(kindName, objectName);
}

Identifier Connection::createObject(const Identifier &kindName, const Identifier &objectName)
{
    return p->createObject(kindName, objectName);
}

void Connection::renameObject(const Identifier &kindName, const Identifier &oldObjectName, const Identifier &newObjectName)
{
    p->renameObject(kindName, oldObjectName, newObjectName);
}

void Connection::setAttribute(const Identifier &kindName, const Identifier &objectName, const Identifier &attributeName, const Value &attributeData)
{
    p->setAttribute(kindName, objectName, attributeName, attributeData);
}

void Connection::setAttributeInsert(const Identifier &kindName, const Identifier &objectName, const Identifier &setName, const Identifier &data)
{
    p->setAttributeInsert(kindName, objectName, setName, data);
}

void Connection::setAttributeRemove(const Identifier &kindName, const Identifier &objectName, const Identifier &setName, const Identifier &data)
{
    p->setAttributeRemove(kindName, objectName, setName, data);
}

void Connection::applyBatchedChanges(const std::vector<ObjectModification> &modifications)
{
    p->applyBatchedChanges(modifications);
}

TemporaryChangesetId Connection::startChangeset()
{
    return p->startChangeset();
}

RevisionId Connection::commitChangeset(const std::string &commitMessage)
{
    return p->commitChangeset(commitMessage);
}

std::vector<PendingChangeset> Connection::pendingChangesets(const boost::optional<Filter> &filter)
{
    return p->pendingChangesets(filter);
}

void Connection::resumeChangeset(const TemporaryChangesetId changeset)
{
    return p->resumeChangeset(changeset);
}

void Connection::detachFromCurrentChangeset(const std::string &message)
{
    return p->detachFromCurrentChangeset(message);
}

void Connection::abortCurrentChangeset()
{
    p->abortCurrentChangeset();
}

void Connection::freezeView()
{
    p->freezeView();
}

void Connection::unFreezeView()
{
    p->unFreezeView();
}

std::vector<RevisionMetadata> Connection::listRevisions(const boost::optional<Filter> &filter) const
{
    return p->listRevisions(filter);
}

std::vector<ObjectModification> Connection::dataDifference(const RevisionId revisionA, const RevisionId revisionB, const boost::optional<Filter> &filter) const
{
    return p->dataDifference(revisionA, revisionB, filter);
}

std::vector<ObjectModification> Connection::resolvedDataDifference(const RevisionId revisionA, const RevisionId revisionB, const boost::optional<Filter> &filter) const
{
    return p->resolvedDataDifference(revisionA, revisionB, filter);
}

std::vector<ObjectModification> Connection::dataDifferenceInTemporaryChangeset(const TemporaryChangesetId changeset, const boost::optional<Filter> &filter) const
{
    return p->dataDifferenceInTemporaryChangeset(changeset, filter);
}

std::vector<ObjectModification> Connection::resolvedDataDifferenceInTemporaryChangeset(const TemporaryChangesetId changeset, const boost::optional<Filter> &filter) const
{
    return p->resolvedDataDifferenceInTemporaryChangeset(changeset, filter);
}

std::string Connection::showConfigDiff(bool forceRegenerate)
{
    return p->showConfigDiff(forceRegenerate);
}

}
}
