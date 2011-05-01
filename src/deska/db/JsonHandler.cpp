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

#include <boost/date_time/posix_time/time_parsers.hpp>
#include <boost/foreach.hpp>
// The Phoenix is rather prone to missing includes. The compilation is roughly 10% slower
// when including everything, but it's a worthwhile sacrifice, as it prevents many nasty
// errors which are rather hard to debug.
#include <boost/spirit/include/phoenix.hpp>
#include <boost/optional.hpp>
#include "JsonHandler.h"
#include "JsonApi.h"

using namespace std;
using json_spirit::Object;
using json_spirit::Pair;

static std::string j_command = "command";
static std::string j_response = "response";

namespace Deska {
namespace Db {

/** @short Variant visitor convert a Deska::Db::Value to json_spirit::Value */
struct DeskaValueToJsonValue: public boost::static_visitor<json_spirit::Value>
{
    /** @short Simply use json_spirit::Value's overloaded constructor */
    template <typename T>
    result_type operator()(const T &value) const
    {
        // A strange thing -- when the operator() is not const-qualified, it won't compile.
        // Too bad that the documentation doesn't mention that. Could it be related to the
        // fact that the variant we operate on is itself const? But why is there the
        // requirement to const-qualify the operator() and not only the value it reads?
        //
        // How come that this builds fine:
        // template <typename T>
        // result_type operator()(T &value) const
        return value;
    }
};

/** @short Variant visitor for converting from Deska::Db::ObjectModification to json_spirit::Value */
struct ObjectModificationToJsonValue: public boost::static_visitor<json_spirit::Value>
{
    template <typename T> result_type operator()(const T&) const;
};

template <>
ObjectModificationToJsonValue::result_type ObjectModificationToJsonValue::operator()(
    const Deska::Db::CreateObjectModification &value) const
{
    json_spirit::Object o;
    o.push_back(json_spirit::Pair("command", "createObject"));
    o.push_back(json_spirit::Pair("kindName", value.kindName));
    o.push_back(json_spirit::Pair("objectName", value.objectName));
    return o;
}

template <>
ObjectModificationToJsonValue::result_type ObjectModificationToJsonValue::operator()(
    const Deska::Db::DeleteObjectModification &value) const
{
    json_spirit::Object o;
    o.push_back(json_spirit::Pair("command", "deleteObject"));
    o.push_back(json_spirit::Pair("kindName", value.kindName));
    o.push_back(json_spirit::Pair("objectName", value.objectName));
    return o;
}

template <>
ObjectModificationToJsonValue::result_type ObjectModificationToJsonValue::operator()(
    const Deska::Db::RenameObjectModification &value) const
{
    json_spirit::Object o;
    o.push_back(json_spirit::Pair("command", "renameObject"));
    o.push_back(json_spirit::Pair("kindName", value.kindName));
    o.push_back(json_spirit::Pair("oldObjectName", value.oldObjectName));
    o.push_back(json_spirit::Pair("newObjectName", value.newObjectName));
    return o;
}

template <>
ObjectModificationToJsonValue::result_type ObjectModificationToJsonValue::operator()(
    const Deska::Db::RemoveAttributeModification &value) const
{
    json_spirit::Object o;
    o.push_back(json_spirit::Pair("command", "removeAttribute"));
    o.push_back(json_spirit::Pair("kindName", value.kindName));
    o.push_back(json_spirit::Pair("objectName", value.objectName));
    o.push_back(json_spirit::Pair("attributeName", value.attributeName));
    return o;
}

template <>
ObjectModificationToJsonValue::result_type ObjectModificationToJsonValue::operator()(
    const Deska::Db::SetAttributeModification &value) const
{
    json_spirit::Object o;
    o.push_back(json_spirit::Pair("command", "setAttribute"));
    o.push_back(json_spirit::Pair("kindName", value.kindName));
    o.push_back(json_spirit::Pair("objectName", value.objectName));
    o.push_back(json_spirit::Pair("attributeName", value.attributeName));
    o.push_back(json_spirit::Pair("attributeData",
                                  boost::apply_visitor(DeskaValueToJsonValue(), value.attributeData)));
    o.push_back(json_spirit::Pair("oldAttributeData",
                                  boost::apply_visitor(DeskaValueToJsonValue(), value.oldAttributeData)));
    return o;
}

/** @short Convert a json_spirit::Value to Deska::Value

No type information is checked.
*/
Value jsonValueToDeskaValue(const json_spirit::Value &v)
{
    if (v.type() == json_spirit::str_type) {
        return v.get_str();
    } else if (v.type() == json_spirit::int_type) {
        return v.get_int();
    } else if (v.type() == json_spirit::real_type) {
        return v.get_real();
    } else {
        throw JsonStructureError("Unsupported type of attribute data");
    }
}

/** @short Define how to extract a custom JSON type into C++ class */
template<typename T> struct JsonExtractionTraits {};

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
    virtual void extract(const json_spirit::Value &value)
    {
        JsonContext c1("In SpecializedExtractor<T>");
        *target = JsonExtractionTraits<T>::implementation(value);
    }
};


/** @short Convert JSON into a wrapped, type-checked object attributes

This function is special, as it needs access to already existing data; that's why it's implemented in this place,
unlike the generic way of going through the JsonExtractionTraits<T>.
*/
template<>
void SpecializedExtractor<JsonWrappedAttribute>::extract(const json_spirit::Value &value)
{
    BOOST_ASSERT(target);
    JsonContext c1("When extracting attribute " + target->attrName);
    switch (target->dataType) {
    case TYPE_STRING:
    case TYPE_IDENTIFIER:
        if (value.type() != json_spirit::str_type)
            throw JsonStructureError("Attribute value is not string");
        target->value = value.get_str();
        return;
    case TYPE_INT:
        if (value.type() != json_spirit::int_type)
            throw JsonStructureError("Attribute value is not an integer");
        target->value = value.get_int();
        return;
    case TYPE_DOUBLE:
        if (value.type() != json_spirit::real_type && value.type() != json_spirit::int_type)
            throw JsonStructureError("Attribute value is not a real");
        target->value = value.get_real();
        return;
    }
    std::ostringstream ss;
    ss << "Unsupported data type " << target->dataType;
    throw JsonStructureError(ss.str());
}

/** @short Convert JSON into a wrapped, type-checked vector of attributes

This function is special, as it needs access to already existing target in order to be able to read the type
information from somewhere. This means that we can't use the generic way of merely forwarding a call to
JsonExtractionTraits<T>::implementation because that copies stuff by value.
*/
template<>
void SpecializedExtractor<JsonWrappedAttributeMap>::extract(const json_spirit::Value &value)
{
    BOOST_ASSERT(target);
    JsonContext c1("When extracting attributes");
    JsonHandler h;
    std::vector<JsonWrappedAttribute> wrappedAttrs;

    // At first, allocate space for storing the values. We can't set up the extraction yet
    // because the vector might reallocate memory while it grows, leaving us with dangling pointers.
    BOOST_FOREACH(const KindAttributeDataType &attr, target->dataTypes) {
        wrappedAttrs.push_back(JsonWrappedAttribute(attr.type, attr.name));
    }

    // Set up the extractor so that it knows what fields to work with
    int i = 0;
    BOOST_FOREACH(const KindAttributeDataType &attr, target->dataTypes) {
        h.read(attr.name).extract(&wrappedAttrs[i]);
        ++i;
    }

    // Do the JSON parsing and verification
    h.parseJsonObject(value.get_obj());

    // Now copy the results back
    i = 0;
    BOOST_FOREACH(const KindAttributeDataType &attr, target->dataTypes) {
        target->attributes[attr.name] = wrappedAttrs[i].value;
        ++i;
    }
}



JsonField::JsonField(const std::string &name):
    isForSending(false), isRequiredToReceive(true), isAlreadyReceived(false), valueShouldMatch(false),
    jsonFieldRead(name), jsonFieldWrite(name)
{
}

/** @short Register this field for future extraction to the indicated location */
template<typename T>
JsonField &JsonField::extract(T *where)
{
    extractor.reset(new SpecializedExtractor<T>(where));
    return *this;
}

JsonHandlerApiWrapper::JsonHandlerApiWrapper(const JsonApiParser * const api, const std::string &cmd): p(api)
{
    command(cmd);
}

void JsonHandlerApiWrapper::send()
{
    JsonContext c1("When sending JSON data");
    json_spirit::Object o;
    BOOST_FOREACH(const JsonField &f, fields) {
        if (f.isForSending) {
            o.push_back(json_spirit::Pair(f.jsonFieldWrite, f.jsonValue));
        }
    }
    p->sendJsonObject(o);
}

void JsonHandlerApiWrapper::receive()
{
    JsonContext c1("When reading and processing JSON data");
    // FIXME: check for the j_errorPrefix here
    try {
        parseJsonObject(p->readJsonObject());
    } catch (JsonParseError &e) {
        if (boost::optional<std::string> rawJson = p->wantJustReadData())
            e.addRawJsonData(*rawJson);
        throw;
    }
}

void JsonHandlerApiWrapper::work()
{
    send();
    receive();
}

void JsonHandlerApiWrapper::command(const std::string &cmd)
{
    JsonField f(j_command);
    f.jsonFieldRead = j_response;
    f.jsonValue = cmd;
    f.isForSending = true;
    f.valueShouldMatch = true;
    fields.push_back(f);
}

JsonHandler::JsonHandler(): m_failOnUnknownFields(true)
{
}

JsonHandler::~JsonHandler()
{
}

void JsonHandler::failOnUnknownFields(const bool shouldThrow)
{
    m_failOnUnknownFields = shouldThrow;
}

void JsonHandler::parseJsonObject(const json_spirit::Object &jsonObject)
{
    using namespace boost::phoenix;
    using namespace arg_names;

    JsonContext c1("When parsing JSON object");

    BOOST_FOREACH(const Pair& node, jsonObject) {
        JsonContext c2("When checking JSON key " + node.name_);

        // At first, find a matching rule for this particular key
        std::vector<JsonField>::iterator rule =
                std::find_if(fields.begin(), fields.end(), bind(&JsonField::jsonFieldRead, arg1) == node.name_);

        if (rule == fields.end()) {
            // No such rule

            if (!m_failOnUnknownFields)
                continue;

            std::ostringstream s;
            s << "JSON field '" << node.name_ << "' is not allowed in this context (expecting one of:";
            BOOST_FOREACH(const JsonField &f, fields) {
                s << " " << f.jsonFieldRead;
            }
            s << ").";
            throw JsonStructureError(s.str());
        }

        if (rule->isAlreadyReceived) {
            // Duplicate rule
            std::ostringstream s;
            s << "Duplicate JSON field '" << node.name_ << "'";
            throw JsonStructureError(s.str());
        }

        // Check the value
        if (rule->valueShouldMatch) {
            // Oh yeah, json_spirit::Value doesn't implement operator!=. Well, at least it has operator== :).
            if (!(node.value_ == rule->jsonValue)) {
                std::ostringstream s;
                // FIXME: print the value here; this could be tricky as not all of them could be converted to string
                s << "JSON value mismatch for field '" << rule->jsonFieldRead << "'";
                throw JsonStructureError(s.str());
            }
        }

        // Extract the value from JSON
        if (rule->extractor) {
            JsonContext c3("When extracting the JSON value");
            rule->extractor->extract(node.value_);
        }

        // Mark this field as "processed"
        rule->isAlreadyReceived = true;
    }

    // Verify that each mandatory field was present
    std::vector<JsonField>::iterator rule =
            std::find_if(fields.begin(), fields.end(),
                         ! bind(&JsonField::isAlreadyReceived, arg1) && bind(&JsonField::isRequiredToReceive, arg1) );
    if ( rule != fields.end() ) {
        std::ostringstream s;
        s << "Mandatory field '" << rule->jsonFieldRead << "' not present in the response";
        throw JsonStructureError(s.str());
    }
}

JsonField &JsonHandler::write(const std::string &name, const std::string &value)
{
    JsonField f(name);
    f.jsonValue = value;
    f.isForSending = true;
    f.valueShouldMatch = true;
    fields.push_back(f);
    return *(--fields.end());
}

JsonField &JsonHandler::write(const std::string &name, const RevisionId value)
{
    JsonField f(name);
    std::ostringstream s;
    s << value;
    f.jsonValue = s.str();
    f.isForSending = true;
    f.valueShouldMatch = true;
    fields.push_back(f);
    return *(--fields.end());
}

JsonField &JsonHandler::write(const std::string &name, const TemporaryChangesetId value)
{
    JsonField f(name);
    std::ostringstream s;
    s << value;
    f.jsonValue = s.str();
    f.isForSending = true;
    f.valueShouldMatch = true;
    fields.push_back(f);
    return *(--fields.end());
}

JsonField &JsonHandler::write(const std::string &name, const Deska::Db::Value &value)
{
    JsonField f(name);
    f.jsonValue = boost::apply_visitor(DeskaValueToJsonValue(), value);
    f.isForSending = true;
    f.valueShouldMatch = true;
    fields.push_back(f);
    return *(--fields.end());
}

JsonField &JsonHandler::write(const std::string &name, const std::vector<Deska::Db::ObjectModification> &value)
{
    JsonField f(name);
    json_spirit::Array jsonArray;
    BOOST_FOREACH(const Deska::Db::ObjectModification &item, value) {
        jsonArray.push_back(boost::apply_visitor(ObjectModificationToJsonValue(), item));
    }
    f.jsonValue = jsonArray;
    f.isForSending = true;
    f.valueShouldMatch = true;
    fields.push_back(f);
    return *(--fields.end());
}

JsonField &JsonHandler::read(const std::string &name)
{
    JsonField f(name);
    fields.push_back(f);
    return *(--fields.end());
}

boost::optional<JsonField&> JsonHandler::writeIfNotZero(const std::string &name, const TemporaryChangesetId value)
{
    if (value == TemporaryChangesetId::null)
        return boost::optional<JsonField&>();
    else
        return write(name, value);
}

boost::optional<JsonField&> JsonHandler::writeIfNotZero(const std::string &name, const RevisionId value)
{
    if (value == RevisionId::null)
        return boost::optional<JsonField&>();
    else
        return write(name, value);
}

JsonWrappedAttributeMap::JsonWrappedAttributeMap(const std::vector<KindAttributeDataType> dataTypes_):
    dataTypes(dataTypes_)
{
}

JsonWrappedAttribute::JsonWrappedAttribute(const Type dataType_, const Identifier &attrName_):
    dataType(dataType_), attrName(attrName_)
{
}

// Template instances for the linker
template JsonField& JsonField::extract(RevisionId*);
template JsonField& JsonField::extract(TemporaryChangesetId*);
template JsonField& JsonField::extract(vector<Identifier>*);
template JsonField& JsonField::extract(vector<KindAttributeDataType>*);
template JsonField& JsonField::extract(vector<ObjectRelation>*);
template JsonField& JsonField::extract(map<Identifier,pair<Identifier,Value> >*);
template JsonField& JsonField::extract(boost::optional<std::string>*);
template JsonField& JsonField::extract(std::vector<PendingChangeset>*);
template JsonField& JsonField::extract(PendingChangeset::AttachStatus*);
template JsonField& JsonField::extract(boost::posix_time::ptime*);
template JsonField& JsonField::extract(JsonWrappedAttribute*);
template JsonField& JsonField::extract(JsonWrappedAttributeMap*);
template JsonField& JsonField::extract(std::vector<RevisionMetadata>*);
template JsonField& JsonField::extract(std::vector<ObjectModification>*);

}
}
