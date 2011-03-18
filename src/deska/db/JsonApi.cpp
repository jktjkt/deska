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
#include "JsonApi.h"

using namespace std;
using json_spirit::Object;
using json_spirit::Pair;


static std::string j_command = "command";
static std::string j_response = "response";
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

namespace Deska
{

JsonApiParser::JsonApiParser()
{
}

JsonApiParser::~JsonApiParser()
{
}

void JsonApiParser::sendJsonObject(const json_spirit::Object &o) const
{
    writeString(json_spirit::write(o));
}

json_spirit::Object JsonApiParser::readJsonObject() const
{
    json_spirit::Value res;
    json_spirit::read(readString(), res);
    const json_spirit::Object &o = res.get_obj();
    // FIXME: check for the j_errorPrefix here
    return o;
}

vector<Identifier> JsonApiParser::kindNames() const
{
    // Send the command
    Object o;
    o.push_back(Pair(j_command, j_cmd_kindNames));
    sendJsonObject(o);

    // Retrieve and process the response
    bool gotCmdId = false;
    bool gotData = false;
    vector<Identifier> res;

    BOOST_FOREACH(const Pair& node, readJsonObject()) {
        if (node.name_ == j_response) {
            if (node.value_.get_str() != j_cmd_kindNames)
                throw JsonParseError("Response belongs to another command");
            gotCmdId = true;
        } else if (node.name_ == "topLevelObjectKinds") {
            json_spirit::Array data = node.value_.get_array();
            // simply copy a string from the JSON representation into a vector<string>
            std::transform(data.begin(), data.end(), std::back_inserter(res), std::mem_fun_ref(&json_spirit::Value::get_str));
            gotData = true;
        } else {
            throw JsonParseError("Response contains aditional data");
        }
    }
    if (!gotCmdId)
        throw JsonParseError("Response doesn't contain command identification");
    if (!gotData)
        throw JsonParseError("Response doesn't contain usable data");

    return res;
}

#define JSON_REQUIRE_CMD_DATA_KINDNAME \
    if (!gotCmdId) \
        throw JsonParseError("Response doesn't contain command identification"); \
    if (!gotData) \
        throw JsonParseError("Response doesn't contain usable data"); \
    if (!gotKindName) \
        throw JsonParseError("Response doesn't contain kind identification");

#define JSON_REQUIRE_CMD_DATA_KINDNAME_REVISION \
    JSON_REQUIRE_CMD_DATA_KINDNAME; \
    if (!gotRevision) \
        throw JsonParseError("Response doesn't contain revision");

#define JSON_REQUIRE_OBJNAME \
    if (!gotObjectName) \
        throw JsonParseError("Response doesn't contain object identification");

#define JSON_REQUIRE_ATTRNAME \
    if (!gotAttrName) \
        throw JsonParseError("Response doesn't contain attribute name");

#define JSON_BLOCK_CHECK_COMMAND(X) \
    if (node.name_ == j_response) { \
        if (node.value_.get_str() != X) \
            throw JsonParseError("Response belongs to another command"); \
        gotCmdId = true; \
    }

#define JSON_BLOCK_CHECK_KINDNAME \
    else if (node.name_ == j_kindName) { \
        if (node.value_.get_str() != kindName) { \
            throw JsonParseError("Response addressed to a different kindAttributes request"); \
        } \
        gotKindName = true; \
    }

#define JSON_BLOCK_CHECK_OBJNAME \
    else if (node.name_ == j_objName) { \
        if (node.value_.get_str() != objectName) { \
            throw JsonParseError("Response concerning another object name"); \
        } \
        gotObjectName = true; \
    }

#define JSON_BLOCK_CHECK_REVISION \
    else if (node.name_ == j_revision) { \
        if (node.value_.get_int64() != rev) { \
            throw JsonParseError("Got unmatching revision"); \
        } \
        gotRevision = true; \
    }

#define JSON_BLOCK_CHECK_ATTRNAME \
    else if (node.name_ == j_attrName) { \
        if (node.value_.get_str() != attrName) { \
            throw JsonParseError("Got unmatching attribute name"); \
        } \
        gotAttrName = true; \
    }

#define JSON_BLOCK_CHECK_ELSE \
    else { \
        throw JsonParseError("Response contains aditional data"); \
    }

vector<KindAttributeDataType> JsonApiParser::kindAttributes( const Identifier &kindName ) const
{
    Object o;
    o.push_back(Pair(j_command, j_cmd_kindAttributes));
    o.push_back(Pair(j_kindName, kindName));
    sendJsonObject(o);

    bool gotCmdId = false;
    bool gotKindName = false;
    bool gotData = false;
    vector<KindAttributeDataType> res;

    BOOST_FOREACH(const Pair &node, readJsonObject()) {
        JSON_BLOCK_CHECK_COMMAND(j_cmd_kindAttributes)
        else if (node.name_ == "kindAttributes") {
            BOOST_FOREACH(const Pair &item, node.value_.get_obj()) {
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
                    throw JsonParseError(s.str());
                }
            }
            gotData = true;
        }
        JSON_BLOCK_CHECK_KINDNAME
        JSON_BLOCK_CHECK_ELSE
    }

