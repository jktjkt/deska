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
#include <boost/process.hpp>
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
    // This is evil. Due to the standard library limitation, where exceptions thrown from the streambuf functions are caught
    // and reprocessed, we have to use this nasty side channel to get an information about what went wrong. That sucks.
    boost::process::detail::systembuf *systembuf = dynamic_cast<boost::process::detail::systembuf *>(writeStream->rdbuf());
    const char *errorString = 0;
    if (systembuf && systembuf->last_errno)
        errorString = ::strerror(systembuf->last_errno);
    if (writeStream->bad()) {
        std::ostringstream ss;
        ss << "Write error: output stream in 'bad' state";
        if (errorString)
            ss << ": " << errorString;
        throw JsonConnectionError(ss.str());
    }
    if (writeStream->fail()) {
        std::ostringstream ss;
        ss << "Write error: output stream in 'fail' state";
        if (errorString)
            ss << ": " << errorString;
        throw JsonConnectionError(ss.str());
    }
    if (writeStream->eof()) {
        std::ostringstream ss;
        ss << "Write error: EOF";
        if (errorString)
            ss << ": " << errorString;
        throw JsonConnectionError(ss.str());
    }
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
        throw JsonConnectionError("Read error: input stream in 'bad' state");
    if (readStream->fail())
        throw JsonConnectionError("Read error: input stream in 'fail' state");
    if (readStream->eof())
        throw JsonConnectionError("Read error: EOF");
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
    h.argument(j_kindName, kindName);
    h.read("kindRelations").extract(&res);
    h.work();
    return res;
}

vector<Identifier> JsonApiParser::kindInstances(const Identifier &kindName, const boost::optional<Filter> &filter, const boost::optional<RevisionId> &revision) const
{
    JsonCommandContext c1("kindInstances");

    vector<Identifier> res;
    JsonHandlerApiWrapper h(this, "kindInstances");
    h.argument(j_kindName, kindName);
    if (revision)
        h.argument(j_revision, *revision);
    if (filter)
        h.argument(j_filter, *filter);
    h.read("kindInstances").extract(&res);
    h.work();
    return res;
}

map<Identifier, Value> JsonApiParser::objectData(const Identifier &kindName, const Identifier &objectName, const boost::optional<RevisionId> &revision)
{
    JsonCommandContext c1("objectData");

    JsonHandlerApiWrapper h(this, "objectData");
    h.argument(j_kindName, kindName);
    h.argument(j_objName, objectName);
    if (revision)
        h.argument(j_revision, *revision);
    JsonWrappedAttributeMap res(kindAttributesWithoutRelation(kindName));
    h.read("objectData").extract(&res);
    h.work();
    return res.attributes;
}

map<Identifier, Value> JsonApiParser::resolvedObjectData(const Identifier &kindName, const Identifier &objectName, const boost::optional<RevisionId> &revision)
{
    JsonCommandContext c1("resolvedObjectData");

    JsonHandlerApiWrapper h(this, "resolvedObjectData");
    h.argument(j_kindName, kindName);
    h.argument(j_objName, objectName);
    if (revision)
        h.argument(j_revision, *revision);
    JsonWrappedAttributeMap res(kindAttributesWithoutRelation(kindName));
    h.read("resolvedObjectData").extract(&res);
    h.work();
    return res.attributes;
}

std::map<Identifier, std::map<Identifier, Value> > JsonApiParser::multipleObjectData(const Identifier &kindName, const boost::optional<Filter> &filter, const boost::optional<RevisionId> &revision)
{
    JsonCommandContext c1("multipleObjectData");

    JsonHandlerApiWrapper h(this, "multipleObjectData");
    h.argument(j_kindName, kindName);
    if (filter)
        h.argument(j_filter, *filter);
    if (revision)
        h.argument(j_revision, *revision);
    JsonWrappedAttributeMapList res(kindAttributesWithoutRelation(kindName));
    h.read("multipleObjectData").extract(&res);
    h.work();
    return res.objects;
}

