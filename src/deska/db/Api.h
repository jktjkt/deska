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

#ifndef DESKA_API_H
#define DESKA_API_H

#include <map>
#include <stdexcept>
#include <vector>
#include "libebt/libebt_backtraceable.hh"

#include "deska/db/Objects.h"
#include "deska/db/Revisions.h"
#include "deska/db/ObjectModification.h"

/*
 * TODO items for the DB API:
 *
 * - Think about how to retrieve older revisions from the DB (is the default RevisionId=0
 *   enough/suitable?)
 * - Exceptions -- current idea is that all Deska::Api operations throw an exception upon any error
 * - Reorganize namespace and class names?
 *
 *
 * */


namespace Deska {
namespace Db {

/** @short INTERNAL: tag for the libebt */
struct ApiExceptionTag {};

/** @short INTERNAL: convenience typedef for exception reporting */
typedef libebt::BacktraceContext<ApiExceptionTag> ApiContext;

/** @short Exception occured during processing of the request */
class RemoteDbError: public std::runtime_error
{
public:
    RemoteDbError(const std::string &message);
    virtual ~RemoteDbError() throw ();
};

#define REMOTEDBEXCEPTION(CLASS) \
class CLASS: public RemoteDbError \
{ \
public: \
    CLASS(const std::string &message); \
    virtual ~CLASS() throw (); \
};

/** @short The database doesn't contian such object */
REMOTEDBEXCEPTION(NotFoundError)
/** @short Tried to perform an operation which requires being attached into a pending changeset without one */
REMOTEDBEXCEPTION(NoChangesetError)
/** @short Execution of SQL statements resulted in an error */
REMOTEDBEXCEPTION(SqlError)
/** @short The server has experienced an internal error */
REMOTEDBEXCEPTION(ServerError)

#undef REMOTEDBEXCEPTION


/** @short Class representing the database API
 *
 * This class should contain all functionality required for working with the Deska DB.
 * */
class Api
{
public:
    virtual ~Api();

    // Querying schema definition

    /** @short Return list of names of configured top-level Kinds 
     *
     * Top-level Kinds are entities like enclosure etc, that is, object representing types.  These
     * Kinds could then be instantiated.
     * */
    virtual std::vector<Identifier> kindNames() const = 0;

    /** @short Return Attributes which are defined for a particular Kind 
     *
     * The returned data is a list of <name, datatype> pairs.
     * */
    virtual std::vector<KindAttributeDataType> kindAttributes(const Identifier &kindName) const = 0;


    /** @short Retrieve relations between different Kinds
     *
     * This function returns a list of relations for the specified kind of entities -- for more
     * details and examples, see the ObjectRelation struct.
     * */
    virtual std::vector<ObjectRelation> kindRelations(const Identifier &kindName) const = 0;


    // Returning data for existing objects

    /** @short Get identifiers of all concrete objects of a given Kind */
    virtual std::vector<Identifier> kindInstances(const Identifier &kindName, const RevisionId = RevisionId::null) const = 0;

    /** @short Get all attributes for a named object of a particular kind
     *
     * Templates: this function should not have any knowledge of "templates"; see the
     * resolvedObjectData() for template support.
     * */
    virtual std::map<Identifier, Value> objectData(
        const Identifier &kindName, const Identifier &objectName, const RevisionId = RevisionId::null) = 0;

    /** @short Get all attributes, including the inherited ones
     *
     * This function walks through the template hierarchy (see ObjectRelationKind::RELATION_TEMPLATE's documentation)
     * from the bottom up and if a value of an attribute is not explicitly set at the current level,
     * it shall continue upwards until a match for all values is known.
     *
     * The return value is a map indexed by the attribute name, with values being a pair; the second
     * value in this pair is the actual attribute value, while the first one is the identifier of an
     * object which defined said attribute. Example:
     *
     * Requesting data for the "hw DL360":
     *      power_consumption: (DL360, 500W)
     *      height: (template-1U, 1)
     *      ...
     * */
    virtual std::map<Identifier, std::pair<Identifier, Value> > resolvedObjectData(
        const Identifier &kindName, const Identifier &objectName, const RevisionId = RevisionId::null) = 0;

    /** @short Get a list of identifiers of objects which explicitly override a given attribute 
     *
     * This function walks the inheritance tree (see ObjectRelationKind::RELATION_TEMPLATE) and checks the hierarchy for objects which
     * explicitly override a declaration of an attribute value which happened at the specified template level by a new
     * definition in the object itself.
     *
     * An example is a template "boxmodel generic-1u" which specifies the height to 1, and a derived "dl360" which has
     * anything in the "height" attribute. This function's result will include the "dl360" when asked for "what gets
     * affected by a change of the "height" attribute of the "generic-1u" boxmodel.
     *
     * Note that calling this function could be very expensive.
     *
     * @see findNonOverriddenAttrs()
     *
     * */
    virtual std::vector<Identifier> findOverriddenAttrs(
        const Identifier &kindName, const Identifier &objectName, const Identifier &attributeName) = 0;

