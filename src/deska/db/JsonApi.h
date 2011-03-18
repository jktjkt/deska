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
#include <boost/signals2.hpp>
#include "Api.h"
#include "3rd-party/json_spirit_4.04/json_spirit/json_spirit.h"

namespace Deska
{

/** @short An error occured during parsing of the server's response */
class JsonParseError: public std::runtime_error
{
public:
    JsonParseError(const std::string &message);
    virtual ~JsonParseError() throw ();
};

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
    virtual std::vector<Identifier> kindInstances( const Identifier &kindName, const Revision rev = 0 ) const;
    virtual std::map<Identifier, Value> objectData(
        const Identifier &kindName, const Identifier &objectName, const Revision rev = 0 );
    virtual std::map<Identifier, std::pair<Identifier, Value> > resolvedObjectData(
            const Identifier &kindName, const Identifier &objectName, const Revision rev = 0 );
    virtual std::vector<Identifier> findOverriddenAttrs(
        const Identifier &kindName, const Identifier &objectName, const Identifier &attributeName );
    virtual std::vector<Identifier> findNonOverriddenAttrs(
        const Identifier &kindName, const Identifier &objectName, const Identifier &attributeName );

    // Manipulating objects
    virtual void deleteObject( const Identifier &kindName, const Identifier &objectName );
    virtual void createObject( const Identifier &kindName, const Identifier &objectName );
    virtual void renameObject( const Identifier &kindName, const Identifier &oldName, const Identifier &newName );
    virtual void removeAttribute(
        const Identifier &kindName, const Identifier &objectName, const Identifier &attributeName );
    virtual void setAttribute(
        const Identifier &kindName, const Identifier &objectName, const Identifier &attributeName, const Value &value );

    // SCM-like operation and transaction control
    virtual Revision startChangeset();
    virtual Revision commitChangeset();
    virtual Revision rebaseChangeset(const Revision oldRevision);
    virtual std::vector<Revision> pendingChangesetsByMyself();
    virtual void resumeChangeset(const Revision revision);
    virtual void detachFromActiveChangeset();
    virtual void abortChangeset(const Revision revision);

    /** @short Write JSON data to the DB server
     *
     * This signal is emitted when the JSON implementation of the Deska DB API needs to send a JSON request to the Deska DB.
     */
    boost::signals2::signal<void (const std::string &jsonDataToWrite)> writeString;
    /** @short Ask for JSON data from the DB server
     *
     * This signal is used to request data from the remote DB server over the JSON protocol. No arguments are passed, but the
     * attached slot should return a signle string when all data were received.
     */
    boost::signals2::signal<std::string (), boost::signals2::last_value<std::string> > readString;

private:
    /** @short The implementation wants to send a JSON object */
    void sendJsonObject(const json_spirit::Object &o) const;
    /** @short The implementation tries to obtain the JSON data */
    json_spirit::Object readJsonObject() const;

    /** @short Helper for findOverriddenAttrs and findNonOverriddenAttrs */
    std::vector<Identifier> helperOverridenAttrs(const std::string &cmd, const Identifier &kindName,
                                                 const Identifier &objectName, const Identifier &attributeName);

    /** @short Helper for createObject() and deleteObject() */
    void helperCreateDeleteObject(const std::string &cmd, const Identifier &kindName, const Identifier &objectName);

    /** @short Helper for startChangeset() and commitChangeset() */
    Revision helperStartCommitChangeset(const std::string &cmd);
};

}

#endif // DESKA_FAKEAPI_H