std::map<Identifier, std::map<Identifier, Value> > JsonApiParser::multipleResolvedObjectData(const Identifier &kindName, const boost::optional<Filter> &filter, const boost::optional<RevisionId> &revision)
{
    JsonCommandContext c1("multipleResolvedObjectData");

    JsonHandlerApiWrapper h(this, "multipleResolvedObjectData");
    h.argument(j_kindName, kindName);
    if (filter)
        h.argument(j_filter, *filter);
    if (revision)
        h.argument(j_revision, *revision);
    JsonWrappedAttributeMapList res(kindAttributesWithoutRelation(kindName));
    h.read("multipleResolvedObjectData").extract(&res);
    h.work();
    return res.objects;
}

map<Identifier, pair<boost::optional<Identifier>, Value> > JsonApiParser::resolvedObjectDataWithOrigin(const Identifier &kindName,
                                                                      const Identifier &objectName, const boost::optional<RevisionId> &revision)
{
    JsonCommandContext c1("resolvedObjectDataWithOrigin");

    JsonHandlerApiWrapper h(this, "resolvedObjectDataWithOrigin");
    h.argument(j_kindName, kindName);
    h.argument(j_objName, objectName);
    if (revision)
        h.argument(j_revision, *revision);
    JsonWrappedAttributeMapWithOrigin res(kindAttributesWithoutRelation(kindName));
    h.read("resolvedObjectDataWithOrigin").extract(&res);
    h.work();
    return res.attributes;
}

std::map<Identifier, std::map<Identifier, std::pair<boost::optional<Identifier>, Value> > > JsonApiParser::multipleResolvedObjectDataWithOrigin(
    const Identifier &kindName, const boost::optional<Filter> &filter, const boost::optional<RevisionId> &revision)
{
    JsonCommandContext c1("multipleResolvedObjectDataWithOrigin");

    JsonHandlerApiWrapper h(this, "multipleResolvedObjectDataWithOrigin");
    h.argument(j_kindName, kindName);
    if (filter)
        h.argument(j_filter, *filter);
    if (revision)
        h.argument(j_revision, *revision);
    JsonWrappedAttributeMapWithOriginList res(kindAttributesWithoutRelation(kindName));
    h.read("multipleResolvedObjectDataWithOrigin").extract(&res);
    h.work();
    return res.objects;
}

void JsonApiParser::deleteObject( const Identifier &kindName, const Identifier &objectName )
{
    JsonCommandContext c1("deleteObject");

    JsonHandlerApiWrapper h(this, "deleteObject");
    h.argument(j_kindName, kindName);
    h.argument(j_objName, objectName);
    h.work();
}

void JsonApiParser::restoreDeletedObject(const Identifier &kindName, const Identifier &objectName)
{
    JsonCommandContext c1("restoreDeletedObject");

    JsonHandlerApiWrapper h(this, "restoreDeletedObject");
    h.argument(j_kindName, kindName);
    h.argument(j_objName, objectName);
    h.work();
}

Identifier JsonApiParser::createObject( const Identifier &kindName, const Identifier &objectName )
{
    JsonCommandContext c1("createObject");

    Identifier result;
    JsonHandlerApiWrapper h(this, "createObject");
    h.argument(j_kindName, kindName);
    h.argument(j_objName, objectName);
    h.read("createObject").extract(&result);
    h.work();
    return result;
}

void JsonApiParser::renameObject( const Identifier &kindName, const Identifier &oldObjectName, const Identifier &newObjectName )
{
    JsonCommandContext c1("renameObject");

    JsonHandlerApiWrapper h(this, "renameObject");
    h.argument(j_kindName, kindName);
    h.argument("oldObjectName", oldObjectName);
    h.argument("newObjectName", newObjectName);
    h.work();
}

