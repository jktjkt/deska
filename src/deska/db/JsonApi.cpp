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

#include "JsonApi.h"
#include "JsonHandler.h"

using namespace std;
using json_spirit::Object;
using json_spirit::Pair;


static std::string j_kindName = "kindName";
static std::string j_objName = "objectName";
static std::string j_attrName = "attributeName";
static std::string j_revision = "revision";
static std::string j_errorPrefix = "error";

static std::string j_cmd_kindNames = "getTopLevelObjectNames";
static std::string j_cmd_kindAttributes = "getKindAttributes";
static std::string j_cmd_kindRelations = "getKindRelations";
static std::string j_cmd_kindInstances = "getKindInstances";
static std::string j_cmd_objectData = "getObjectData";
static std::string j_cmd_resolvedObjectData = "getResolvedObjectData";
static std::string j_cmd_findObjectsOverridingAttrs = "getObjectsOverridingAttribute";
static std::string j_cmd_findObjectsNotOverridingAttrs = "getObjectsNotOverridingAttribute";
static std::string j_cmd_createObject = "createObject";
static std::string j_cmd_deleteObject = "deleteObject";
static std::string j_cmd_renameObject = "renameObject";
static std::string j_cmd_removeAttribute = "removeObjectAttribute";
static std::string j_cmd_setAttribute = "setObjectAttribute";
static std::string j_cmd_startChangeset = "vcsStartChangeset";
static std::string j_cmd_commitChangeset = "vcsCommitChangeset";
static std::string j_cmd_rebaseChangeset = "vcsRebaseChangeset";
static std::string j_cmd_pendingChangesetsByMyself = "vcsGetPendingChangesetsByMyself";
static std::string j_cmd_resumeChangeset = "vcsResumePendingChangeset";
static std::string j_cmd_detachFromCurrentChangeset = "vcsDetachFromCurrentChangeset";
static std::string j_cmd_abortCurrentChangeset = "vcsAbortCurrentChangeset";

