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
#include "json_spirit/json_spirit_writer_template.h"
#include "JsonExtraction.h"
#include "JsonHandler.h"

using namespace std;
using json_spirit::Object;
using json_spirit::Pair;

static std::string j_command = "command";
static std::string j_response = "response";

namespace Deska {
namespace Db {

JsonHandlerApiWrapper::JsonHandlerApiWrapper(const JsonApiParser * const api, const std::string &cmd): p(api)
{
    command(cmd);
    // FIXME: get the tag form the Api*
    write(std::string("tag"), std::string("T"));
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
    json_spirit::Object obj;

    // At first, parse the underlying string into a JSON representation...
    try {
        obj = p->readJsonObject();
    } catch (JsonParseError &e) {
        if (boost::optional<std::string> rawJson = p->wantJustReadData())
            e.addRawJsonData(*rawJson);
        throw;
    }

    processPossibleException(obj);

    // Now convert it into the class that we expect
    try {
        parseJsonObject(obj);
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

void JsonHandlerApiWrapper::processPossibleException(const json_spirit::Object &jsonObject)
{
    BOOST_FOREACH(const json_spirit::Pair &node, jsonObject) {
        JsonContext c1("When checking JSON key " + node.name_);
        if (node.name_ != "dbException")
            continue;

        try {
            // This will throw the exception
            JsonConversionTraits<RemoteDbError>::extract(node.value_);
        } catch (RemoteDbError &e) {
            // ...but unfortunately it won't include raw JSON data, that's why we add that here
            if (boost::optional<std::string> rawJson = p->wantJustReadData()) {
                e.setRawResponseData(*rawJson);
            }
            throw;
        }
    }
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

        if (!rule->isAllowedToReceive) {
            // Just for sending
            std::ostringstream s;
            s << "JSON field '" << node.name_ << "' is just for sending, and not supposed to be read back";
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
                s << "JSON value mismatch for field '" << rule->jsonFieldRead << "'. Expected ";
                json_spirit::write_stream(node.value_, s);
                s << ", got ";
                json_spirit::write_stream(rule->jsonValue, s);
                s << " instead.";
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

template<typename T>
JsonField &JsonHandler::write(const std::string &name, const T &value)
{
    JsonField f(name);
    f.jsonValue = JsonConversionTraits<T>::toJson(value);
    f.isForSending = true;
    f.valueShouldMatch = true;
    fields.push_back(f);
    return *(--fields.end());
}

template<>
JsonField &JsonHandler::write(const std::string &name, const std::vector<Deska::Db::ObjectModificationCommand> &value)
{
    JsonField f(name);
    json_spirit::Array jsonArray;
    BOOST_FOREACH(const Deska::Db::ObjectModificationCommand &item, value) {
        jsonArray.push_back(boost::apply_visitor(ObjectModificationCommandToJsonValue(), item));
    }
    f.jsonValue = jsonArray;
    f.isForSending = true;
    f.valueShouldMatch = true;
    fields.push_back(f);
    return *(--fields.end());
}

template<>
JsonField &JsonHandler::write(const std::string &name, const Deska::Db::Filter &filter)
{
    JsonField f(name);
    f.jsonValue = boost::apply_visitor(DeskaFilterToJsonValue(), filter);
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

template<typename T>
JsonField &JsonHandlerApiWrapper::argument(const std::string &name, const T &value)
{
    JsonField &f = JsonHandler::write(name, value);
    f.isRequiredToReceive = false;
    f.valueShouldMatch = false;
    f.isAllowedToReceive = false;
    return f;
}


JsonWrappedAttributeMap::JsonWrappedAttributeMap(const std::vector<KindAttributeDataType> dataTypes_):
    dataTypes(dataTypes_)
{
}

JsonWrappedAttributeMapList::JsonWrappedAttributeMapList(const std::vector<KindAttributeDataType> dataTypes_):
    dataTypes(dataTypes_)
{
}

JsonWrappedAttributeMapWithOrigin::JsonWrappedAttributeMapWithOrigin(const std::vector<KindAttributeDataType> dataTypes_):
    dataTypes(dataTypes_)
{
}

JsonWrappedAttributeMapWithOriginList::JsonWrappedAttributeMapWithOriginList(const std::vector<KindAttributeDataType> dataTypes_):
    dataTypes(dataTypes_)
{
}

JsonWrappedAttribute::JsonWrappedAttribute(const Type dataType_, const Identifier &attrName_):
    dataType(dataType_), attrName(attrName_)
{
}

JsonWrappedAttributeWithOptionalOrigin::JsonWrappedAttributeWithOptionalOrigin(const Type dataType_, const Identifier &attrName_):
    JsonWrappedAttribute(dataType_, attrName_)
{
}

JsonWrappedObjectModification::JsonWrappedObjectModification(const std::map<Identifier, std::vector<KindAttributeDataType> > *dataTypesOfEverything_):
    dataTypesOfEverything(dataTypesOfEverything_)
{
}

JsonWrappedAttribute JsonWrappedObjectModification::wrappedAttribute(const Identifier &kindName, const Identifier &attributeName) const
{
    JsonContext c1("When looking up kind " + kindName);
    std::map<Identifier, std::vector<KindAttributeDataType> >::const_iterator it1 = dataTypesOfEverything->find(kindName);
    if (it1 == dataTypesOfEverything->end())
        throw JsonStructureError("Kind " + kindName + " not recognized");
    const std::vector<KindAttributeDataType> &attributes = it1->second;

    JsonContext c2("When looking up attribute " + attributeName);
    using namespace boost::phoenix;
    std::vector<KindAttributeDataType>::const_iterator it2 = std::find_if(attributes.begin(), attributes.end(),
                                                                          bind(&KindAttributeDataType::name, arg_names::arg1) == attributeName);
    if (it2 == attributes.end())
        throw JsonStructureError("Attribute " + attributeName + " not recognized");
    return JsonWrappedAttribute(it2->type, attributeName);
}

JsonWrappedObjectModificationResultSequence::JsonWrappedObjectModificationResultSequence(
    const std::map<Identifier, std::vector<KindAttributeDataType> > *dataTypesOfEverything_):
    dataTypesOfEverything(dataTypesOfEverything_)
{
}

// template instances for the linker
template JsonField &JsonHandlerApiWrapper::argument(const std::string &, const std::string &);
template JsonField &JsonHandlerApiWrapper::argument(const std::string &, const Value &);
template JsonField &JsonHandlerApiWrapper::argument(const std::string &, const RevisionId &);
template JsonField &JsonHandlerApiWrapper::argument(const std::string &, const TemporaryChangesetId &);
template JsonField &JsonHandlerApiWrapper::argument(const std::string &, const Filter &);
template JsonField &JsonHandlerApiWrapper::argument(const std::string &, const std::vector<ObjectModificationCommand> &);
template JsonField &JsonHandlerApiWrapper::argument(const std::string &, const bool &);
template JsonField &JsonHandlerApiWrapper::argument(const std::string &, const boost::posix_time::ptime &);

}
}