    JSON_REQUIRE_CMD_DATA_KINDNAME

    return res;
}

vector<ObjectRelation> JsonApiParser::kindRelations( const Identifier &kindName ) const
{
    Object o;
    o.push_back(Pair(j_command, j_cmd_kindRelations));
    o.push_back(Pair(j_kindName, kindName));
    sendJsonObject(o);

    bool gotCmdId = false;
    bool gotKindName = false;
    bool gotData = false;
    vector<ObjectRelation> res;

    BOOST_FOREACH(const Pair &node, readJsonObject()) {
        JSON_BLOCK_CHECK_COMMAND(j_cmd_kindRelations)
        else if (node.name_ == "kindRelations") {
            BOOST_FOREACH(const json_spirit::Value &item, node.value_.get_array()) {
                json_spirit::Array relationRecord = item.get_array();
                switch (relationRecord.size()) {
                // got to enclose the individual branches in curly braces to be able to use local variables...
                case 2:
                {
                    // EMBED_INTO, IS_TEMPLATE
                    std::string kind = relationRecord[0].get_str();
                    if (kind == "EMBED_INTO") {
                        res.push_back(ObjectRelation::embedInto(relationRecord[1].get_str()));
                    } else if (kind == "IS_TEMPLATE") {
                        res.push_back(ObjectRelation::isTemplate(relationRecord[1].get_str()));
                    } else {
                        std::ostringstream s;
                        s << "Invalid relation kind " << kind << " with one argument";
                        throw JsonParseError(s.str());
                    }
                }
                break;
                case 3:
                {
                    // MERGE_WITH, TEMPLATIZED
                    std::string kind = relationRecord[0].get_str();
                    if (kind == "MERGE_WITH") {
                        res.push_back(ObjectRelation::mergeWith(relationRecord[1].get_str(), relationRecord[2].get_str()));
                    } else if (kind == "TEMPLATIZED") {
                        res.push_back(ObjectRelation::templatized(relationRecord[1].get_str(), relationRecord[2].get_str()));
                    } else {
                        std::ostringstream s;
                        s << "Invalid relation kind " << kind << " with two arguments";
                        throw JsonParseError(s.str());
                    }
                }
                break;
                default:
                    throw JsonParseError("Relation record has invalid number of arguments");
                }
            }
            gotData = true;
        }
        JSON_BLOCK_CHECK_KINDNAME
        JSON_BLOCK_CHECK_ELSE
    }

    JSON_REQUIRE_CMD_DATA_KINDNAME;

    return res;
}

