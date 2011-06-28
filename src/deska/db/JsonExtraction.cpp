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

#include <boost/algorithm/string/predicate.hpp>
#include <boost/date_time/posix_time/time_parsers.hpp>
#include <boost/date_time/posix_time/time_formatters.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include "JsonExtraction.h"
#include "JsonHandler.h"
#include "JsonApi.h"

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

/** @short Simply use json_spirit::Value's overloaded constructor */
template <typename T>
DeskaValueToJsonValue::result_type DeskaValueToJsonValue::operator()(const T &value) const
{
    // A strange thing -- when the operator() is not const-qualified, it won't compile.
    // Too bad that the documentation doesn't mention that. Could it be related to the
    // fact that the variant we operate on is itself const? But why is there the
    // requirement to const-qualify the operator() and not only the value it reads?
    //
    // How come that this builds fine:
    // template <typename T>
    // result_type operator()(T &value) const
    return JsonConversionTraits<T>::toJson(value);
}

// Template instances for the Deska::Db::Value conversions from JSON
template DeskaValueToJsonValue::result_type DeskaValueToJsonValue::operator()(const std::string &) const;
template DeskaValueToJsonValue::result_type DeskaValueToJsonValue::operator()(const int &) const;
template DeskaValueToJsonValue::result_type DeskaValueToJsonValue::operator()(const double &) const;

/** @short Convert a Deska::Db::Value into JSON */
template<>
json_spirit::Value JsonConversionTraits<Value>::toJson(const Value &value) {
    return value ? boost::apply_visitor(DeskaValueToJsonValue(), *value) : json_spirit::Value();
}

template<>
inline json_spirit::Value JsonConversionTraits<int>::toJson(const int &value) {
    return value;
}

template<>
inline json_spirit::Value JsonConversionTraits<double>::toJson(const double &value) {
    return value;
}

std::string jsonValueTypeToString(const json_spirit::Value_type type)
{
    switch (type) {
    case json_spirit::obj_type:
        return "obj_type";
    case json_spirit::array_type:
        return "array_type";
    case json_spirit::str_type:
        return "str_type";
    case json_spirit::bool_type:
        return "bool_type";
    case json_spirit::int_type:
        return "int_type";
    case json_spirit::real_type:
        return "real_type";
    case json_spirit::null_type:
        return "null_type";
    }
    std::ostringstream ss;
    ss << static_cast<int>(type);
    return ss.str();
}

void checkJsonValueType(const json_spirit::Value &v, const json_spirit::Value_type desiredType)
{
    if (v.type() != desiredType) {
        throw JsonStructureError("Expected JSON type " + jsonValueTypeToString(desiredType) + ", got " + jsonValueTypeToString(v.type())+ " instead");
    }
}

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
    const Deska::Db::SetAttributeModification &value) const
{
    json_spirit::Object o;
    o.push_back(json_spirit::Pair("command", "setAttribute"));
    o.push_back(json_spirit::Pair("kindName", value.kindName));
    o.push_back(json_spirit::Pair("objectName", value.objectName));
    o.push_back(json_spirit::Pair("attributeName", value.attributeName));
    o.push_back(json_spirit::Pair("attributeData", JsonConversionTraits<Value>::toJson(value.attributeData)));
    o.push_back(json_spirit::Pair("oldAttributeData", JsonConversionTraits<Value>::toJson(value.oldAttributeData)));
    return o;
}

