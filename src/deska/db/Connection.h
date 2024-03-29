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

#ifndef DESKA_DB_CONNECTION_H
#define DESKA_DB_CONNECTION_H

#include <boost/noncopyable.hpp>
#include "deska/db/Api.h"

namespace Deska {
namespace Db {

class Connection_p;

/** @short Connection to the Deska database

This class wraps a connection to the Deska database over an ABI-stable interface.  The usual PIMPL idiom (also known as the
d-pointer) is employed to provide access to an underlying Connection_p.  The API we provide at this level is a complete
implementation of the Deska::Db::Api interface.

@see Deska::Db::Api

*/
class Connection: public Api, private boost::noncopyable
{
public:
    /** @short Instantiate a Connection that communicated over a pair of existing file descriptors */
    explicit Connection(const int rfd, const int wfd);

    /** @short Instantiate a Connection that communicates over a child process */
    explicit Connection(const std::vector<std::string> &args);

    virtual ~Connection();

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
        const Identifier &kindName, const boost::optional<Filter> &filter, const boost::optional<RevisionId> &revision = boost::optional<RevisionId>());
    virtual std::map<Identifier, std::map<Identifier, Value> > multipleResolvedObjectData(
        const Identifier &kindName, const boost::optional<Filter> &filter, const boost::optional<RevisionId> &revision = boost::optional<RevisionId>());
    virtual std::map<Identifier, std::pair<boost::optional<Identifier>, Value> > resolvedObjectDataWithOrigin(
            const Identifier &kindName, const Identifier &objectName, const boost::optional<RevisionId> &revision = boost::optional<RevisionId>());
    virtual std::map<Identifier, std::map<Identifier, std::pair<boost::optional<Identifier>, Value> > > multipleResolvedObjectDataWithOrigin(
        const Identifier &kindName, const boost::optional<Filter> &filter, const boost::optional<RevisionId> &revision = boost::optional<RevisionId>());

    // Manipulating objects
    virtual void deleteObject(const Identifier &kindName, const Identifier &objectName);
    virtual void restoreDeletedObject(const Identifier &kindName, const Identifier &objectName);
    virtual Identifier createObject(const Identifier &kindName, const Identifier &objectName);
    virtual void renameObject(const Identifier &kindName, const Identifier &oldObjectName, const Identifier &newObjectName);
    virtual void setAttribute(
        const Identifier &kindName, const Identifier &objectName, const Identifier &attributeName, const Value &attributeData);
    virtual void setAttributeInsert(const Identifier &kindName, const Identifier &objectName, const Identifier &attributeName, const Identifier &attributeData);
    virtual void setAttributeRemove(const Identifier &kindName, const Identifier &objectName, const Identifier &attributeName, const Identifier &attributeData);
    virtual void applyBatchedChanges(const std::vector<ObjectModificationCommand> &modifications);

    // SCM-like operation and transaction control
    virtual TemporaryChangesetId startChangeset();
    virtual RevisionId commitChangeset(const std::string &commitMessage);
    virtual std::vector<PendingChangeset> pendingChangesets(const boost::optional<Filter> &filter=boost::optional<Filter>());
    virtual void resumeChangeset(const TemporaryChangesetId changeset);
    virtual void detachFromCurrentChangeset(const std::string &message);
    virtual void abortCurrentChangeset();
    virtual void lockCurrentChangeset();
    virtual void unlockCurrentChangeset();
    virtual void freezeView();
    virtual void unFreezeView();
    virtual RevisionId restoringCommit(const std::string &commitMessage, const std::string &author, const boost::posix_time::ptime &timestamp);

    // Diffing
    virtual std::vector<RevisionMetadata> listRevisions(const boost::optional<Filter> &filter=boost::optional<Filter>()) const;
    virtual std::vector<ObjectModificationResult> dataDifference(const RevisionId revisionA, const RevisionId revisionB, const boost::optional<Filter> &filter=boost::optional<Filter>()) const;
    virtual std::vector<ObjectModificationResult> resolvedDataDifference(const RevisionId revisionA, const RevisionId revisionB, const boost::optional<Filter> &filter=boost::optional<Filter>()) const;
    virtual std::vector<ObjectModificationResult> dataDifferenceInTemporaryChangeset(const TemporaryChangesetId changeset, const boost::optional<Filter> &filter=boost::optional<Filter>()) const;
    virtual std::vector<ObjectModificationResult> resolvedDataDifferenceInTemporaryChangeset(const TemporaryChangesetId changeset, const boost::optional<Filter> &filter=boost::optional<Filter>()) const;

    // Output config runners
    virtual std::string showConfigDiff(const ConfigGeneratingMode forceRegenerate=MAYBE_REGENERATE);

    /** @short Set a policy for delayed/immediate sending of commands

    Setting the policy to a lower level of caching will also make sure that at least thsoe commands which cannot be cached in the
    new level are flushed immediately.
    */
    virtual void setCommandBatching(const CommandBatchingMode mode);
private:
    Connection_p *p;
};

}
}

#endif // DESKA_DB_CONNECTION_H
