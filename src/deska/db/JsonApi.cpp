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

#include <boost/foreach.hpp>
#include "json_spirit/json_spirit_reader_template.h"
#include "json_spirit/json_spirit_writer_template.h"
#include "JsonApi.h"
#include "JsonHandler.h"

using namespace std;
using json_spirit::Object;
using json_spirit::Pair;


static std::string j_kindName = "kindName";
static std::string j_objName = "objectName";
static std::string j_attrName = "attributeName";
static std::string j_revision = "revision";
static std::string j_changeset = "changeset";
static std::string j_filter = "filter";
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
    JsonCommandContext c1("kindNames");

    vector<Identifier> res;
    JsonHandlerApiWrapper h(this, "kindNames");
    h.read("kindNames").extract(&res);
    h.work();
    return res;
}

vector<KindAttributeDataType> JsonApiParser::kindAttributes( const Identifier &kindName ) const
{
    JsonCommandContext c1("kindAttributes");

    vector<KindAttributeDataType> res;
    JsonHandlerApiWrapper h(this, "kindAttributes");
    h.argument(j_kindName, kindName);
    h.read("kindAttributes").extract(&res);
    h.work();
    return res;
}

vector<ObjectRelation> JsonApiParser::kindRelations( const Identifier &kindName ) const
{
    JsonCommandContext c1("kindRelations");

    vector<ObjectRelation> res;
    JsonHandlerApiWrapper h(this, "kindRelations");
    h.write(j_kindName, kindName);
    h.read("kindRelations").extract(&res);
    h.work();
    return res;
}

vector<Identifier> JsonApiParser::kindInstances(const Identifier &kindName, const boost::optional<Filter> &filter, const RevisionId revision) const
{
    JsonCommandContext c1("kindInstances");

    vector<Identifier> res;
    JsonHandlerApiWrapper h(this, "kindInstances");
    h.write(j_kindName, kindName);
    if (revision != RevisionId::null)
        h.write(j_revision, revision);
    if (filter)
        h.write(j_filter, *filter);
    h.read("kindInstances").extract(&res);
    h.work();
    return res;
}

map<Identifier, Value> JsonApiParser::objectData( const Identifier &kindName, const Identifier &objectName, const RevisionId revision )
{
    JsonCommandContext c1("objectData");

    JsonHandlerApiWrapper h(this, "objectData");
    h.write(j_kindName, kindName);
    h.write(j_objName, objectName);
    if (revision != RevisionId::null)
        h.write(j_revision, revision);
    JsonWrappedAttributeMap res(kindAttributesWithoutRelation(kindName));
    h.read("objectData").extract(&res);
    h.work();
    return res.attributes;
}

std::map<Identifier, std::map<Identifier, Value> > JsonApiParser::multipleObjectData(const Identifier &kindName, const Filter &filter, const RevisionId revision)
{
    JsonCommandContext c1("multipleObjectData");

    JsonHandlerApiWrapper h(this, "multipleObjectData");
    h.write(j_kindName, kindName);
    h.write(j_filter, filter);
    if (revision != RevisionId::null)
        h.write(j_revision, revision);
    JsonWrappedAttributeMapList res(kindAttributesWithoutRelation(kindName));
    h.read("multipleObjectData").extract(&res);
    h.work();
    return res.objects;
}


map<Identifier, pair<Identifier, Value> > JsonApiParser::resolvedObjectData(const Identifier &kindName,
                                                                      const Identifier &objectName, const RevisionId revision )
{
    JsonCommandContext c1("resolvedObjectData");

    JsonHandlerApiWrapper h(this, "resolvedObjectData");
    h.write(j_kindName, kindName);
    h.write(j_objName, objectName);
    if (revision != RevisionId::null)
        h.write(j_revision, revision);
    JsonWrappedAttributeMapWithOrigin res(kindAttributesWithoutRelation(kindName));
    h.read("resolvedObjectData").extract(&res);
    h.work();
    return res.attributes;
}

