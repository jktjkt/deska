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

#ifndef DESKA_FAKEAPI_H
#define DESKA_FAKEAPI_H

#include "Api.h"

namespace Deska {
namespace Db {

/** @short A fake implementation of the DB API for testing purposes */
class FakeApi: public Api
{
public:
    FakeApi();
    virtual ~FakeApi();

    // Querying schema definition
    virtual std::vector<Identifier> kindNames() const;
    virtual std::vector<KindAttributeDataType> kindAttributes(const Identifier &kindName) const;
    virtual std::vector<ObjectRelation> kindRelations(const Identifier &kindName) const;

    // Returning data for existing objects
    virtual std::vector<Identifier> kindInstances(const Identifier &kindName, const boost::optional<Filter> &filter=boost::optional<Filter>(),
                                                  const boost::optional<RevisionId> &revision = boost::optional<RevisionId>()) const;
    virtual std::map<Identifier, Value> objectData(
        const Identifier &kindName, const Identifier &objectName, const boost::optional<RevisionId> &revision = boost::optional<RevisionId>());
    virtual std::map<Identifier, Value> resolvedObjectData(
        const Identifier &kindName, const Identifier &objectName, const boost::optional<RevisionId> &revision = boost::optional<RevisionId>());
    virtual std::map<Identifier, std::map<Identifier, Value> > multipleObjectData(
        const Identifier &kindName, const Filter &filter, const boost::optional<RevisionId> &revision = boost::optional<RevisionId>());
    virtual std::map<Identifier, std::map<Identifier, Value> > multipleResolvedObjectData(
        const Identifier &kindName, const Filter &filter, const boost::optional<RevisionId> &revision = boost::optional<RevisionId>());
    virtual std::map<Identifier, std::pair<Identifier, Value> > resolvedObjectDataWithOrigin(
            const Identifier &kindName, const Identifier &objectName, const boost::optional<RevisionId> &revision = boost::optional<RevisionId>());
    virtual std::map<Identifier, std::map<Identifier, std::pair<Identifier, Value> > > multipleResolvedObjectDataWithOrigin(
        const Identifier &kindName, const Filter &filter, const boost::optional<RevisionId> &revision = boost::optional<RevisionId>());

    // Manipulating objects
    virtual void deleteObject(const Identifier &kindName, const Identifier &objectName);
    virtual void restoreDeletedObject(const Identifier &kindName, const Identifier &objectName);
    virtual void createObject(const Identifier &kindName, const Identifier &objectName);
    virtual void renameObject(const Identifier &kindName, const Identifier &oldObjectName, const Identifier &newObjectName);
    virtual void setAttribute(
        const Identifier &kindName, const Identifier &objectName, const Identifier &attributeName, const Value &attributeData);
    virtual void applyBatchedChanges(const std::vector<ObjectModification> &modifications);

    // SCM-like operation and transaction control
    virtual TemporaryChangesetId startChangeset();
    virtual RevisionId commitChangeset(const std::string &commitMessage);
    virtual void rebaseChangeset(const RevisionId parentRevision);
    virtual std::vector<PendingChangeset> pendingChangesets(const boost::optional<Filter> &filter=boost::optional<Filter>());
    virtual void resumeChangeset(const TemporaryChangesetId changeset);
    virtual void detachFromCurrentChangeset(const std::string &message);
    virtual void abortCurrentChangeset();
    virtual void freezeView();
    virtual void unFreezeView();

    // Diffing
    virtual std::vector<RevisionMetadata> listRevisions(const boost::optional<Filter> &filter=boost::optional<Filter>()) const;
    virtual std::vector<ObjectModification> dataDifference(const RevisionId revisionA, const RevisionId revisionB, const boost::optional<Filter> &filter=boost::optional<Filter>()) const;
    virtual std::vector<ObjectModification> resolvedDataDifference(const RevisionId revisionA, const RevisionId revisionB, const boost::optional<Filter> &filter=boost::optional<Filter>()) const;
    virtual std::vector<ObjectModification> dataDifferenceInTemporaryChangeset(const TemporaryChangesetId changeset, const boost::optional<Filter> &filter=boost::optional<Filter>()) const;
    virtual std::vector<ObjectModification> resolvedDataDifferenceInTemporaryChangeset(const TemporaryChangesetId changeset, const boost::optional<Filter> &filter=boost::optional<Filter>()) const;

    virtual std::string showConfigDiff(bool forceRegenerate=false);

    // These members should be accessible for modifications from the test suite

    /** @short Attributes of various top-level objects */
    std::map<std::string, std::vector<KindAttributeDataType> > attrs;

    /** @short Relations, as "retrieved" from the DB scheme */
    std::map<std::string, std::vector<ObjectRelation> > relations;

    // FIXME: add more members for implementing other methods
};

}
}

#endif // DESKA_FAKEAPI_H
