/*
* Copyright (C) 2011 Jan Kundrát <kundratj@fzu.cz>
* Copyright (C) 2011 Tomáš Hubík <hubik.tomas@gmail.com>
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

#ifndef DESKA_DB_INTERACTION_H
#define DESKA_DB_INTERACTION_H

#include <vector>
#include <boost/noncopyable.hpp>
#include "CliObjects.h"
#include "deska/db/Revisions.h"
#include "deska/db/ObjectModification.h"
#include "ContextStack.h"

namespace Deska {

namespace Db {
class Api;
}

namespace Cli {


/** @short Class used for communication with the database to provide all necessary functions to the user interface.
*
*   @see UserInterface
*/
class DbInteraction: public boost::noncopyable
{
public:

    /** @short Constructor only initializes pointer to the API.
    *
    *   @param api Pointer to the api for communication with the DB
    */
    DbInteraction(Db::Api *api);

    /** @short Creates new object.
    *
    *   @param context Path and object definition to create
    *   @return ContextStackItem identifying really created object or set of objects
    */
    ContextStackItem createObject(const ContextStack &context);
    /** @short Restores deleted object.
    *
    *   @param context Path and object definition to restore
    */
    void restoreDeletedObject(const ContextStack &context);
    /** @short Deletes object.
    *
    *   @param context Path and object definition to delete
    */
    void deleteObject(const ContextStack &context);
    /** @short Renames object.
    *
    *   @param context Path and object definition to delete
    *   @param newName New name of the object
    */
    void renameObject(const ContextStack &context, const Db::Identifier &newName);
    /** @short Sets attribute value in the object.
    *
    *   @param context Path to the object which attribute will be changed
    *   @param attribute Attribute and value to set
    */
    void setAttribute(const ContextStack &context, const AttributeDefinition &attribute);
    /** @short Removes attribute value in the object.
    *
    *   @param context Path to the object which attribute value will be removed
    *   @param attribute Attribute to remove value from
    */
    void removeAttribute(const ContextStack &context, const Db::Identifier &attribute);

    /** @short Obtains list of all defined kind names.
    *
    *   @return Vector of all kind names
    */
    std::vector<Db::Identifier> kindNames();

    /** @short Obtains list of top-level kinds.
    *
    *   @return Vector of kinds that are ambedded in any object.
    */
    std::vector<Db::Identifier> topLevelKinds();

    /** @short Obtains list of instances of given kind
    *
    *   @param kindName Kind for which the instances are obtained
    *   @return Vector of all instances of the kind
    */
    std::vector<ObjectDefinition> kindInstances(const Db::Identifier &kindName);

    /** @short Obtains all attributes of given object.
    *
    *   @param object Object for which the attributes are obtained
    *   @return Vector of all attributes
    */
    std::vector<AttributeDefinition> allAttributes(const ObjectDefinition &object);

    /** @short Obtains all attributes with resolved values from templates of given object with origins.
    *
    *   @param context Path to the object for which the attributes are obtained
    *   @return Vector of pairs with all attributes and their origins
    */
    std::vector<std::pair<AttributeDefinition, Db::Identifier> > allAttributesResolvedWithOrigin(
        const ContextStack &context);

    /** @short Obtains all attributes with resolved values from templates of given object with origins.
    *
    *   @param object Object for which the attributes are obtained
    *   @return Vector of pairs with all attributes and their origins
    */
    std::vector<std::pair<AttributeDefinition, Db::Identifier> > allAttributesResolvedWithOrigin(
        const ObjectDefinition &object);

    /** @short Obtains all attributes of given object.
    *
    *   @param context Path to the object for which the attributes are obtained
    *   @return Vector of all attributes
    */
    std::vector<AttributeDefinition> allAttributes(const ContextStack &context);

    /** @short Obtains all attributes of given object.
    *
    *   @param object Object for which the nested kinds are obtained
    *   @return Vector of all nested kinds when there is some context, else list of top-level objects
    */
    std::vector<ObjectDefinition> allNestedObjects(const ObjectDefinition &object);