vector<Identifier> JsonApiParser::kindInstances( const Identifier &kindName, const Revision rev ) const
{
    Object o;
    o.push_back(Pair(j_command, j_cmd_kindInstances));
    o.push_back(Pair(j_kindName, kindName));
    // The following cast is required because the json_spirit doesn't have an overload for uint...
    o.push_back(Pair(j_revision, static_cast<int64_t>(rev)));
    sendJsonObject(o);

    // Retrieve and process the response
    bool gotCmdId = false;
    bool gotData = false;
    bool gotKindName = false;
    bool gotRevision = false;
    vector<Identifier> res;

    BOOST_FOREACH(const Pair& node, readJsonObject()) {
        JSON_BLOCK_CHECK_COMMAND(j_cmd_kindInstances)
        else if (node.name_ == "objectInstances") {
            json_spirit::Array data = node.value_.get_array();
            // simply copy a string from the JSON representation into a vector<string>
            std::transform(data.begin(), data.end(), std::back_inserter(res), std::mem_fun_ref(&json_spirit::Value::get_str));
            gotData = true;
        }
        JSON_BLOCK_CHECK_KINDNAME
        JSON_BLOCK_CHECK_REVISION
        JSON_BLOCK_CHECK_ELSE
    }

    JSON_REQUIRE_CMD_DATA_KINDNAME_REVISION;

    return res;
}

Value JsonApiParser::jsonValueToDeskaValue(const json_spirit::Value &v)
{
    // FIXME: check type information for the attributes, and even attribute existence. This will require already cached kindAttributes()...
    if (v.type() == json_spirit::str_type) {
        return v.get_str();
    } else if (v.type() == json_spirit::int_type) {
        return v.get_int();
    } else if (v.type() == json_spirit::real_type) {
        return v.get_real();
    } else {
        throw JsonParseError("Unsupported type of attribute data");
    }
}

map<Identifier, Value> JsonApiParser::objectData( const Identifier &kindName, const Identifier &objectName, const Revision rev )
{
    Object o;
    o.push_back(Pair(j_command, j_cmd_objectData));
    o.push_back(Pair(j_kindName, kindName));
    o.push_back(Pair(j_objName, objectName));
    // The following cast is required because the json_spirit doesn't have an overload for uint...
    o.push_back(Pair(j_revision, static_cast<int64_t>(rev)));
    sendJsonObject(o);

    bool gotCmdId = false;
    bool gotData = false;
    bool gotKindName = false;
    bool gotObjectName = false;
    bool gotRevision = false;
    map<Identifier, Value> res;

    BOOST_FOREACH(const Pair& node, readJsonObject()) {
        JSON_BLOCK_CHECK_COMMAND(j_cmd_objectData)
        else if (node.name_ == "objectData") {
            BOOST_FOREACH(const Pair &item, node.value_.get_obj()) {
                res[item.name_] = jsonValueToDeskaValue(item.value_);
            }
            gotData = true;
        }
        JSON_BLOCK_CHECK_KINDNAME
        JSON_BLOCK_CHECK_OBJNAME
        JSON_BLOCK_CHECK_REVISION
        JSON_BLOCK_CHECK_ELSE
    }

    JSON_REQUIRE_CMD_DATA_KINDNAME_REVISION;
    JSON_REQUIRE_OBJNAME;

    return res;
}

map<Identifier, pair<Identifier, Value> > JsonApiParser::resolvedObjectData(const Identifier &kindName,
                                                                      const Identifier &objectName, const Revision rev )
{
    Object o;
    o.push_back(Pair(j_command, j_cmd_resolvedObjectData));
    o.push_back(Pair(j_kindName, kindName));
    o.push_back(Pair(j_objName, objectName));
    // The following cast is required because the json_spirit doesn't have an overload for uint...
    o.push_back(Pair(j_revision, static_cast<int64_t>(rev)));
    sendJsonObject(o);

    bool gotCmdId = false;
    bool gotData = false;
    bool gotKindName = false;
    bool gotObjectName = false;
    bool gotRevision = false;
    map<Identifier, pair<Identifier, Value> > res;

    BOOST_FOREACH(const Pair& node, readJsonObject()) {
        JSON_BLOCK_CHECK_COMMAND(j_cmd_resolvedObjectData)
        else if (node.name_ == "resolvedObjectData") {
            BOOST_FOREACH(const Pair &item, node.value_.get_obj()) {
                json_spirit::Array a = item.value_.get_array();
                if (a.size() != 2) {
                    throw JsonParseError("Malformed record of resolved attribute");
                }
                res[item.name_] = std::make_pair(a[0].get_str(), jsonValueToDeskaValue(a[1]));
            }
            gotData = true;
        }
        JSON_BLOCK_CHECK_KINDNAME
        JSON_BLOCK_CHECK_OBJNAME
        JSON_BLOCK_CHECK_REVISION
        JSON_BLOCK_CHECK_ELSE
    }

    JSON_REQUIRE_CMD_DATA_KINDNAME_REVISION;
    JSON_REQUIRE_OBJNAME;

    return res;
}

