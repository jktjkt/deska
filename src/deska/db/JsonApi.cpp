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

#include "3rd-party/json_spirit_4.04/json_spirit/json_spirit_reader_template.h"
#include "3rd-party/json_spirit_4.04/json_spirit/json_spirit_writer_template.h"
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

namespace Deska {
namespace Db {

JsonApiParser::JsonApiParser(): m_writeStream(0), m_readStream(0)
{
}

JsonApiParser::~JsonApiParser()
{
}

void JsonApiParser::setStreams(std::ostream *writeStream, std::istream *readStream)
{
    m_writeStream = writeStream;
    m_readStream = readStream;
}

void JsonApiParser::sendJsonObject(const json_spirit::Object &o) const
{
    BOOST_ASSERT(m_writeStream);
    json_spirit::write_stream(json_spirit::Value(o), *m_writeStream, json_spirit::remove_trailing_zeros);
    *m_writeStream << "\n";
    m_writeStream->flush();
    if (m_writeStream->bad())
        throw JsonParseError("Write error: output stream in 'bad' state");
    if (m_writeStream->fail())
        throw JsonParseError("Write error: output stream in 'fail' state");
    if (m_writeStream->eof())
        throw JsonParseError("Write error: EOF");
}

json_spirit::Object JsonApiParser::readJsonObject() const
{
    BOOST_ASSERT(m_readStream);
    json_spirit::Value res;
    try {
        json_spirit::read_stream_or_throw(*m_readStream, res);
    } catch (const json_spirit::Error_position &e) {
        // FIXME: Exception handling. This one is rather naive approach, see bug #155 for details.
        std::ostringstream s;
        s << "JSON parsing error at line " << e.line_ << " column " << e.column_ << ": " << e.reason_;
        throw JsonParseError(s.str());
    }
    const json_spirit::Object &o = res.get_obj();
    // FIXME: check for the j_errorPrefix here
    if (m_readStream->bad())
        throw JsonParseError("Read error: input stream in 'bad' state");
    if (m_readStream->fail())
        throw JsonParseError("Read error: input stream in 'fail' state");
    if (m_readStream->eof())
        throw JsonParseError("Read error: EOF");
    return o;
}

vector<Identifier> JsonApiParser::kindNames() const
{
    vector<Identifier> res;
    JsonHandlerApiWrapper h(this, "kindNames");
    h.read("kindNames").extract(&res);
    h.work();
    return res;
}

vector<KindAttributeDataType> JsonApiParser::kindAttributes( const Identifier &kindName ) const
{
    vector<KindAttributeDataType> res;
    JsonHandlerApiWrapper h(this, "kindAttributes");
    h.write(j_kindName, kindName);
    h.read("kindAttributes").extract(&res);
    h.work();
    return res;
}

vector<ObjectRelation> JsonApiParser::kindRelations( const Identifier &kindName ) const
{
    vector<ObjectRelation> res;
    JsonHandlerApiWrapper h(this, "kindRelations");
    h.write(j_kindName, kindName);
    h.read("kindRelations").extract(&res);
    h.work();
    return res;
}

vector<Identifier> JsonApiParser::kindInstances( const Identifier &kindName, const RevisionId revision ) const
{
    vector<Identifier> res;
    JsonHandlerApiWrapper h(this, "kindInstances");
    h.write(j_kindName, kindName);
    h.writeIfNotZero(j_revision, revision);
    h.read("kindInstances").extract(&res);
    h.work();
    return res;
}

map<Identifier, Value> JsonApiParser::objectData( const Identifier &kindName, const Identifier &objectName, const RevisionId revision )
{
    map<Identifier, Value> res;
    JsonHandlerApiWrapper h(this, "objectData");
    h.write(j_kindName, kindName);
    h.write(j_objName, objectName);
    h.writeIfNotZero(j_revision, revision);
    h.read("objectData").extract(&res);
    h.work();
    return res;
}

map<Identifier, pair<Identifier, Value> > JsonApiParser::resolvedObjectData(const Identifier &kindName,
                                                                      const Identifier &objectName, const RevisionId revision )
{
    map<Identifier, pair<Identifier, Value> > res;
    JsonHandlerApiWrapper h(this, "resolvedObjectData");
    h.write(j_kindName, kindName);
    h.write(j_objName, objectName);
    h.writeIfNotZero(j_revision, revision);
    h.read("resolvedObjectData").extract(&res);
    h.work();
    return res;
}

vector<Identifier> JsonApiParser::findOverriddenAttrs(const Identifier &kindName, const Identifier &objectName,
                                                const Identifier &attributeName)
{
    vector<Identifier> res;
    JsonHandlerApiWrapper h(this, "findOverriddenAttrs");
    h.write(j_kindName, kindName);
    h.write(j_objName, objectName);
    h.write(j_attrName, attributeName);
    h.read("findOverriddenAttrs").extract(&res);
    h.work();
    return res;
}

vector<Identifier> JsonApiParser::findNonOverriddenAttrs(const Identifier &kindName, const Identifier &objectName,
                                                   const Identifier &attributeName)
{
    vector<Identifier> res;
    JsonHandlerApiWrapper h(this, "findNonOverriddenAttrs");
    h.write(j_kindName, kindName);
    h.write(j_objName, objectName);
    h.write(j_attrName, attributeName);
    h.read("findNonOverriddenAttrs").extract(&res);
    h.work();
    return res;
}

void JsonApiParser::deleteObject( const Identifier &kindName, const Identifier &objectName )
{
    JsonHandlerApiWrapper h(this, "deleteObject");
    h.write(j_kindName, kindName);
    h.write(j_objName, objectName);
    h.expectTrue("result");
    h.work();
}

void JsonApiParser::createObject( const Identifier &kindName, const Identifier &objectName )
{
    JsonHandlerApiWrapper h(this, "createObject");
    h.write(j_kindName, kindName);
    h.write(j_objName, objectName);
    h.expectTrue("result");
    h.work();
}

void JsonApiParser::renameObject( const Identifier &kindName, const Identifier &oldName, const Identifier &newName )
{
    JsonHandlerApiWrapper h(this, "renameObject");
    h.write(j_kindName, kindName);
    h.write(j_objName, oldName);
    h.write("newObjectName", newName);
    h.expectTrue("result");
    h.work();
}

void JsonApiParser::removeAttribute(const Identifier &kindName, const Identifier &objectName, const Identifier &attributeName)
{
    JsonHandlerApiWrapper h(this, "removeAttribute");
    h.write(j_kindName, kindName);
    h.write(j_objName, objectName);
    h.write(j_attrName, attributeName);
    h.expectTrue("result");
    h.work();
}

void JsonApiParser::setAttribute(const Identifier &kindName, const Identifier &objectName, const Identifier &attributeName,
                           const Value &value)
{
    JsonHandlerApiWrapper h(this, "setAttribute");
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
    JsonHandlerApiWrapper h(this, "startChangeset");
    h.read("startChangeset").extract(&revision);
    h.work();
    return revision;
}

RevisionId JsonApiParser::commitChangeset(const std::string &commitMessage)
{
    RevisionId revision = RevisionId::null;
    JsonHandlerApiWrapper h(this, "commitChangeset");
    h.read("commitChangeset").extract(&revision);
    h.write("commitMessage", commitMessage);
    h.work();
    return revision;
}

TemporaryChangesetId JsonApiParser::rebaseChangeset(const RevisionId oldRevision)
{
    TemporaryChangesetId revision = TemporaryChangesetId::null;
    JsonHandlerApiWrapper h(this, "rebaseChangeset");
    h.write("currentRevision", oldRevision);
    h.read("rebaseChangeset").extract(&revision);
    h.work();
    return revision;
}

vector<PendingChangeset> JsonApiParser::pendingChangesets()
{
    vector<PendingChangeset> res;
    JsonHandlerApiWrapper h(this, "pendingChangesets");
    h.read("pendingChangesets").extract(&res);
    h.work();
    return res;
}

void JsonApiParser::resumeChangeset(const TemporaryChangesetId revision)
{
    JsonHandlerApiWrapper h(this, "resumeChangeset");
    h.write(j_revision, revision);
    h.expectTrue("result");
    h.work();
}

void JsonApiParser::detachFromCurrentChangeset(const std::string &message)
{
    JsonHandlerApiWrapper h(this, "detachFromCurrentChangeset");
    h.write("message", message);
    h.work();
}

void JsonApiParser::abortCurrentChangeset()
{
    JsonHandlerApiWrapper h(this, "abortCurrentChangeset");
    h.expectTrue("result");
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