    /** @short Get a list of identifiers of objects which would be affected by a change in an attribute
     *
     * This function serves a similar role to the findOverriddenAttrs, but looks for objects which do not specify any
     * value for the attribute in question.
     *
     * Note that calling this function could be very expensive and it could very easily return vast amounts of data.
     *
     * @see findOverriddenAttrs()
     * */
    virtual std::vector<Identifier> findNonOverriddenAttrs(
        const Identifier &kindName, const Identifier &objectName, const Identifier &attributeName) = 0;

    // Manipulating objects

    /** @short Delete an item from one of the lists of objects */
    virtual void deleteObject(const Identifier &kindName, const Identifier &objectName) = 0;

    /** @short Create new object */
    virtual void createObject(const Identifier &kindName, const Identifier &objectName) = 0;

    /** @short Change object's name */
    virtual void renameObject(const Identifier &kindName, const Identifier &oldName, const Identifier &newName) = 0;

    /** @short Remove an attribute from one instance of an object */
    virtual void removeAttribute(
        const Identifier &kindName, const Identifier &objectName, const Identifier &attributeName) = 0;

    /** @short Set an attribute that belongs to some object to a new value */
    virtual void setAttribute(
        const Identifier &kindName, const Identifier &objectName, const Identifier &attributeName, const Value &attributeData) = 0;

    /** @short Apply a list of modifications */
    virtual void applyBatchedChanges(const std::vector<ObjectModification> &modifications) = 0;



    // SCM-like operation and transaction control

    /** @short Create a temporary changeset and allow modifying the DB 
     *
     * All changes affect just the temporary revision, nothing touches the live data until the
     * commit() succeeds.
     *
     * @returns a short-lived revision ID which represents the changeset being created
     * */
    virtual TemporaryChangesetId startChangeset() = 0;

    /** @short Commit current in-progress changeset 
     *
     * This operation will commit the temporary changeset (ie. everything since the corresponding
     * startChangeset()) into the production DB, creating an identifiable revision in the process.
     *
     * @arg commitMessage Human-readable message to associate with the changeset
     *
     * @returns identification of a persistent revision we just created
     * */
    virtual RevisionId commitChangeset(const std::string &commitMessage) = 0;

    /** @short Make current in-progress changeset appear as a child of a specified revision
     *
     * In order to prevent a possible loss of information, Deska won't allow a commit of an in-progress changeset to the
     * persistent, production revisions unless the latest persistent revision is the same as was at the time the user started
     * working on her in-progress copy. For example, if there was a revision X and user A started working on a changeset J, and
     * while the J still was not comitted, nother user went ahead and created revision X+1, user A won't be able to push her
     * changes to the DB, as the J changeset is internally marked as "I'm based on revision X". In order to be able to push J and
     * turn it into a persistent revision, it has to be explicitly marked as derived from X+1, which is exactly what this
     * function performs.
     *
     * @returns current revision after the rebasing; this might remain the same, or change to an arbitrary value
     */
    virtual TemporaryChangesetId rebaseChangeset(const RevisionId oldRevision) = 0;

    /** @short Return a list of all pending revisions */
    virtual std::vector<PendingChangeset> pendingChangesets() = 0;

    /** @short Re-open a pre-existing changeset
     *
     * This function will attach current session to a pre-existing changeset which hasn't been comitted yet. An example where
     * doing that would be handy is upon the initial connect to the DB, where the client would typically call
     * pendingRevisionsByMyself(), and ask the real person whether she wants to resume working on her changes, perhaps because
     * the original session has died.
     *
     * @see startChangeset()
     * @see pendingRevisionsByMyself()
     */
    virtual void resumeChangeset(const TemporaryChangesetId revision) = 0;

    /** @short Detach this session from its active changeset
     *
     * This function will detach current session from its associated active changeset. Each user can have multiple non-persistent
     * changeset in progress (that is, stored in the remote database in a special section dedicated to in-progress changesets),
     * but only one can be active at any point. This particular changeset is called an active one, and will receive updates from
     * the functions performing modifications to individual objects.
     *
     * The purpose of this function is to faciliate a way to temporarily detach from a revision which still needs some time
     * before it could be commited. After the former active changeset is detached, it remains available for further processing
     * via the resumeChangeset() function, but the current session is not associated with an active changeset anymore. This is
     * intended to make sure that user has to explicitly ask for her changes to be "set aside" instead of doing that implicitly
     * from inside startChangeset().
     *
     * @see startChangeset();
     * @see abortCurrentChangeset();
     * @see resumeChangeset();
     */
    virtual void detachFromCurrentChangeset(const std::string &message) = 0;

    /** @short Abort an in-progress changeset */
    virtual void abortCurrentChangeset() = 0;

    // Diffing

    /** @short Return a list of metadata for matching revisions */
    virtual std::vector<RevisionMetadata> listRevisions() const = 0;

    /** @short Return differences between the database state in the specified versions */
    virtual std::vector<ObjectModification> dataDifference(const RevisionId a, const RevisionId b) const = 0;

    /** @short Return differences created in a temporary changeset */
    virtual std::vector<ObjectModification> dataDifferenceInTemporaryChangeset(const TemporaryChangesetId a) const = 0;
};

}
}

#endif // DESKA_API_H
