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

JsonApiParser::JsonApiParser()
{
}

JsonApiParser::~JsonApiParser()
{
}

void JsonApiParser::sendJsonObject(const json_spirit::Object &o) const
{
    std::ostream *writeStream = willWrite();
    BOOST_ASSERT(writeStream);
    json_spirit::write_stream(json_spirit::Value(o), *writeStream, json_spirit::remove_trailing_zeros);
    *writeStream << "\n";
    writeStream->flush();
    if (writeStream->bad())
        throw JsonSyntaxError("Write error: output stream in 'bad' state");
    if (writeStream->fail())
        throw JsonSyntaxError("Write error: output stream in 'fail' state");
    if (writeStream->eof())
        throw JsonSyntaxError("Write error: EOF");
}

json_spirit::Object JsonApiParser::readJsonObject() const
{
    JsonContext c1("When reading JSON object");

    std::istream *readStream = willRead();
    BOOST_ASSERT(readStream);
    json_spirit::Value res;
    try {
        json_spirit::read_stream_or_throw(*readStream, res);
    } catch (const json_spirit::Error_position &e) {
        std::ostringstream s;
        s << "JSON parsing error at line " << e.line_ << " column " << e.column_ << ": " << e.reason_;
        throw JsonSyntaxError(s.str());
    } catch (const std::string &e) {
        std::ostringstream s;
        s << "JSON parsing error: " << e;
        throw JsonSyntaxError(s.str());
    } catch (const std::runtime_error &e) {
        std::ostringstream s;
        s << "JSON parsing error: runtime_error: " << e.what();
        throw JsonSyntaxError(s.str());
    }
    if (readStream->bad())
        throw JsonSyntaxError("Read error: input stream in 'bad' state");
    if (readStream->fail())
        throw JsonSyntaxError("Read error: input stream in 'fail' state");
    if (readStream->eof())
        throw JsonSyntaxError("Read error: EOF");
    return res.get_obj();
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
    JsonContext c1("In kindAttributes() API method");

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
    JsonContext c1("In objectData() API method");
    JsonHandlerApiWrapper h(this, "objectData");
    h.write(j_kindName, kindName);
    h.write(j_objName, objectName);
    h.writeIfNotZero(j_revision, revision);
    JsonWrappedAttributeMap res(kindAttributes(kindName));
    h.read("objectData").extract(&res);
    h.work();
    return res.attributes;
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
    h.work();
}

void JsonApiParser::createObject( const Identifier &kindName, const Identifier &objectName )
{
    JsonHandlerApiWrapper h(this, "createObject");
    h.write(j_kindName, kindName);
    h.write(j_objName, objectName);
    h.work();
}

void JsonApiParser::renameObject( const Identifier &kindName, const Identifier &oldName, const Identifier &newName )
{
    JsonHandlerApiWrapper h(this, "renameObject");
    h.write(j_kindName, kindName);
    h.write(j_objName, oldName);
    h.write("newObjectName", newName);
    h.work();
}

void JsonApiParser::removeAttribute(const Identifier &kindName, const Identifier &objectName, const Identifier &attributeName)
{
    JsonHandlerApiWrapper h(this, "removeAttribute");
    h.write(j_kindName, kindName);
    h.write(j_objName, objectName);
    h.write(j_attrName, attributeName);
    h.work();
}

void JsonApiParser::setAttribute(const Identifier &kindName, const Identifier &objectName, const Identifier &attributeName,
                           const Value &attributeData)
{
    JsonHandlerApiWrapper h(this, "setAttribute");
    h.write(j_kindName, kindName);
    h.write(j_objName, objectName);
    h.write(j_attrName, attributeName);
    h.write("attributeData", attributeData);
    h.work();
}

void JsonApiParser::applyBatchedChanges(const std::vector<ObjectModification> &modifications)
{
    JsonHandlerApiWrapper h(this, "applyBatchedChanges");
    h.write("modifications", modifications);
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
    JsonContext c1("In commitChangeset() API method");
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
    h.work();
}

std::vector<RevisionMetadata> JsonApiParser::listRevisions() const
{
    JsonContext c1("In listRevisions() API method");
    std::vector<RevisionMetadata> res;
    JsonHandlerApiWrapper h(this, "listRevisions");
    h.read("listRevisions").extract(&res);
    h.work();
    return res;
}

std::vector<ObjectModification> JsonApiParser::dataDifference(const RevisionId a, const RevisionId b) const
{
    JsonContext c1("In dataDifference API method");
    std::vector<ObjectModification> res;
    JsonHandlerApiWrapper h(this, "dataDifference");
    h.write("revisionA", a);
    h.write("revisionB", b);
    h.read("dataDifference").extract(&res);
    h.work();
    return res;
}

std::vector<ObjectModification> JsonApiParser::dataDifferenceInTemporaryChangeset(const TemporaryChangesetId changeset) const
{
    JsonContext c1("In dataDifferenceInTemporaryChangeset API method");
    std::vector<ObjectModification> res;
    JsonHandlerApiWrapper h(this, "dataDifferenceInTemporaryChangeset");
    h.write("changeset", changeset);
    h.read("dataDifferenceInTemporaryChangeset").extract(&res);
    h.work();
    return res;
}




JsonParseError::JsonParseError(const std::string &message): std::runtime_error(message)
{
}

JsonParseError::~JsonParseError() throw ()
{
}

void JsonParseError::addRawJsonData(const std::string &data)
{
    std::ostringstream ss;
    ss << std::runtime_error::what() << std::endl << "Raw JSON data read from the process: '" << data << "'";
    m_completeError = ss.str();
}

const char* JsonParseError::what() const throw()
{
    return m_completeError.empty() ? std::runtime_error::what() : m_completeError.c_str();
}

std::string JsonParseError::whatWithBacktrace() const throw()
{
    // We're required not to throw, so we have to use a generic catch-all block here
    try {
        std::ostringstream ss;
        ss << "* " << backtrace("\n * ") << what() << std::endl;
        return ss.str();
    } catch (...) {
        return what();
    }
}

JsonSyntaxError::JsonSyntaxError(const std::string &message): JsonParseError(message)
{
}

JsonSyntaxError::~JsonSyntaxError() throw ()
{
}

JsonStructureError::JsonStructureError(const std::string &message): JsonParseError(message)
{
}

JsonStructureError::~JsonStructureError() throw ()
{
}

}
}