    /** @short Obtains all attributes of given object.
    *
    *   @param context Path to the object for which the nested kinds are obtained
    *   @return Vector of all nested kinds when there is some context, else list of top-level objects
    */
    std::vector<ObjectDefinition> allNestedObjects(const ContextStack &context);

    /** @short Check if object exists or not
    *
    *   @param object The object to search for
    *   @return True if object exists else false
    */
    bool objectExists(const ObjectDefinition &object);

    /** @short Check if object in the context exists or not
    *
    *   @param context Path to the object to search for
    *   @return True if object exists else false
    */
    bool objectExists(const ContextStack &context);

    /** @short Function for obtaining all pending chandesets.
    *
    *   @return Vector of all pending changesets
    */
    std::vector<Db::PendingChangeset> allPendingChangesets();
    /** @short Function for creating and connecting to the new changeset.
    *
    *   @return Temporary id of created changeset
    */
    Db::TemporaryChangesetId createNewChangeset();
    /** @short Connects to some existing changeset.
    *
    *   @param changesetId Id of changeset to connect to
    */
    void resumeChangeset(const Db::TemporaryChangesetId &changesetId);
    /** @short Commits current changeset
    *
    *   @param message Commit message
    */
    void commitChangeset(const std::string &message);
    /** @short Detaches from current changeset
    *
    *   @param message Detach message
    */
    void detachFromChangeset(const std::string &message);
    /** @short Aborts current changeset. */
    void abortChangeset();

    /** Function for obtaining all revisions.
    *
    *   @return Vector of all revisions.
    */
    std::vector<Db::RevisionMetadata> allRevisions();

    /** Function for obtaining revisions matching specified filter.
    *
    *   @param filter Filter specifying which revisions will be obtained.
    *   @return Vector of revisions.
    */
    std::vector<Db::RevisionMetadata> filteredRevisions(const Db::Filter &filter);

    /** @short Returns differences between two revisions
    *
    *   @param revisionA First revision.
    *   @param revisionA Second revision.
    *   @return Vector of modifications how to get from first revision to second revision.
    */
    std::vector<Db::ObjectModification> revisionsDifference(const Db::RevisionId &revisionA, const Db::RevisionId &revisionB);

    /** @short Returns differences between temporary changeset and its parent
    *
    *   @param changeset Temporary changeset.
    *   @return Vector of modifications how to get from first revision to second revision.
    */
    std::vector<Db::ObjectModification> revisionsDifferenceChangeset(const Db::TemporaryChangesetId &changeset);

    /** @short Function for obtaining parent for given changeset ID
    *
    *   @param changeset TemporaryChangesetId for which the parent will be obtained.
    *   @return parent RevisionId
    */
    Db::RevisionId changesetParent(const Db::TemporaryChangesetId &changeset);

    /** @short Obtains string containing the human readable difference in the generated configuration,
    *          as determined by changes in the current changeset
    *
    *   @param forceRegenerate Forces regeneration of the configurtation
    */
    std::string configDiff(bool forceRegenerate = false);

    /** @short Expands context stack into a vector of objects
    *
    *   When context stack does not contain any filter, result will be only one object, but in case of filters,
    *   all objects that are matched by the sequence of filters and kindNames are extracted into the vector.
    *
    *   @param context Context stack to expand.
    *   @return Vector of objects with fully qualified names.
    */
    std::vector<ObjectDefinition> expandContextStack(const ContextStack &context);

private:

    /** Identifiers of top level kinds. */
    std::vector<Db::Identifier> pureTopLevelKinds;
    /** Identifiers of all kinds. */
    std::vector<Db::Identifier> allKinds;
    /** Map of kind names and kinds, where is the kind embedded. */
    std::map<Db::Identifier, Db::Identifier> embeddedInto;
    /** Map of kinds and vector of kinds, that are embedded in the kind. */
    std::map<Db::Identifier, std::vector<Db::Identifier> > embeds;

    /** Pointer to the api for communication with the DB. */
    Db::Api *m_api;
};


}
}

#endif  // DESKA_DB_INTERACTION_H
