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
#include "deska/db/Api.h"
#include "deska/db/Objects.h"
#include "deska/db/Revisions.h"

namespace Deska {

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
    *   @param object Object to create
    */
    void createObject(const Db::ObjectDefinition &object);
    /** @short Deletes object.
    *
    *   @param object Object to delete
    */
    void deleteObject(const Db::ObjectDefinition &object);
    /** @short Sets attribute value in the object.
    *
    *   @param object Object which attribute will be changed
    *   @param attribute Attribute and value to set
    */
    void setAttribute(const Db::ObjectDefinition &object, const Db::AttributeDefinition &attribute);

    /** @short Obtains list of all objects in the DB.
    *
    *   @return Vector of object definitions from the DB
    */
    std::vector<Db::ObjectDefinition> allObjects();
    /** @short Obtains all attributes of given object.
    *
    *   @param object Object for which the attributes are obtained
    *   @return Vector of all attributes
    */
    std::vector<Db::AttributeDefinition> allAttributes(const Db::ObjectDefinition &object);

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

private:

    /** Pointer to the api for communication with the DB. */
    Db::Api *m_api;
};


}
}

#endif  // DESKA_DB_INTERACTION_H
