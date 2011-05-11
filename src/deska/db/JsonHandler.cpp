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
#include "JsonExtraction.h"
#include "JsonHandler.h"
#include "JsonApi.h"

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

JsonWrappedAttributeMapWithOrigin::JsonWrappedAttributeMapWithOrigin(const std::vector<KindAttributeDataType> dataTypes_):
    dataTypes(dataTypes_)
{
}

JsonWrappedAttribute::JsonWrappedAttribute(const Type dataType_, const Identifier &attrName_):
    dataType(dataType_), attrName(attrName_)
{
}

JsonWrappedAttributeWithOrigin::JsonWrappedAttributeWithOrigin(const Type dataType_, const Identifier &attrName_):
    JsonWrappedAttribute(dataType_, attrName_)
{
}


}
}