void JsonApiParser::setAttribute(const Identifier &kindName, const Identifier &objectName, const Identifier &attributeName, const Value &attributeData)
{
    JsonCommandContext c1("setAttribute");

    JsonHandlerApiWrapper h(this, "setAttribute");
    h.argument(j_kindName, kindName);
    h.argument(j_objName, objectName);
    h.argument(j_attrName, attributeName);
    h.argument("attributeData", attributeData);
    h.work();
}

void JsonApiParser::setAttributeInsert(const Identifier &kindName, const Identifier &objectName, const Identifier &attributeName, const Identifier &attributeData)
{
    JsonCommandContext c1("setAttributeInsert");

    JsonHandlerApiWrapper h(this, "setAttributeInsert");
    h.argument(j_kindName, kindName);
    h.argument(j_objName, objectName);
    h.argument(j_attrName, attributeName);
    h.argument("attributeData", attributeData);
    h.work();
}

void JsonApiParser::setAttributeRemove(const Identifier &kindName, const Identifier &objectName, const Identifier &attributeName, const Identifier &attributeData)
{
    JsonCommandContext c1("setAttributeRemove");

    JsonHandlerApiWrapper h(this, "setAttributeRemove");
    h.argument(j_kindName, kindName);
    h.argument(j_objName, objectName);
    h.argument(j_attrName, attributeName);
    h.argument("attributeData", attributeData);
    h.work();
}

void JsonApiParser::applyBatchedChanges(const std::vector<ObjectModificationCommand> &modifications)
{
    JsonCommandContext c1("applyBatchedChanges");

    JsonHandlerApiWrapper h(this, "applyBatchedChanges");
    h.argument("modifications", modifications);
    h.work();
}

TemporaryChangesetId JsonApiParser::startChangeset()
{
    JsonCommandContext c1("startChanges");

    boost::optional<TemporaryChangesetId> revision;
    JsonHandlerApiWrapper h(this, "startChangeset");
    h.read("startChangeset").extract(&revision);
    h.work();
    BOOST_ASSERT(revision);
    return *revision;
}

RevisionId JsonApiParser::commitChangeset(const std::string &commitMessage)
{
    JsonCommandContext c1("commitChangeset");

    boost::optional<RevisionId> revision;
    JsonHandlerApiWrapper h(this, "commitChangeset");
    h.read("commitChangeset").extract(&revision);
    h.argument("commitMessage", commitMessage);
    h.work();
    BOOST_ASSERT(revision);
    return *revision;
}

vector<PendingChangeset> JsonApiParser::pendingChangesets(const boost::optional<Filter> &filter)
{
    JsonCommandContext c1("pendingChangesets");

    vector<PendingChangeset> res;
    JsonHandlerApiWrapper h(this, "pendingChangesets");
    if (filter)
        h.argument(j_filter, *filter);
    h.read("pendingChangesets").extract(&res);
    h.work();
    return res;
}

void JsonApiParser::resumeChangeset(const TemporaryChangesetId changeset)
{
    JsonCommandContext c1("resumeChangeset");

    JsonHandlerApiWrapper h(this, "resumeChangeset");
    h.argument(j_changeset, changeset);
    h.work();
}

void JsonApiParser::detachFromCurrentChangeset(const std::string &message)
{
    JsonCommandContext c1("detachFromCurrentChangeset");

    JsonHandlerApiWrapper h(this, "detachFromCurrentChangeset");
    h.argument("message", message);
    h.work();
}

void JsonApiParser::abortCurrentChangeset()
{
    JsonCommandContext c1("abortCurrentChangeset");

    JsonHandlerApiWrapper h(this, "abortCurrentChangeset");
    h.work();
}

void JsonApiParser::lockCurrentChangeset()
{
    JsonCommandContext c1("lockCurrentChangeset");

    JsonHandlerApiWrapper h(this, "lockCurrentChangeset");
    h.work();
}