std::map<Identifier, std::map<Identifier, std::pair<Identifier, Value> > > JsonApiParser::multipleResolvedObjectData(
    const Identifier &kindName, const Filter &filter, const RevisionId revision)
{
    JsonCommandContext c1("multipleResolvedObjectData");

    JsonHandlerApiWrapper h(this, "multipleResolvedObjectData");
    h.write(j_kindName, kindName);
    h.write(j_filter, filter);
    if (revision != RevisionId::null)
        h.write(j_revision, revision);
    JsonWrappedAttributeMapWithOriginList res(kindAttributesWithoutRelation(kindName));
    h.read("multipleResolvedObjectData").extract(&res);
    h.work();
    return res.objects;
}

void JsonApiParser::deleteObject( const Identifier &kindName, const Identifier &objectName )
{
    JsonCommandContext c1("deleteObject");

    JsonHandlerApiWrapper h(this, "deleteObject");
    h.write(j_kindName, kindName);
    h.write(j_objName, objectName);
    h.work();
}

void JsonApiParser::restoreDeletedObject(const Identifier &kindName, const Identifier &objectName)
{
    JsonCommandContext c1("restoreDeletedObject");

    JsonHandlerApiWrapper h(this, "restoreDeletedObject");
    h.write(j_kindName, kindName);
    h.write(j_objName, objectName);
    h.work();
}

void JsonApiParser::createObject( const Identifier &kindName, const Identifier &objectName )
{
    JsonCommandContext c1("createObject");

    JsonHandlerApiWrapper h(this, "createObject");
    h.write(j_kindName, kindName);
    h.write(j_objName, objectName);
    h.work();
}

void JsonApiParser::renameObject( const Identifier &kindName, const Identifier &oldObjectName, const Identifier &newObjectName )
{
    JsonCommandContext c1("renameObject");

    JsonHandlerApiWrapper h(this, "renameObject");
    h.write(j_kindName, kindName);
    h.write("oldObjectName", oldObjectName);
    h.write("newObjectName", newObjectName);
    h.work();
}

void JsonApiParser::setAttribute(const Identifier &kindName, const Identifier &objectName, const Identifier &attributeName,
                           const Value &attributeData)
{
    JsonCommandContext c1("setAttribute");

    JsonHandlerApiWrapper h(this, "setAttribute");
    h.write(j_kindName, kindName);
    h.write(j_objName, objectName);
    h.write(j_attrName, attributeName);
    h.write("attributeData", attributeData);
    h.work();
}

void JsonApiParser::applyBatchedChanges(const std::vector<ObjectModification> &modifications)
{
    JsonCommandContext c1("applyBatchedChanges");

    JsonHandlerApiWrapper h(this, "applyBatchedChanges");
    h.write("modifications", modifications);
    h.work();
}

TemporaryChangesetId JsonApiParser::startChangeset()
{
    JsonCommandContext c1("startChanges");

    TemporaryChangesetId revision = TemporaryChangesetId::null;
    JsonHandlerApiWrapper h(this, "startChangeset");
    h.read("startChangeset").extract(&revision);
    h.work();
    return revision;
}

RevisionId JsonApiParser::commitChangeset(const std::string &commitMessage)
{
    JsonCommandContext c1("commitChangeset");

    RevisionId revision = RevisionId::null;
    JsonHandlerApiWrapper h(this, "commitChangeset");
    h.read("commitChangeset").extract(&revision);
    h.write("commitMessage", commitMessage);
    h.work();
    return revision;
}

void JsonApiParser::rebaseChangeset(const RevisionId parentRevision)
{
    JsonCommandContext c1("rebaseChangeset");

    JsonHandlerApiWrapper h(this, "rebaseChangeset");
    h.write("parentRevision", parentRevision);
    h.work();
}

vector<PendingChangeset> JsonApiParser::pendingChangesets(const boost::optional<Filter> &filter)
{
    JsonCommandContext c1("pendingChangesets");

    vector<PendingChangeset> res;
    JsonHandlerApiWrapper h(this, "pendingChangesets");
    if (filter)
        h.write(j_filter, *filter);
    h.read("pendingChangesets").extract(&res);
    h.work();
    return res;
}

