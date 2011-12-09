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

#ifndef DESKA_DB_CACHINGAPI_H
#define DESKA_DB_CACHINGAPI_H

#include <boost/noncopyable.hpp>
#include "JsonApi.h"

namespace Deska {
namespace Db {

class FakeApi;

/** @short API implementation that caches metadata queries and groups related modifications together

This implementation of the DB API will ask for the metadata (the DB scheme) only once, upon the first request, and then
use that information to answer eventual further questions.  In addition to that, functions for performing modifications
might be delayed before they get forwarded to the real API implementation for execution.

WARNING: users are responsible to change the command caching mode to SEND_IMMEDIATELY before they proceed to use any "interesting"
methods like diffing, commits etc etc.  This is a limitation which might be lifted in future versions (see Redmine #404 for progress).

In future, the feature set of this class could be extended to remember other stuff, like lists of object names etc etc, but none
of these is implemented at this point.

It looks like one can't create stuff like CachingApi<JsonApiParser> p( [JsonApiParser's constructor arguments go here] ),
so we have to stick with having having this "decorator" implemented as a dumb class setting the type of the base class in
stone.
*/
class CachingJsonApi: public JsonApiParser, boost::noncopyable
{
public:
    CachingJsonApi();
    virtual ~CachingJsonApi();

    // Querying schema definition
    virtual std::vector<Identifier> kindNames() const;
    virtual std::vector<KindAttributeDataType> kindAttributes( const Identifier &kindName ) const;
    virtual std::vector<ObjectRelation> kindRelations( const Identifier &kindName ) const;

    virtual void deleteObject(const Identifier &kindName, const Identifier &objectName);
    virtual void restoreDeletedObject(const Identifier &kindName, const Identifier &objectName);
    virtual Identifier createObject(const Identifier &kindName, const Identifier &objectName);
    virtual void renameObject(const Identifier &kindName, const Identifier &oldObjectName, const Identifier &newObjectName);
    virtual void setAttribute(const Identifier &kindName, const Identifier &objectName, const Identifier &attributeName, const Value &attributeData);
    virtual void setAttributeInsert(const Identifier &kindName, const Identifier &objectName, const Identifier &attributeName, const Identifier &attributeData);
    virtual void setAttributeRemove(const Identifier &kindName, const Identifier &objectName, const Identifier &attributeName, const Identifier &attributeData);
    virtual void applyBatchedChanges(const std::vector<ObjectModificationCommand> &modifications);

    // Other functions are not overriden

    /** @short Set a policy for delayed/immediate sending of commands

    Setting the policy to a lower level of caching will also make sure that at least thsoe commands which cannot be cached in the
    new level are flushed immediately.
    */
    void setCommandBatching(const CommandBatchingMode mode);

private:
    /** @short Ask the real underlying database for the metadata */
    void askForMetadata() const;

    /** @short Immediately flush all commands which have been queued */
    void flushCommandsNow();

    /** @short Is the previous operation targetted at the same object as this one? */
    bool isPreviousSameObjectAs(const Identifier &kindName, const Identifier &objectName);

    /** @short Storage for values which were already retrieved */
    FakeApi *m_cache;

    /** @short Did we ask for the metadata already? */
    mutable bool m_askedForMetadata;

    /** @short List of modifications which are waiting for completion */
    std::vector<ObjectModificationCommand> m_pendingModifications;

    /** @short Are we delaying some commands? */
    CommandBatchingMode m_commandBatching;
};

}
}

#endif // DESKA_DB_CACHINGAPI_H