/** @short Extract PendingChangeset::AttachStatus from JSON */
template<> struct JsonConversionTraits<PendingChangeset::AttachStatus> {
    static PendingChangeset::AttachStatus extract(const json_spirit::Value &value) {
        JsonContext c1("When extracting Deska::Db::PendingChangeset::AttachStatus");
        checkJsonValueType(value, json_spirit::str_type);
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

    static json_spirit::Value toJson(const PendingChangeset::AttachStatus &value) {
        switch (value) {
        case PendingChangeset::ATTACH_DETACHED:
            return std::string("DETACHED");
        case PendingChangeset::ATTACH_IN_PROGRESS:
            return std::string("INPROGRESS");
        }
        std::ostringstream ss;
        ss << "PendingChangeset::AttachStatus: value " << static_cast<int>(value) << " is out of range";
        throw domain_error(ss.str());
    }
};

/** @short Convert an IPv4 address to and from its JSON representation */
template<> struct JsonConversionTraits<boost::asio::ip::address_v4> {
    static boost::asio::ip::address_v4 extract(const json_spirit::Value &v) {
        JsonContext c1("When extracting boost::asio::ip::address_v4");
        checkJsonValueType(v, json_spirit::str_type);
        return boost::asio::ip::address_v4::from_string(v.get_str());
    }

    static json_spirit::Value toJson(const boost::asio::ip::address_v4 &value) {
        return value.to_string();
    }
};

/** @short Convert an IPv6 address to and from its JSON representation */
template<> struct JsonConversionTraits<boost::asio::ip::address_v6> {
    static boost::asio::ip::address_v6 extract(const json_spirit::Value &v) {
        JsonContext c1("When extracting boost::asio::ip::address_v6");
        checkJsonValueType(v, json_spirit::str_type);
        return boost::asio::ip::address_v6::from_string(v.get_str());
    }

    static json_spirit::Value toJson(const boost::asio::ip::address_v6 &value) {
        return value.to_string();
    }
};

/** @short Convert boost::grgorian::date to and from its JSON representation */
template<> struct JsonConversionTraits<boost::gregorian::date> {
    static boost::gregorian::date extract(const json_spirit::Value &v) {
        JsonContext c1("When extracting boost::gregorian::date");
        checkJsonValueType(v, json_spirit::str_type);
        return boost::gregorian::from_simple_string(v.get_str());
    }

    static json_spirit::Value toJson(const boost::gregorian::date &value) {
        return boost::gregorian::to_simple_string(value);
    }
};

/** @short Convert a MAC address to and from its JSON representation */
template<> struct JsonConversionTraits<Deska::Db::MacAddress> {
    static Deska::Db::MacAddress extract(const json_spirit::Value &v) {
        JsonContext c1("When extracting Deska::Db::MacAddress");
        checkJsonValueType(v, json_spirit::str_type);
        try {
            return Deska::Db::MacAddress(v.get_str());
        } catch (std::domain_error &e) {
            throw JsonStructureError(e.what());
        }
    }

    static json_spirit::Value toJson(const Deska::Db::MacAddress &value) {
        std::ostringstream ss;
        ss << value;
        return ss.str();
    }
};

/** @short Variant visitor for converting Deska::Db::MetadataValue to json_spirit::Value */
struct DeskaFilterMetadataValueToJsonValue: public boost::static_visitor<json_spirit::Value>
{
    template <typename T>
    result_type operator()(const T& value) const
    {
        return JsonConversionTraits<T>::toJson(value);
    }
};

template<> struct JsonConversionTraits<Deska::Db::ComparisonOperator> {
    static json_spirit::Value toJson(const Deska::Db::ComparisonOperator &value) {
        switch (value) {
        case FILTER_COLUMN_EQ:
            return std::string("columnEq");
        case FILTER_COLUMN_NE:
            return std::string("columnNe");
        case FILTER_COLUMN_GT:
            return std::string("columnGt");
        case FILTER_COLUMN_GE:
            return std::string("columnGe");
        case FILTER_COLUMN_LT:
            return std::string("columnLt");
        case FILTER_COLUMN_LE:
            return std::string("columnLe");
        }
        throw std::domain_error("Value of Deska::Db::ExpressionKind is out of bounds");
    }
};

struct DeskaFilterExpressionToJsonValue: public boost::static_visitor<json_spirit::Value>
{
    result_type operator()(const Deska::Db::MetadataExpression &expression) const
    {
        json_spirit::Object o;
        o.push_back(json_spirit::Pair("condition", JsonConversionTraits<ComparisonOperator>::toJson(expression.comparison)));
        o.push_back(json_spirit::Pair("metadata", expression.metadata));
        o.push_back(json_spirit::Pair("value", boost::apply_visitor(DeskaFilterMetadataValueToJsonValue(), expression.constantValue)));
        return o;
    }

    result_type operator()(const Deska::Db::AttributeExpression &expression) const
    {
        json_spirit::Object o;
        o.push_back(json_spirit::Pair("condition", JsonConversionTraits<ComparisonOperator>::toJson(expression.comparison)));
        o.push_back(json_spirit::Pair("kind", expression.kind));
        o.push_back(json_spirit::Pair("attribute", expression.attribute));
        o.push_back(json_spirit::Pair("value", JsonConversionTraits<Value>::toJson(expression.constantValue)));
        return o;
    }
};

template <>
DeskaFilterToJsonValue::result_type DeskaFilterToJsonValue::operator()(const Deska::Db::AndFilter &filter) const
{
    json_spirit::Array a;
    BOOST_FOREACH(const Deska::Db::Filter &element, filter.operands) {
        a.push_back(boost::apply_visitor(DeskaFilterToJsonValue(), element));
    }
    json_spirit::Object o;
    o.push_back(json_spirit::Pair("operator", json_spirit::Value("and")));
    o.push_back(json_spirit::Pair("operands", a));
    return o;
};

template <>
DeskaFilterToJsonValue::result_type DeskaFilterToJsonValue::operator()(const Deska::Db::OrFilter &filter) const
{
    json_spirit::Array a;
    BOOST_FOREACH(const Deska::Db::Filter &element, filter.operands) {
        a.push_back(boost::apply_visitor(DeskaFilterToJsonValue(), element));
    }
    json_spirit::Object o;
    o.push_back(json_spirit::Pair("operator", json_spirit::Value("or")));
    o.push_back(json_spirit::Pair("operands", a));
    return o;
};

template <>
DeskaFilterToJsonValue::result_type DeskaFilterToJsonValue::operator()(const Deska::Db::Expression &expression) const
{
    return boost::apply_visitor(DeskaFilterExpressionToJsonValue(), expression);
};

/** @short Specialization for extracting Identifiers from JSON */
template<>
Identifier JsonConversionTraits<Identifier>::extract(const json_spirit::Value &v) {
    JsonContext c1("When extracting Identifier");
    checkJsonValueType(v, json_spirit::str_type);
    return v.get_str();
}

/** @short Specialization for extracting a PendingChangeset representation from JSON */
template<>
PendingChangeset JsonConversionTraits<PendingChangeset>::extract(const json_spirit::Value &v) {
    JsonContext c1("When converting a JSON Value into a Deska::Db::PendingChangeset");
    JsonHandler h;
    TemporaryChangesetId changeset(0);
    std::string author;
    boost::posix_time::ptime timestamp;
    RevisionId parentRevision(0);
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

    // This is guaranteed by the extractor
    BOOST_ASSERT(attachStatus == PendingChangeset::ATTACH_DETACHED || attachStatus == PendingChangeset::ATTACH_IN_PROGRESS);

    return PendingChangeset(changeset, author, timestamp, parentRevision, message, attachStatus, activeConnectionInfo);
}

/** @short Specialization for extracting ObjectRelation into a C++ class from JSON */
template<>
ObjectRelation JsonConversionTraits<ObjectRelation>::extract(const json_spirit::Value &v) {
    JsonContext c1("When converting JSON Object into Deska::Db::ObjectRelation");
    JsonHandler h;
    std::string relationKind;
    std::string target;
    h.read("relation").extract(&relationKind);
    h.read("target").extract(&target);
    checkJsonValueType(v, json_spirit::obj_type);
    h.parseJsonObject(v.get_obj());

    // Now process the actual data
    if (relationKind == "EMBED_INTO") {
        return ObjectRelation::embedInto(target);
    } else if (relationKind == "IS_TEMPLATE") {
        return ObjectRelation::isTemplate(target);
    } else if (relationKind == "MERGE_WITH") {
        return ObjectRelation::mergeWith(target);
    } else if (relationKind == "REFERS_TO") {
        return ObjectRelation::refersTo(target);
    } else if (relationKind == "TEMPLATIZED") {
        return ObjectRelation::templatized(target);
    } else {
        std::ostringstream s;
        s << "Invalid relation kind '" << relationKind << "'";
        throw JsonStructureError(s.str());
    }
}

/** @short Extract RevisionMetadata from JSON */
template<>
RevisionMetadata JsonConversionTraits<RevisionMetadata>::extract(const json_spirit::Value &v) {
    JsonContext c1("When converting a JSON Value into a Deska::Db::RevisionMetadata");
    JsonHandler h;
    RevisionId revision(0);
    std::string author;
    boost::posix_time::ptime timestamp;
    std::string commitMessage;
    h.read("revision").extract(&revision);
    h.read("author").extract(&author);
    h.read("timestamp").extract(&timestamp);
    h.read("commitMessage").extract(&commitMessage);
    checkJsonValueType(v, json_spirit::obj_type);
    h.parseJsonObject(v.get_obj());
    return RevisionMetadata(revision, author, timestamp, commitMessage);
}

template<typename T> T extractRevisionFromJson(const std::string &prefix, const std::string &name, const std::string &jsonStr)
{
    if (boost::starts_with(jsonStr, prefix)) {
        try {
            return T(boost::lexical_cast<unsigned int>(jsonStr.substr(prefix.size())));
        } catch (const boost::bad_lexical_cast &) {
            std::ostringstream s;
            s << "Value \"" << jsonStr << "\" can't be interpreted as a " << name << ".";
            throw JsonStructureError(s.str());
        }
    } else {
        std::ostringstream s;
        s << "Value \"" << jsonStr << "\" does not look like a valid " << name << ".";
        throw JsonStructureError(s.str());
    }

}

/** @short Extract a RevisionId from JSON */
template<>
RevisionId JsonConversionTraits<RevisionId>::extract(const json_spirit::Value &v) {
    JsonContext c1("When extracting RevisionId");
    checkJsonValueType(v, json_spirit::str_type);
    return extractRevisionFromJson<RevisionId>("r", "RevisionId", v.get_str());
}

/** @short Extract a TemporaryChangesetId form JSON */
template<>
TemporaryChangesetId JsonConversionTraits<TemporaryChangesetId>::extract(const json_spirit::Value &v)
{
    JsonContext c1("When extracting TemporaryChangesetId");
    checkJsonValueType(v, json_spirit::str_type);
    return extractRevisionFromJson<TemporaryChangesetId>("tmp", "TemporaryChangesetId", v.get_str());
}

/** @short Extract timestamp from JSON */
template<>
boost::posix_time::ptime JsonConversionTraits<boost::posix_time::ptime>::extract(const json_spirit::Value &v)
{
    JsonContext c1("When extracting boost::posix_time::ptime");
    checkJsonValueType(v, json_spirit::str_type);
    return boost::posix_time::time_from_string(v.get_str());
}

/** @short Convert timestamp to JSON */
template<>
json_spirit::Value JsonConversionTraits<boost::posix_time::ptime>::toJson(const boost::posix_time::ptime &value)
{
    return boost::posix_time::to_simple_string(value);
}

/** @short Convert boolean to JSON */
template<>
json_spirit::Value JsonConversionTraits<bool>::toJson(const bool &value)
{
    return value;
}

/** @short Helper for extracting an optional value */
template<typename T> struct JsonConversionTraits<boost::optional<T> > {
    static boost::optional<T> extract(const json_spirit::Value &v) {
        JsonContext c1("When extracting boost::optional<T>");
        if (v.type() == json_spirit::null_type)
            return boost::optional<T>();
        else
            return JsonConversionTraits<T>::extract(v);
    }
};

/** @short Helper for extracting a vector */
template<typename T> struct JsonConversionTraits<std::vector<T> > {
    static std::vector<T> extract(const json_spirit::Value &v) {
        JsonContext c1("When extracting std::vector<T>");
        std::vector<T> res;
        int i = 0;
        checkJsonValueType(v, json_spirit::array_type);
        BOOST_FOREACH(const json_spirit::Value &item, v.get_array()) {
            JsonContext c2("When processing field #" + libebt::stringify(i));
            res.push_back(JsonConversionTraits<T>::extract(item));
        }
        return res;
    }
};

/** @short Convert JSON into a vector of attribute data types

This one is special, as it arrives as a JSON object and not as a JSON list, hence we have to specialize and not use the generic vector extractor
*/
template<> struct JsonConversionTraits<std::vector<KindAttributeDataType> > {
    static std::vector<KindAttributeDataType> extract(const json_spirit::Value &v) {
        JsonContext c1("When extracting std::vector<KindAttributeDataType>");
        std::vector<KindAttributeDataType> res;
        checkJsonValueType(v, json_spirit::obj_type);
        BOOST_FOREACH(const Pair &item, v.get_obj()) {
            JsonContext c2("When handling attribute " + item.name_);
            checkJsonValueType(item.value_, json_spirit::str_type);
            std::string datatype = item.value_.get_str();
            if (datatype == "string") {
                res.push_back(KindAttributeDataType(item.name_, TYPE_STRING));
            } else if (datatype == "int") {
                res.push_back(KindAttributeDataType(item.name_, TYPE_INT));
            } else if (datatype == "identifier") {
                res.push_back(KindAttributeDataType(item.name_, TYPE_IDENTIFIER));
            } else if (datatype == "double") {
                res.push_back(KindAttributeDataType(item.name_, TYPE_DOUBLE));
            } else if (datatype == "macaddress") {
                res.push_back(KindAttributeDataType(item.name_, TYPE_MAC_ADDRESS));
            } else if (datatype == "ipv4address") {
                res.push_back(KindAttributeDataType(item.name_, TYPE_IPV4_ADDRESS));
            } else if (datatype == "ipv6address") {
                res.push_back(KindAttributeDataType(item.name_, TYPE_IPV6_ADDRESS));
            } else if (datatype == "timestamp") {
                res.push_back(KindAttributeDataType(item.name_, TYPE_TIMESTAMP));
            } else if (datatype == "date") {
                res.push_back(KindAttributeDataType(item.name_, TYPE_DATE));
            } else {
                std::ostringstream s;
                s << "Unsupported data type \"" << datatype << "\" for attribute \"" << item.name_ << "\"";
                throw JsonStructureError(s.str());
            }
        }
        return res;
    }
};

template<>
void SpecializedExtractor<JsonWrappedObjectModification>::extract(const json_spirit::Value &v)
{
    BOOST_ASSERT(target);
    JsonContext c1("When converting JSON Object into Deska::Db::ObjectModification");
    // At first, check just the "command" field and ignore everything else. That will be used and checked later on.
    JsonHandler h;
    std::string modificationKind;
    h.failOnUnknownFields(false);
    h.read("command").extract(&modificationKind);
    checkJsonValueType(v, json_spirit::obj_type);
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
        target->diff = CreateObjectModification(kindName, objectName);
    } else if (modificationKind == "deleteObject") {
        JsonContext c2("When processing the deleteObject data");
        Identifier kindName, objectName;
        h.read("kindName").extract(&kindName);
        h.read("objectName").extract(&objectName);
        h.parseJsonObject(v.get_obj());
        target->diff = DeleteObjectModification(kindName, objectName);
    } else if (modificationKind == "renameObject") {
        JsonContext c2("When processing the renameObject data");
        Identifier kindName, oldObjectName, newObjectName;
        h.read("kindName").extract(&kindName);
        h.read("oldObjectName").extract(&oldObjectName);
        h.read("newObjectName").extract(&newObjectName);
        h.parseJsonObject(v.get_obj());
        target->diff = RenameObjectModification(kindName, oldObjectName, newObjectName);
    } else if (modificationKind == "setAttribute") {
        // FIXME: check and preserve the attribute data types here!
        JsonContext c2("When processing the setAttribute data");
        Identifier kindName, objectName, attributeName;
        h.read("kindName").extract(&kindName);
        h.read("objectName").extract(&objectName);
        h.read("attributeName").extract(&attributeName);
        h.failOnUnknownFields(false);
        h.parseJsonObject(v.get_obj());

        JsonContext c3("When processing the setAttribute data and checking type information");

        // Got to re-init it again; don't bother with extratcing them, though
        h = JsonHandler();
        h.read("command");
        h.read("kindName");
        h.read("objectName");
        h.read("attributeName");

        JsonWrappedAttribute attributeData = target->wrappedAttribute(kindName, attributeName);
        JsonWrappedAttribute oldAttributeData = target->wrappedAttribute(kindName, attributeName);
        h.read("attributeData").extract(&attributeData);
        h.read("oldAttributeData").extract(&oldAttributeData);
        h.parseJsonObject(v.get_obj());
        target->diff = SetAttributeModification(kindName, objectName, attributeName, attributeData.value, oldAttributeData.value);
    } else {
        std::ostringstream s;
        s << "Invalid modification kind '" << modificationKind << "'";
        throw JsonStructureError(s.str());
    }
}