void JsonApiParser::unlockCurrentChangeset()
{
    JsonCommandContext c1("unlockCurrentChangeset");

    JsonHandlerApiWrapper h(this, "unlockCurrentChangeset");
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

RevisionId JsonApiParser::restoringCommit(const std::string &commitMessage, const std::string &author, const boost::posix_time::ptime &timestamp)
{
    JsonCommandContext c1("restoringCommit");

    boost::optional<RevisionId> revision;
    JsonHandlerApiWrapper h(this, "restoringCommit");
    h.read("restoringCommit").extract(&revision);
    h.argument("commitMessage", commitMessage);
    h.argument("author", author);
    h.argument("timestamp", timestamp);
    h.work();
    BOOST_ASSERT(revision);
    return *revision;
}

std::vector<RevisionMetadata> JsonApiParser::listRevisions(const boost::optional<Filter> &filter) const
{
    JsonCommandContext c1("listRevisions");

    std::vector<RevisionMetadata> res;
    JsonHandlerApiWrapper h(this, "listRevisions");
    if (filter)
        h.argument(j_filter, *filter);
    h.read("listRevisions").extract(&res);
    h.work();
    return res;
}

namespace {

/** @short Helper for the diffing functions */
std::vector<ObjectModificationResult> diffHelper(const JsonApiParser * const dbapi, const std::string name, const boost::optional<Filter> &filter,
                                           boost::optional<TemporaryChangesetId> changeset,
                                           boost::optional<RevisionId> a, boost::optional<RevisionId> b)
{
    JsonCommandContext c1(name);

    // Request all attributes
    std::map<Identifier, std::vector<KindAttributeDataType> > allAttrTypes;
    BOOST_FOREACH(const Identifier& kindName, dbapi->kindNames()) {
        allAttrTypes[kindName] = dbapi->kindAttributes(kindName);
    }
    JsonWrappedObjectModificationResultSequence helper(&allAttrTypes);
    JsonHandlerApiWrapper h(dbapi, name);
    if (changeset) {
        h.argument("changeset", *changeset);
    } else {
        h.argument("revisionA", *a);
        h.argument("revisionB", *b);
    }
    if (filter)
        h.argument(j_filter, *filter);
    h.read(name).extract(&helper);
    h.work();
    return helper.diff;
}

}

std::vector<ObjectModificationResult> JsonApiParser::dataDifference(const RevisionId a, const RevisionId b, const boost::optional<Filter> &filter) const
{
    return diffHelper(this, "dataDifference", filter, boost::optional<TemporaryChangesetId>(), a, b);
}

std::vector<ObjectModificationResult> JsonApiParser::resolvedDataDifference(const RevisionId a, const RevisionId b, const boost::optional<Filter> &filter) const
{
    return diffHelper(this, "resolvedDataDifference", filter, boost::optional<TemporaryChangesetId>(), a, b);
}

std::vector<ObjectModificationResult> JsonApiParser::dataDifferenceInTemporaryChangeset(const TemporaryChangesetId changeset, const boost::optional<Filter> &filter) const
{
    return diffHelper(this, "dataDifferenceInTemporaryChangeset", filter, changeset, boost::optional<RevisionId>(), boost::optional<RevisionId>());
}

std::vector<ObjectModificationResult> JsonApiParser::resolvedDataDifferenceInTemporaryChangeset(const TemporaryChangesetId changeset, const boost::optional<Filter> &filter) const
{
    return diffHelper(this, "resolvedDataDifferenceInTemporaryChangeset", filter, changeset, boost::optional<RevisionId>(), boost::optional<RevisionId>());
}

std::string JsonApiParser::showConfigDiff(const ConfigGeneratingMode forceRegenerate)
{
    JsonCommandContext c1("showConfigDiff");
    JsonHandlerApiWrapper h(this, "showConfigDiff");
    switch (forceRegenerate) {
    case MAYBE_REGENERATE:
        break;
    case FORCE_REGENERATE:
        h.argument("forceRegenerate", true);
    }
    std::string res;
    h.read("showConfigDiff").extract(&res);
    h.work();
    return res;
}

}
}