namespace Deska {
namespace Db {

JsonApiParser::JsonApiParser()
{
}

JsonApiParser::~JsonApiParser()
{
}

void JsonApiParser::sendJsonObject(const json_spirit::Object &o) const
{
    writeString(json_spirit::write(o, json_spirit::remove_trailing_zeros));
}

json_spirit::Object JsonApiParser::readJsonObject() const
{
    json_spirit::Value res;
    try {
        json_spirit::read_or_throw(readString(), res);
        // FIXME: convert this to iteratos, as this method would happily parse "{}{" and not report back
        // that the "{" was silently ignored
    } catch (const json_spirit::Error_position &e) {
        // FIXME: Exception handling. This one is rather naive approach, see bug #155 for details.
        std::ostringstream s;
        s << "JSON parsing error at line " << e.line_ << " column " << e.column_ << ": " << e.reason_;
        throw JsonParseError(s.str());
    }
    const json_spirit::Object &o = res.get_obj();
    // FIXME: check for the j_errorPrefix here
    return o;
}

vector<Identifier> JsonApiParser::kindNames() const
{
    vector<Identifier> res;
    JsonHandler h(this, j_cmd_kindNames);
    h.read("topLevelObjectKinds").extract(&res);
    h.work();
    return res;
}

vector<KindAttributeDataType> JsonApiParser::kindAttributes( const Identifier &kindName ) const
{
    vector<KindAttributeDataType> res;
    JsonHandler h(this, j_cmd_kindAttributes);
    h.write(j_kindName, kindName);
    h.read("kindAttributes").extract(&res);
    h.work();
    return res;
}

vector<ObjectRelation> JsonApiParser::kindRelations( const Identifier &kindName ) const
{
    vector<ObjectRelation> res;
    JsonHandler h(this, j_cmd_kindRelations);
    h.write(j_kindName, kindName);
    h.read("kindRelations").extract(&res);
    h.work();
    return res;
}

vector<Identifier> JsonApiParser::kindInstances( const Identifier &kindName, const RevisionId revision ) const
{
    vector<Identifier> res;
    JsonHandler h(this, j_cmd_kindInstances);
    h.write(j_kindName, kindName);
    h.write(j_revision, revision);
    h.read("objectInstances").extract(&res);
    h.work();
    return res;
}

map<Identifier, Value> JsonApiParser::objectData( const Identifier &kindName, const Identifier &objectName, const RevisionId revision )
{
    map<Identifier, Value> res;
    JsonHandler h(this, j_cmd_objectData);
    h.write(j_kindName, kindName);
    h.write(j_objName, objectName);
    h.write(j_revision, revision);
    h.read("objectData").extract(&res);
    h.work();
    return res;
}

map<Identifier, pair<Identifier, Value> > JsonApiParser::resolvedObjectData(const Identifier &kindName,
                                                                      const Identifier &objectName, const RevisionId revision )
{
    map<Identifier, pair<Identifier, Value> > res;
    JsonHandler h(this, j_cmd_resolvedObjectData);
    h.write(j_kindName, kindName);
    h.write(j_objName, objectName);
    h.write(j_revision, revision);
    h.read("resolvedObjectData").extract(&res);
    h.work();
    return res;
}

vector<Identifier> JsonApiParser::findOverriddenAttrs(const Identifier &kindName, const Identifier &objectName,
                                                const Identifier &attributeName)
{
    vector<Identifier> res;
    JsonHandler h(this, j_cmd_findObjectsOverridingAttrs);
    h.write(j_kindName, kindName);
    h.write(j_objName, objectName);
    h.write(j_attrName, attributeName);
    h.read("objectInstances").extract(&res);
    h.work();
    return res;
}

vector<Identifier> JsonApiParser::findNonOverriddenAttrs(const Identifier &kindName, const Identifier &objectName,
                                                   const Identifier &attributeName)
{
    vector<Identifier> res;
    JsonHandler h(this, j_cmd_findObjectsNotOverridingAttrs);
    h.write(j_kindName, kindName);
    h.write(j_objName, objectName);
    h.write(j_attrName, attributeName);
    h.read("objectInstances").extract(&res);
    h.work();
    return res;
}

void JsonApiParser::deleteObject( const Identifier &kindName, const Identifier &objectName )
{
    JsonHandler h(this, j_cmd_deleteObject);
    h.write(j_kindName, kindName);
    h.write(j_objName, objectName);
    h.expectTrue("result");
    h.work();
}

void JsonApiParser::createObject( const Identifier &kindName, const Identifier &objectName )
{
    JsonHandler h(this, j_cmd_createObject);
    h.write(j_kindName, kindName);
    h.write(j_objName, objectName);
    h.expectTrue("result");
    h.work();
}

void JsonApiParser::renameObject( const Identifier &kindName, const Identifier &oldName, const Identifier &newName )
{
    JsonHandler h(this, j_cmd_renameObject);
    h.write(j_kindName, kindName);
    h.write(j_objName, oldName);
    h.write("newObjectName", newName);
    h.expectTrue("result");
    h.work();
}

void JsonApiParser::removeAttribute(const Identifier &kindName, const Identifier &objectName, const Identifier &attributeName)
{
    JsonHandler h(this, j_cmd_removeAttribute);
    h.write(j_kindName, kindName);
    h.write(j_objName, objectName);
    h.write(j_attrName, attributeName);
    h.expectTrue("result");
    h.work();
}

void JsonApiParser::setAttribute(const Identifier &kindName, const Identifier &objectName, const Identifier &attributeName,
                           const Value &value)
{
    JsonHandler h(this, j_cmd_setAttribute);
    h.write(j_kindName, kindName);
    h.write(j_objName, objectName);
    h.write(j_attrName, attributeName);
    h.write("attributeData", value);
    h.expectTrue("result");
    h.work();
}

TemporaryChangesetId JsonApiParser::startChangeset()
{
    TemporaryChangesetId revision = TemporaryChangesetId::null;
    JsonHandler h(this, j_cmd_startChangeset);
    h.read(j_revision).extract(&revision);
    h.work();
    return revision;
}

RevisionId JsonApiParser::commitChangeset()
{
    RevisionId revision = RevisionId::null;
    JsonHandler h(this, j_cmd_commitChangeset);
    h.read(j_revision).extract(&revision);
    h.work();
    return revision;
}

TemporaryChangesetId JsonApiParser::rebaseChangeset(const RevisionId oldRevision)
{
    TemporaryChangesetId revision = TemporaryChangesetId::null;
    JsonHandler h(this, j_cmd_rebaseChangeset);
    h.write("currentRevision", oldRevision);
    h.read(j_revision).extract(&revision);
    h.work();
    return revision;
}

vector<TemporaryChangesetId> JsonApiParser::pendingChangesetsByMyself()
{
    vector<TemporaryChangesetId> res;
    JsonHandler h(this, j_cmd_pendingChangesetsByMyself);
    h.read("revisions").extract(&res);
    h.work();
    return res;
}

void JsonApiParser::resumeChangeset(const TemporaryChangesetId revision)
{
    JsonHandler h(this, j_cmd_resumeChangeset);
    h.write(j_revision, revision);
    h.work();
}

void JsonApiParser::detachFromCurrentChangeset(const std::string &message)
{
    JsonHandler h(this, j_cmd_detachFromCurrentChangeset);
    h.write("message", message);
    h.work();
}

void JsonApiParser::abortCurrentChangeset()
{
    JsonHandler h(this, j_cmd_abortCurrentChangeset);
    h.work();
}



JsonParseError::JsonParseError(const std::string &message): std::runtime_error(message)
{
}

JsonParseError::~JsonParseError() throw ()
{
}

}
}