/** @short Hack: use the JSON conversion traits for parsing of "remote errors"

The hack part of this approach is that we can't safely return a whole class here (slicing would happen),
so we instead make that function return void and throw errors around.
*/
void JsonConversionTraits<RemoteDbError>::extract(const json_spirit::Value &v)
{
        JsonContext c1("When checking for presence of a RemoteDbError");
        // At first, check just the kind of the exception
        JsonHandler h;
        std::string exceptionClass;
        h.failOnUnknownFields(false);
        h.read("type").extract(&exceptionClass);
        checkJsonValueType(v, json_spirit::obj_type);
        h.parseJsonObject(v.get_obj());

        // Now re-initialize the JSON handler and throw an exception matching the server's response
        h = JsonHandler();
        h.read("type"); // throw away
        // "message" is said to be shared by all server-side errors
        std::string message;
        h.read("message").extract(&message);

#define DESKA_CATCH_REMOTE_EXCEPTION(X) \
    if (exceptionClass == #X ) { \
        JsonContext c2("When parsing " #X); \
        h.parseJsonObject(v.get_obj()); \
        throw X(message); \
    }
        DESKA_CATCH_REMOTE_EXCEPTION(ServerError)
        else DESKA_CATCH_REMOTE_EXCEPTION(NotFoundError)
        else DESKA_CATCH_REMOTE_EXCEPTION(InvalidKindError)
        else DESKA_CATCH_REMOTE_EXCEPTION(InvalidAttributeError)
        else DESKA_CATCH_REMOTE_EXCEPTION(NoChangesetError)
        else DESKA_CATCH_REMOTE_EXCEPTION(ChangesetAlreadyOpenError)
        else DESKA_CATCH_REMOTE_EXCEPTION(FilterError)
        else DESKA_CATCH_REMOTE_EXCEPTION(SqlError)
        else DESKA_CATCH_REMOTE_EXCEPTION(ReCreateObjectError)
        else DESKA_CATCH_REMOTE_EXCEPTION(RevisionParsingError)
        else DESKA_CATCH_REMOTE_EXCEPTION(ChangesetParsingError)
        else {
            // Unsupported/unknown/invalid/... class of exception
            JsonContext c2("When parsing an unknown server-side exception");
            // We don't have any idea about this exception, so be future-proof and allow optional arguments here
            h.failOnUnknownFields(false);
            h.parseJsonObject(v.get_obj());
            // let's consider this a protocol error for now
            std::ostringstream ss;
            ss << "Unknown class of server-side exception '" << exceptionClass << "'. Server's message: " << message;
            throw JsonStructureError(ss.str());
        }
#undef DESKA_CATCH_REMOTE_EXCEPTION
};

template <typename T>
void SpecializedExtractor<T>::extract(const json_spirit::Value &value)
{
    JsonContext c1("In SpecializedExtractor<T>");
    *target = JsonConversionTraits<T>::extract(value);
}

/** @short Convert JSON into a wrapped, type-checked object attributes

This function is special, as it needs access to already existing data; that's why it's implemented in this place,
unlike the generic way of going through the JsonConversionTraits<T>.
*/
template<>
void SpecializedExtractor<JsonWrappedAttribute>::extract(const json_spirit::Value &value)
{
    BOOST_ASSERT(target);
    JsonContext c1("When extracting attribute " + target->attrName);
    if (value.is_null()) {
        target->value = Deska::Db::Value();
        return;
    }
    switch (target->dataType) {
    case TYPE_STRING:
    case TYPE_IDENTIFIER:
    {
        JsonContext c2("When extracting TYPE_STRING or TYPE_IDENTIFIER");
        checkJsonValueType(value, json_spirit::str_type);
        target->value = value.get_str();
        return;
    }
    case TYPE_INT:
    {
        JsonContext c2("When extracting TYPE_INT");
        checkJsonValueType(value, json_spirit::int_type);
        target->value = value.get_int();
        return;
    }
    case TYPE_DOUBLE:
    {
        JsonContext c2("When extracting TYPE_DOUBLE");
        // got to preserve our special case for checking for both possibilities
        if (value.type() != json_spirit::real_type && value.type() != json_spirit::int_type)
            throw JsonStructureError("Attribute value is not a real");
        target->value = value.get_real();
        return;
    }
    case TYPE_IPV4_ADDRESS:
    {
        JsonContext c2("When extracting TYPE_IPV4_ADDRES");
        target->value = JsonConversionTraits<boost::asio::ip::address_v4>::extract(value);
        return;
    }
    case TYPE_IPV6_ADDRESS:
    {
        JsonContext c2("When extracting TYPE_IPV6_ADDRES");
        target->value = JsonConversionTraits<boost::asio::ip::address_v6>::extract(value);
        return;
    }
    case TYPE_MAC_ADDRESS:
    {
        JsonContext c2("When extracting TYPE_MAC_ADDRESS");
        target->value = JsonConversionTraits<Deska::Db::MacAddress>::extract(value);
        return;
    }
    case TYPE_DATE:
    {
        JsonContext c2("When extracting TYPE_DATE");
        target->value = JsonConversionTraits<boost::gregorian::date>::extract(value);
        return;
    }
    case TYPE_TIMESTAMP:
    {
        JsonContext c2("When extracting TYPE_TIMESTAMP");
        target->value = JsonConversionTraits<boost::posix_time::ptime>::extract(value);
        return;
    }
    }
    std::ostringstream ss;
    ss << "Unsupported data type " << target->dataType;
    throw JsonStructureError(ss.str());
}

/** @short Convert JSON into a wrapped, type-checked object attributes

Similar to SpecializedExtractor<JsonWrappedAttribute>::extract, this function has to be special because it needs certain
pre-existing information to be available in the target member; that's the only way to retrieve type information about the
supported attributes.

@see SpecializedExtractor<JsonWrappedAttribute>::extract
*/
template<>
void SpecializedExtractor<JsonWrappedAttributeWithOrigin>::extract(const json_spirit::Value &value)
{
    BOOST_ASSERT(target);
    JsonContext c1("When extracting attribute " + target->attrName + " with origin information");
    checkJsonValueType(value, json_spirit::array_type);
    json_spirit::Array a = value.get_array();
    if (a.size() != 2) {
        throw JsonStructureError("Tuple of (origin, value) has length != 2");
    }

    // Construct a specialized extractor for our base type, which will deal with actually extracting the data
    SpecializedExtractor<JsonWrappedAttribute> helperExtractor(target);
    helperExtractor.extract(a[1]);

    // Now file in the origin information
    target->origin = a[0].get_str();
}

/** @short Convert JSON into a wrapped, type-checked vector of attributes

This function is special, as it needs access to already existing target in order to be able to read the type
information from somewhere. This means that we can't use the generic way of merely forwarding a call to
JsonConversionTraits<T>::extract because that copies stuff by value.
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
    // As a side-effect, all of the attributes are marked as required to be present

    checkJsonValueType(value, json_spirit::obj_type);
    // Do the JSON parsing and verification
    h.parseJsonObject(value.get_obj());

    // Now copy the results back
    i = 0;
    BOOST_FOREACH(const KindAttributeDataType &attr, target->dataTypes) {
        target->attributes[attr.name] = wrappedAttrs[i].value;
        ++i;
    }
}

/** @short Convert JSON into a wrapped, type-checked structure for multipleObjectData

See SpecializedExtractor<JsonWrappedAttributeMap>::extract for why we need a special function here and
JsonConversionTraits<T>::extract is not enough.
*/
template<>
void SpecializedExtractor<JsonWrappedAttributeMapList>::extract(const json_spirit::Value &value)
{
    BOOST_ASSERT(target);
    JsonContext c1("When extracting list of object attributes");
    checkJsonValueType(value, json_spirit::obj_type);
    BOOST_FOREACH(const Pair item, value.get_obj()) {
        JsonContext c2("When handling attributes for object named " + item.name_);
        JsonWrappedAttributeMap tmp(target->dataTypes);
        SpecializedExtractor<JsonWrappedAttributeMap> extractor(&tmp);
        extractor.extract(item.value_);
        target->objects[item.name_] = tmp.attributes;
    }
}

/** @short Convert JSON into a wrapped, type-checked vector of attributes

Thie functions extends funcitonality provided by the SpecializedExtractor<JsonWrappedAttributeMap>::extract with tracking of the
"origin" information, ie. what object has defined the current value.

@see SpecializedExtractor<JsonWrappedAttributeMap>::extract
*/
template<>
void SpecializedExtractor<JsonWrappedAttributeMapWithOrigin>::extract(const json_spirit::Value &value)
{
    BOOST_ASSERT(target);
    JsonContext c1("When extracting attributes");
    JsonHandler h;
    std::vector<JsonWrappedAttributeWithOrigin> wrappedAttrs;

    // For details about how this function works, please see SpecializedExtractor<JsonWrappedAttributeMap>::extract
    BOOST_FOREACH(const KindAttributeDataType &attr, target->dataTypes) {
        wrappedAttrs.push_back(JsonWrappedAttributeWithOrigin(attr.type, attr.name));
    }

    int i = 0;
    BOOST_FOREACH(const KindAttributeDataType &attr, target->dataTypes) {
        h.read(attr.name).extract(&wrappedAttrs[i]);
        ++i;
    }

    checkJsonValueType(value, json_spirit::obj_type);
    h.parseJsonObject(value.get_obj());

    i = 0;
    BOOST_FOREACH(const KindAttributeDataType &attr, target->dataTypes) {
        // This is slightly different than the SpecializedExtractor<JsonWrappedAttributeMap>::extract
        // in order to accomodate the difference in object layout
        target->attributes[attr.name] = std::make_pair<Identifier, Value>(wrappedAttrs[i].origin, wrappedAttrs[i].value);
        ++i;
    }
}

/** @short Convert JSON into a wrapped, type-checked structure for multipleResolvedObjectData()

@see SpecializedExtractor<JsonWrappedAttributeMapWithOrigin>::extract
@see SpecializedExtractor<JsonWrappedAttributeMapList>::extract
*/
template<>
void SpecializedExtractor<JsonWrappedAttributeMapWithOriginList>::extract(const json_spirit::Value &value)
{
    BOOST_ASSERT(target);
    JsonContext c1("When extracting list of resolved object attributes");
    checkJsonValueType(value, json_spirit::obj_type);
    BOOST_FOREACH(const Pair item, value.get_obj()) {
        JsonContext c2("When handling resolved attributes for object named " + item.name_);
        JsonWrappedAttributeMapWithOrigin tmp(target->dataTypes);
        SpecializedExtractor<JsonWrappedAttributeMapWithOrigin> extractor(&tmp);
        extractor.extract(item.value_);
        target->objects[item.name_] = tmp.attributes;
    }
}


template<>
void SpecializedExtractor<JsonWrappedObjectModificationSequence>::extract(const json_spirit::Value &value)
{
    BOOST_ASSERT(target);
    JsonContext c1("When extracting a list of differences");
    int i = 0;
    checkJsonValueType(value, json_spirit::array_type);
    BOOST_FOREACH(const json_spirit::Value &item, value.get_array()) {
        JsonContext c2("When processing diff item #" + libebt::stringify(i));
        JsonWrappedObjectModification currentDestination(target->dataTypesOfEverything);
        SpecializedExtractor<JsonWrappedObjectModification> helper(&currentDestination);
        helper.extract(item);
        target->diff.push_back(*(currentDestination.diff));
    }
}

JsonField::JsonField(const std::string &name):
    isForSending(false), isRequiredToReceive(true), isAllowedToReceive(true), isAlreadyReceived(false), valueShouldMatch(false),
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

// Template instances for the linker
template JsonField& JsonField::extract(RevisionId*);
template JsonField& JsonField::extract(TemporaryChangesetId*);
template JsonField& JsonField::extract(std::vector<Identifier>*);
template JsonField& JsonField::extract(std::vector<KindAttributeDataType>*);
template JsonField& JsonField::extract(std::vector<ObjectRelation>*);
template JsonField& JsonField::extract(boost::optional<std::string>*);
template JsonField& JsonField::extract(std::vector<PendingChangeset>*);
template JsonField& JsonField::extract(PendingChangeset::AttachStatus*);
template JsonField& JsonField::extract(boost::posix_time::ptime*);
template JsonField& JsonField::extract(JsonWrappedAttributeMap*);
template JsonField& JsonField::extract(JsonWrappedAttributeMapList*);
template JsonField& JsonField::extract(JsonWrappedAttributeMapWithOrigin*);
template JsonField& JsonField::extract(JsonWrappedAttributeMapWithOriginList*);
template JsonField& JsonField::extract(std::vector<RevisionMetadata>*);
template JsonField& JsonField::extract(JsonWrappedObjectModificationSequence*);

}
}