void JsonApiParser::resumeChangeset(const TemporaryChangesetId changeset)
{
    JsonCommandContext c1("resumeChangeset");

    JsonHandlerApiWrapper h(this, "resumeChangeset");
    h.write(j_changeset, changeset);
    h.work();
}

void JsonApiParser::detachFromCurrentChangeset(const std::string &message)
{
    JsonCommandContext c1("detachFromCurrentChangeset");

    JsonHandlerApiWrapper h(this, "detachFromCurrentChangeset");
    h.write("message", message);
    h.work();
}

void JsonApiParser::abortCurrentChangeset()
{
    JsonCommandContext c1("abortCurrentChangeset");

    JsonHandlerApiWrapper h(this, "abortCurrentChangeset");
    h.work();
}

void JsonApiParser::freezeView()
{
    JsonCommandContext c1("freezeView");

    JsonHandlerApiWrapper h(this, "freezeView");
    h.work();
}

void JsonApiParser::unFreezeView()
{
    JsonCommandContext c1("unFreezeView");

    JsonHandlerApiWrapper h(this, "unFreezeView");
    h.work();
}

std::vector<RevisionMetadata> JsonApiParser::listRevisions(const boost::optional<Filter> &filter) const
{
    JsonCommandContext c1("listRevisions");

    std::vector<RevisionMetadata> res;
    JsonHandlerApiWrapper h(this, "listRevisions");
    if (filter)
        h.write(j_filter, *filter);
    h.read("listRevisions").extract(&res);
    h.work();
    return res;
}

namespace {

/** @short Helper for the diffing functions */
std::vector<ObjectModification> diffHelper(const JsonApiParser * const dbapi, const std::string name, const boost::optional<Filter> &filter,
                                           boost::optional<TemporaryChangesetId> changeset,
                                           boost::optional<RevisionId> a, boost::optional<RevisionId> b)
{
    JsonCommandContext c1(name);

    // Request all attributes
    std::map<Identifier, std::vector<KindAttributeDataType> > allAttrTypes;
    BOOST_FOREACH(const Identifier& kindName, dbapi->kindNames()) {
        allAttrTypes[kindName] = dbapi->kindAttributes(kindName);
    }
    JsonWrappedObjectModificationSequence helper(&allAttrTypes);
    JsonHandlerApiWrapper h(dbapi, name);
    if (changeset) {
        h.write("changeset", *changeset);
    } else {
        h.write("revisionA", *a);
        h.write("revisionB", *b);
    }
    if (filter)
        h.write(j_filter, *filter);
    h.read(name).extract(&helper);
    h.work();
    return helper.diff;
}

}

std::vector<ObjectModification> JsonApiParser::dataDifference(const RevisionId a, const RevisionId b, const boost::optional<Filter> &filter) const
{
    return diffHelper(this, "dataDifference", filter, boost::optional<TemporaryChangesetId>(), a, b);
}

std::vector<ObjectModification> JsonApiParser::resolvedDataDifference(const RevisionId a, const RevisionId b, const boost::optional<Filter> &filter) const
{
    return diffHelper(this, "resolvedDataDifference", filter, boost::optional<TemporaryChangesetId>(), a, b);
}

std::vector<ObjectModification> JsonApiParser::dataDifferenceInTemporaryChangeset(const TemporaryChangesetId changeset, const boost::optional<Filter> &filter) const
{
    return diffHelper(this, "dataDifferenceInTemporaryChangeset", filter, changeset, boost::optional<RevisionId>(), boost::optional<RevisionId>());
}

std::vector<ObjectModification> JsonApiParser::resolvedDataDifferenceInTemporaryChangeset(const TemporaryChangesetId changeset, const boost::optional<Filter> &filter) const
{
    return diffHelper(this, "resolvedDataDifferenceInTemporaryChangeset", filter, changeset, boost::optional<RevisionId>(), boost::optional<RevisionId>());
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


JsonCommandContext::JsonCommandContext(const std::string &ctx):
    m_apiContext(std::string("In ") + ctx + " API method"),
    m_jsonContext(std::string("In ") + ctx + " API method")
{
}

}
}