vector<Identifier> JsonApiParser::helperOverridenAttrs(const std::string &cmd, const Identifier &kindName, const Identifier &objectName, const Identifier &attrName)
{
    Object o;
    o.push_back(Pair(j_command, cmd));
    o.push_back(Pair(j_kindName, kindName));
    o.push_back(Pair(j_objName, objectName));
    o.push_back(Pair(j_attrName, attrName));
    sendJsonObject(o);

    bool gotCmdId = false;
    bool gotData = false;
    bool gotKindName = false;
    bool gotObjectName = false;
    bool gotAttrName = false;
    vector<Identifier> res;

    BOOST_FOREACH(const Pair& node, readJsonObject()) {
        JSON_BLOCK_CHECK_COMMAND(cmd)
        else if (node.name_ == "objectInstances") {
            json_spirit::Array data = node.value_.get_array();
            std::transform(data.begin(), data.end(), std::back_inserter(res), std::mem_fun_ref(&json_spirit::Value::get_str));
            gotData = true;
        }
        JSON_BLOCK_CHECK_KINDNAME
        JSON_BLOCK_CHECK_OBJNAME
        JSON_BLOCK_CHECK_ATTRNAME
        JSON_BLOCK_CHECK_ELSE
    }

    JSON_REQUIRE_CMD_DATA_KINDNAME;
    JSON_REQUIRE_OBJNAME;
    JSON_REQUIRE_ATTRNAME;

    return res;
}

vector<Identifier> JsonApiParser::findOverriddenAttrs(const Identifier &kindName, const Identifier &objectName,
                                                const Identifier &attrName)
{
    return helperOverridenAttrs(j_cmd_findObjectsOverridingAttrs, kindName, objectName, attrName);
}

vector<Identifier> JsonApiParser::findNonOverriddenAttrs(const Identifier &kindName, const Identifier &objectName,
                                                   const Identifier &attrName)
{
    return helperOverridenAttrs(j_cmd_findObjectsNotOverridingAttrs, kindName, objectName, attrName);
}

void JsonApiParser::deleteObject( const Identifier &kindName, const Identifier &objectName )
{
    throw 42;
}

void JsonApiParser::createObject( const Identifier &kindName, const Identifier &objectname )
{
    throw 42;
}

void JsonApiParser::renameObject( const Identifier &kindName, const Identifier &oldName, const Identifier &newName )
{
    throw 42;
}

void JsonApiParser::removeAttribute(const Identifier &kindName, const Identifier &objectName, const Identifier &attributeName)
{
    throw 42;
}

void JsonApiParser::setAttribute(const Identifier &kindName, const Identifier &objectName, const Identifier &attributeName,
                           const Value &value)
{
    throw 42;
}

Revision JsonApiParser::startChangeset()
{
    throw 42;
}

Revision JsonApiParser::commitChangeset()
{
    throw 42;
}

Revision JsonApiParser::rebaseChangeset(const Revision oldRevision)
{
    throw 42;
}

std::vector<Revision> JsonApiParser::pendingChangesetsByMyself()
{
    throw 42;
}

Revision JsonApiParser::resumeChangeset(const Revision oldRevision)
{
    throw 42;
}

void JsonApiParser::detachFromActiveChangeset()
{
    throw 42;
}

void JsonApiParser::abortChangeset(const Revision rev)
{
     throw 42;
}

JsonParseError::JsonParseError(const std::string &message): std::runtime_error(message)
{
}

JsonParseError::~JsonParseError() throw ()
{
}

}
