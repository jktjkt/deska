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

#ifndef DESKA_JSONAPI_H
#define DESKA_JSONAPI_H

#include <stdexcept>
#include <boost/signals2/signal.hpp>
#include <boost/signals2/last_value.hpp>
#include "Api.h"
#include "JsonException.h"
#include "json_spirit/json_spirit_value.h"
#include "libebt/libebt_backtraceable.hh"

namespace Deska {
namespace Db {

/** @short Database API implemented through the JSON */
class JsonApiParser: public Api
{
public:
    JsonApiParser();
    virtual ~JsonApiParser();

    // Querying schema definition
    virtual std::vector<Identifier> kindNames() const;
    virtual std::vector<KindAttributeDataType> kindAttributes(const Identifier &kindName) const;
    virtual std::vector<ObjectRelation> kindRelations( const Identifier &kindName ) const;

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
    virtual void deleteObject( const Identifier &kindName, const Identifier &objectName );
    virtual void restoreDeletedObject(const Identifier &kindName, const Identifier &objectName);
    virtual Identifier createObject( const Identifier &kindName, const Identifier &objectName );
    virtual void renameObject( const Identifier &kindName, const Identifier &oldObjectName, const Identifier &newObjectName );
    virtual void setAttribute(
        const Identifier &kindName, const Identifier &objectName, const Identifier &attributeName, const Value &attributeData );
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

    // Configuration output generators
    virtual std::string showConfigDiff(const ConfigGeneratingMode forceRegenerate=MAYBE_REGENERATE);

    /** @short Request stream for reading JSON data */
    boost::signals2::signal<std::istream *(), boost::signals2::last_value<std::istream*> > willRead;
    /** @short Request stream for writing JSON data */
    boost::signals2::signal<std::ostream *(), boost::signals2::last_value<std::ostream*> > willWrite;
    /** @short Request a copy of data someone just read from the stream

    It turns out that it is very hard to get access to data someone just retrieved from a std::istream instance
    over its iterators; we weren't able to safely do that via boost's multi_pass iterator due to the risk of blocking.
    Therefore, this signal will be called "ex-post" when the JSON parsing (which uses iterators for reading from the
    stream) fails for some reason. The return value of this signal (if usable) will be used to provide better error
    messages when the JSON parsing/handling fails for some reason.
    */
    boost::signals2::signal<std::string ()> wantJustReadData;

private:
    /** @short The implementation wants to send a JSON object */
    void sendJsonObject(const json_spirit::Object &o) const;
    /** @short The implementation tries to obtain the JSON data */
    json_spirit::Object readJsonObject() const;

    friend class JsonHandlerApiWrapper;
};

}
}

#endif // DESKA_JSONAPI_H
