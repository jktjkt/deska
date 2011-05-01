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

#ifndef DESKA_DB_JSON_EXTRACTION_H
#define DESKA_DB_JSON_EXTRACTION_H

#include <boost/optional.hpp>
#include "JsonHandler.h"
#include "JsonApi.h"

using namespace std;
using json_spirit::Object;
using json_spirit::Pair;

namespace Deska {
namespace Db {

/** @short Variant visitor convert a Deska::Db::Value to json_spirit::Value */
struct DeskaValueToJsonValue: public boost::static_visitor<json_spirit::Value>
{
    /** @short Simply use json_spirit::Value's overloaded constructor */
    template <typename T>
    result_type operator()(const T &value) const;
};

/** @short Variant visitor for converting from Deska::Db::ObjectModification to json_spirit::Value */
struct ObjectModificationToJsonValue: public boost::static_visitor<json_spirit::Value>
{
    template <typename T> result_type operator()(const T&) const;
};


/** @short Convert a json_spirit::Value to Deska::Value

No type information is checked.
*/
Value jsonValueToDeskaValue(const json_spirit::Value &v);

/** @short Define how to extract a custom JSON type into C++ class */
template<typename T> struct JsonExtractionTraits {};

#if 0
/** @short Specialization for extracting Identifiers from JSON */
template<> struct JsonExtractionTraits<Identifier> {
    static Identifier implementation(const json_spirit::Value &v) {
        JsonContext c1("When extracting Identifier");
        return v.get_str();
    }
};

/** @short Specialization for extracting a PendingChangeset representation from JSON */
template<> struct JsonExtractionTraits<PendingChangeset> {
    static PendingChangeset implementation(const json_spirit::Value &v) {
        JsonContext c1("When converting a JSON Value into a Deska::Db::PendingChangeset");
        JsonHandler h;
        TemporaryChangesetId changeset = TemporaryChangesetId::null;
        std::string author;
        boost::posix_time::ptime timestamp;
        RevisionId parentRevision = RevisionId::null;
        std::string message;
        PendingChangeset::AttachStatus attachStatus;
        boost::optional<std::string> activeConnectionInfo;
        h.read("changeset").extract(&changeset);
        h.read("author").extract(&author);
        h.read("timestamp").extract(&timestamp);
        h.read("parentRevision").extract(&parentRevision);
        h.read("message").extract(&message);
        h.read("status").extract(&attachStatus);
        h.read("activeConnectionInfo").extract(&activeConnectionInfo).isRequiredToReceive = false;
        h.parseJsonObject(v.get_obj());

        // These asserts are enforced by the JsonHandler, as all fields are required here.
        BOOST_ASSERT(changeset != TemporaryChangesetId::null);
        BOOST_ASSERT(parentRevision != RevisionId::null);
        // This is guaranteed by the extractor
        BOOST_ASSERT(attachStatus == PendingChangeset::ATTACH_DETACHED || attachStatus == PendingChangeset::ATTACH_IN_PROGRESS);

        return PendingChangeset(changeset, author, timestamp, parentRevision, message, attachStatus, activeConnectionInfo);
    }
};

/** @short Specialization for extracting ObjectRelation into a C++ class from JSON */
template<> struct JsonExtractionTraits<ObjectRelation> {
    static ObjectRelation implementation(const json_spirit::Value &v) {
        JsonContext c1("When converting JSON Object into Deska::Db::ObjectRelation");
        // At first, check just the "relation" field and ignore everything else. That will be used and checked later on.
        JsonHandler h;
        std::string relationKind;
        h.failOnUnknownFields(false);
        h.read("relation").extract(&relationKind);
        h.parseJsonObject(v.get_obj());

        // Got to re-initialize the handler, because it would otherwise claim that revision was already parsed
        h = JsonHandler();
        h.read("relation");

        // Now process the actual data
        if (relationKind == "EMBED_INTO") {
            std::string into;
            h.read("into").extract(&into);
            h.parseJsonObject(v.get_obj());
            return ObjectRelation::embedInto(into);
        } else if (relationKind == "IS_TEMPLATE") {
            std::string toWhichKind;
            h.read("toWhichKind").extract(&toWhichKind);
            h.parseJsonObject(v.get_obj());
            return ObjectRelation::isTemplate(toWhichKind);
        } else if (relationKind == "MERGE_WITH") {
            std::string targetTableName, sourceAttribute;
            h.read("targetTableName").extract(&targetTableName);
            h.read("sourceAttribute").extract(&sourceAttribute);
            h.parseJsonObject(v.get_obj());
            return ObjectRelation::mergeWith(targetTableName, sourceAttribute);
        } else if (relationKind == "TEMPLATIZED") {
            std::string byWhichKind, sourceAttribute;
            h.read("byWhichKind").extract(&byWhichKind);
            h.read("sourceAttribute").extract(&sourceAttribute);
            h.parseJsonObject(v.get_obj());
            return ObjectRelation::templatized(byWhichKind, sourceAttribute);
        } else {
            std::ostringstream s;
            s << "Invalid relation kind '" << relationKind << "'";
            throw JsonStructureError(s.str());
        }
    }
};

/** @short Extract RevisionMetadata from JSON */
template<> struct JsonExtractionTraits<RevisionMetadata> {
    static RevisionMetadata implementation(const json_spirit::Value &v) {
        JsonContext c1("When converting a JSON Value into a Deska::Db::RevisionMetadata");
        JsonHandler h;
        RevisionId revision = RevisionId::null;
        std::string author;
        boost::posix_time::ptime timestamp;
        std::string commitMessage;
        h.read("revision").extract(&revision);
        h.read("author").extract(&author);
        h.read("timestamp").extract(&timestamp);
        h.read("commitMessage").extract(&commitMessage);
        h.parseJsonObject(v.get_obj());
        BOOST_ASSERT(revision != RevisionId::null);
        return RevisionMetadata(revision, author, timestamp, commitMessage);
    }
};

/** @short Extract a RevisionId from JSON */
template<> struct JsonExtractionTraits<RevisionId> {
    static RevisionId implementation(const json_spirit::Value &v) {
        JsonContext c1("When extracting RevisionId");
        return RevisionId::fromJson(v.get_str());
    }
};

/** @short Extract a TemporaryChangesetId form JSON */
template<> struct JsonExtractionTraits<TemporaryChangesetId> {
    static TemporaryChangesetId implementation(const json_spirit::Value &v) {
        JsonContext c1("When extracting TemporaryChangesetId");
        return TemporaryChangesetId::fromJson(v.get_str());
    }
};

/** @short Extract timestamp from JSON */
template<> struct JsonExtractionTraits<boost::posix_time::ptime> {
    static boost::posix_time::ptime implementation(const json_spirit::Value &v) {
        JsonContext c1("When extracting boost::posix_time::ptime");
        return boost::posix_time::time_from_string(v.get_str());
    }
};

/** @short Extract PendingChangeset::AttachStatus from JSON */
template<> struct JsonExtractionTraits<PendingChangeset::AttachStatus> {
    static PendingChangeset::AttachStatus implementation(const json_spirit::Value &value) {
        JsonContext c1("When extracting Deska::Db::PendingChangeset::AttachStatus");
        if (value.type() != json_spirit::str_type)
            throw JsonStructureError("Value of expected type PendingChangesetAttachStatus is not a string");
        std::string data = value.get_str();
        if (data == "DETACHED") {
            return PendingChangeset::ATTACH_DETACHED;
        } else if (data == "INPROGRESS") {
            return PendingChangeset::ATTACH_IN_PROGRESS;
        } else {
            std::ostringstream ss;
            ss << "Invalid value for attached status of a pending changeset '" << data << "'";
            throw JsonStructureError(ss.str());
        }
    }
};

/** @short Helper for extracting an optional value */
template<typename T> struct JsonExtractionTraits<boost::optional<T> > {
    static boost::optional<T> implementation(const json_spirit::Value &v) {
        JsonContext c1("When extracting boost::optional<T>");
        if (v.type() == json_spirit::null_type)
            return boost::optional<T>();
        else
            return JsonExtractionTraits<T>::implementation(v);
    }
};

/** @short Helper for extracting a vector */
template<typename T> struct JsonExtractionTraits<std::vector<T> > {
    static std::vector<T> implementation(const json_spirit::Value &v) {
        JsonContext c1("When extracting std::vector<T>");
        std::vector<T> res;
        int i = 0;
        BOOST_FOREACH(const json_spirit::Value &item, v.get_array()) {
            JsonContext c2("When processing field #" + libebt::stringify(i));
            res.push_back(JsonExtractionTraits<T>::implementation(item));
        }
        return res;
    }
};

/** @short Helper for extracting attribute definitions */
template<> struct JsonExtractionTraits<std::map<Identifier,pair<Identifier,Value> > > {
    static std::map<Identifier,pair<Identifier,Value> > implementation(const json_spirit::Value &v) {
        JsonContext c1("When extracting std::map<Identifier,pair<Identifier,Value> >");
        std::map<Identifier,pair<Identifier,Value> > res;
        BOOST_FOREACH(const Pair &item, v.get_obj()) {
            JsonContext c2("When extracting attribute " + item.name_);
            if (item.value_.type() != json_spirit::array_type)
                throw JsonStructureError("Value of expected type (Identifier, Deska Value) is not an array");
            json_spirit::Array a = item.value_.get_array();
            if (a.size() != 2) {
                throw JsonStructureError("Value of expected type (Identifier, Deska Value) does not have exactly two records");
            }
            // FIXME: check type information for the attributes, and even attribute existence. This will require already cached kindAttributes()...
            res[item.name_] = std::make_pair(a[0].get_str(), jsonValueToDeskaValue(a[1]));
        }
        return res;
    }
};

/** @short Convert JSON into a vector of attribute data types

This one is special, as it arrives as a JSON object and not as a JSON list, hence we have to specialize and not use the generic vector extractor
*/
template<> struct JsonExtractionTraits<std::vector<KindAttributeDataType> > {
    static std::vector<KindAttributeDataType> implementation(const json_spirit::Value &v) {
        JsonContext c1("When extracting std::vector<KindAttributeDataType>");
        std::vector<KindAttributeDataType> res;
        BOOST_FOREACH(const Pair &item, v.get_obj()) {
            JsonContext c2("When handling attribute " + item.name_);
            if (item.value_.type() != json_spirit::str_type)
                throw JsonStructureError("Value of expected type Data Type is not string");
            std::string datatype = item.value_.get_str();
            if (datatype == "string") {
                res.push_back(KindAttributeDataType(item.name_, TYPE_STRING));
            } else if (datatype == "int") {
                res.push_back(KindAttributeDataType(item.name_, TYPE_INT));
            } else if (datatype == "identifier") {
                res.push_back(KindAttributeDataType(item.name_, TYPE_IDENTIFIER));
            } else if (datatype == "double") {
                res.push_back(KindAttributeDataType(item.name_, TYPE_DOUBLE));
            } else {
                std::ostringstream s;
                s << "Unsupported data type \"" << datatype << "\" for attribute \"" << item.name_ << "\"";
                throw JsonStructureError(s.str());
            }
        }
        return res;
    }
};

/** @short Extract ObjectModification from JSON */
template<> struct JsonExtractionTraits<ObjectModification> {
    static ObjectModification implementation(const json_spirit::Value &v) {
        JsonContext c1("When converting JSON Object into Deska::Db::ObjectModification");
        // At first, check just the "command" field and ignore everything else. That will be used and checked later on.
        JsonHandler h;
        std::string modificationKind;
        h.failOnUnknownFields(false);
        h.read("command").extract(&modificationKind);
        h.parseJsonObject(v.get_obj());

        // Got to re-initialize the handler, because it would otherwise claim that "command" was already parsed
        h = JsonHandler();
        h.read("command"); // and don't bother with extracting again

        // Now process the actual data
        if (modificationKind == "createObject") {
            JsonContext c2("When processing the createObject data");
            Identifier kindName, objectName;
            h.read("kindName").extract(&kindName);
            h.read("objectName").extract(&objectName);
            h.parseJsonObject(v.get_obj());
            return CreateObjectModification(kindName, objectName);
        } else if (modificationKind == "deleteObject") {
            JsonContext c2("When processing the deleteObject data");
            Identifier kindName, objectName;
            h.read("kindName").extract(&kindName);
            h.read("objectName").extract(&objectName);
            h.parseJsonObject(v.get_obj());
            return DeleteObjectModification(kindName, objectName);
        } else if (modificationKind == "renameObject") {
            JsonContext c2("When processing the renameObject data");
            Identifier kindName, oldObjectName, newObjectName;
            h.read("kindName").extract(&kindName);
            h.read("oldObjectName").extract(&oldObjectName);
            h.read("newObjectName").extract(&newObjectName);
            h.parseJsonObject(v.get_obj());
            return RenameObjectModification(kindName, oldObjectName, newObjectName);
        } else if (modificationKind == "removeAttribute") {
            JsonContext c2("When processing the removeAttribute data");
            Identifier kindName, objectName, attributeName;
            h.read("kindName").extract(&kindName);
            h.read("objectName").extract(&objectName);
            h.read("attributeName").extract(&attributeName);
            h.parseJsonObject(v.get_obj());
            return RemoveAttributeModification(kindName,objectName, attributeName);
        } else if (modificationKind == "setAttribute") {
            // FIXME: check and preserve the attribute data types here!
            JsonContext c2("When processing the setAttribute data");
            Identifier kindName, objectName, attributeName;
            Value attributeData, oldAttributeData;
            h.read("kindName").extract(&kindName);
            h.read("objectName").extract(&objectName);
            h.read("attributeName").extract(&attributeName);
            h.read("attributeData").extract(&attributeData);
            h.read("oldAttributeData").extract(&oldAttributeData);
            h.parseJsonObject(v.get_obj());
            return SetAttributeModification(kindName, objectName, attributeName, attributeData, oldAttributeData);
        } else {
            std::ostringstream s;
            s << "Invalid modification kind '" << modificationKind << "'";
            throw JsonStructureError(s.str());
        }
    }
};

template<> struct JsonExtractionTraits<Value> {
    static Value implementation(const json_spirit::Value &v) {
        // FIXME: remove me later
        JsonContext c1("When converting JSON Object into Deska::Db::Value");
        return jsonValueToDeskaValue(v);
    }
};
#endif

/** @short Abstract class for conversion between a JSON value and "something" */
class JsonExtractor
{
public:
    virtual ~JsonExtractor() {}
    /** @short Read the JSON data, convert them to the target form and store into a variable */
    virtual void extract(const json_spirit::Value &value) = 0;
};

/** @short Template class implementing the conversion from JSON to "something" */
template <typename T>
class SpecializedExtractor: public JsonExtractor
{
    T *target;
public:
    /** @short Create an extractor which will save the parsed and converted value to a pointer */
    SpecializedExtractor(T *source): target(source) {}
    virtual void extract(const json_spirit::Value &value);
};

}
}

#endif // DESKA_DB_JSON_EXTRACTION_H
