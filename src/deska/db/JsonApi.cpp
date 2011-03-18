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
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/home/phoenix/bind/bind_member_variable.hpp>
#include "JsonApi.h"

using namespace std;
using json_spirit::Object;
using json_spirit::Pair;


static std::string j_command = "command";
static std::string j_response = "response";
static std::string j_kindName = "kindName";
static std::string j_objName = "objectName";
static std::string j_newObjectName = "newObjectName";
static std::string j_attrName = "attributeName";
static std::string j_attrData = "attributeData";
static std::string j_revision = "revision";
static std::string j_currentRevision = "currentRevision";
static std::string j_errorPrefix = "error";

static std::string j_cmd_kindNames = "getTopLevelObjectNames";
static std::string j_cmd_kindAttributes = "getKindAttributes";
static std::string j_cmd_kindRelations = "getKindRelations";
static std::string j_cmd_kindInstances = "getKindInstances";
static std::string j_cmd_objectData = "getObjectData";
static std::string j_cmd_resolvedObjectData = "getResolvedObjectData";
static std::string j_cmd_findObjectsOverridingAttrs = "getObjectsOverridingAttribute";
static std::string j_cmd_findObjectsNotOverridingAttrs = "getObjectsNotOverridingAttribute";
static std::string j_cmd_createObject = "createObject";
static std::string j_cmd_deleteObject = "deleteObject";
static std::string j_cmd_renameObject = "renameObject";
static std::string j_cmd_removeAttribute = "removeObjectAttribute";
static std::string j_cmd_setAttribute = "setObjectAttribute";
static std::string j_cmd_startChangeset = "vcsStartChangeset";
static std::string j_cmd_commitChangeset = "vcsCommitChangeset";
static std::string j_cmd_rebaseChangeset = "vcsRebaseChangeset";
static std::string j_cmd_pendingChangesetsByMyself = "vcsGetPendingChangesetsByMyself";
static std::string j_cmd_resumeChangeset = "vcsResumePendingChangeset";
static std::string j_cmd_detachFromActiveChangeset = "vcsDetachFromActiveChangeset";
static std::string j_cmd_abortChangeset = "vcsAbortChangeset";

namespace Deska
{

/** @short Variant visitor convert a Deska::Value to json_spirit::Value */
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
        throw JsonParseError("Unsupported type of attribute data");
    }
}

JsonApiParser::JsonApiParser()
{
}

JsonApiParser::~JsonApiParser()
{
}

void JsonApiParser::sendJsonObject(const json_spirit::Object &o) const
{
    writeString(json_spirit::write(o, json_spirit::remove_trailing_zeros));
}

json_spirit::Object JsonApiParser::readJsonObject() const
{
    json_spirit::Value res;
    json_spirit::read(readString(), res);
    const json_spirit::Object &o = res.get_obj();
    // FIXME: check for the j_errorPrefix here
    return o;
}


#define JSON_REQUIRE_CMD \
    if (!gotCmdId) \
        throw JsonParseError("Response doesn't contain command identification");

#define JSON_REQUIRE_DATA \
    if (!gotData) \
        throw JsonParseError("Response doesn't contain usable data");

#define JSON_REQUIRE_CMD_DATA_KINDNAME \
    JSON_REQUIRE_CMD \
    JSON_REQUIRE_DATA \
    if (!gotKindName) \
        throw JsonParseError("Response doesn't contain kind identification");

#define JSON_REQUIRE_REVISION \
    if (!gotRevision) \
        throw JsonParseError("Response doesn't contain revision");

#define JSON_REQUIRE_CMD_DATA_KINDNAME_REVISION \
    JSON_REQUIRE_CMD_DATA_KINDNAME; \
    JSON_REQUIRE_REVISION

#define JSON_REQUIRE_OBJNAME \
    if (!gotObjectName) \
        throw JsonParseError("Response doesn't contain object identification");

#define JSON_REQUIRE_ATTRNAME \
    if (!gotAttrName) \
        throw JsonParseError("Response doesn't contain attribute name");

#define JSON_BLOCK_EXTRACT_REVISION \
    else if (node.name_ == j_revision) { \
        revision = node.value_.get_int64(); \
        gotRevision = true; \
    }

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

#define JSON_BLOCK_CHECK_REVISION(REV_IDENTIFICATION_NAME) \
    else if (node.name_ == REV_IDENTIFICATION_NAME) { \
        if (node.value_.get_int64() != rev) { \
            throw JsonParseError("Got unmatching revision"); \
        } \
        gotRevision = true; \
    }

#define JSON_BLOCK_CHECK_ATTRNAME \
    else if (node.name_ == j_attrName) { \
        if (node.value_.get_str() != attributeName) { \
            throw JsonParseError("Got unmatching attribute name"); \
        } \
        gotAttrName = true; \
    }

#define JSON_BLOCK_CHECK_BOOL_RESULT(CMD, RESULT_VARIABLE) \
    else if (node.name_ == "result") { \
        if (!node.value_.get_bool()) { \
            /* Yes, we really do require true here. The idea is that failed operations are reported using another, \
               different mechanism, likely via an exception. */ \
            std::ostringstream s; \
            s << "Mallformed " << CMD << " reply: got something else than true as a 'result'."; \
            throw JsonParseError(s.str()); \
        } \
        RESULT_VARIABLE = true; \
    }

#define JSON_BLOCK_CHECK_ELSE \
    else { \
        throw JsonParseError("Response contains aditional data"); \
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
        JSON_BLOCK_CHECK_COMMAND(j_cmd_kindNames)
        else if (node.name_ == "topLevelObjectKinds") {
            json_spirit::Array data = node.value_.get_array();
            // simply copy a string from the JSON representation into a vector<string>
            std::transform(data.begin(), data.end(), std::back_inserter(res), std::mem_fun_ref(&json_spirit::Value::get_str));
            gotData = true;
        }
        JSON_BLOCK_CHECK_ELSE
    }
    JSON_REQUIRE_CMD;
    JSON_REQUIRE_DATA;
    return res;
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
        JSON_BLOCK_CHECK_REVISION(j_revision)
        JSON_BLOCK_CHECK_ELSE
    }

    JSON_REQUIRE_CMD_DATA_KINDNAME_REVISION;

    return res;
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
                // FIXME: check type information for the attributes, and even attribute existence. This will require already cached kindAttributes()...
                res[item.name_] = jsonValueToDeskaValue(item.value_);
            }
            gotData = true;
        }
        JSON_BLOCK_CHECK_KINDNAME
        JSON_BLOCK_CHECK_OBJNAME
        JSON_BLOCK_CHECK_REVISION(j_revision)
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
                // FIXME: check type information for the attributes, and even attribute existence. This will require already cached kindAttributes()...
                res[item.name_] = std::make_pair(a[0].get_str(), jsonValueToDeskaValue(a[1]));
            }
            gotData = true;
        }
        JSON_BLOCK_CHECK_KINDNAME
        JSON_BLOCK_CHECK_OBJNAME
        JSON_BLOCK_CHECK_REVISION(j_revision)
        JSON_BLOCK_CHECK_ELSE
    }

    JSON_REQUIRE_CMD_DATA_KINDNAME_REVISION;
    JSON_REQUIRE_OBJNAME;

    return res;
}

vector<Identifier> JsonApiParser::helperOverridenAttrs(const std::string &cmd, const Identifier &kindName, const Identifier &objectName, const Identifier &attributeName)
{
    Object o;
    o.push_back(Pair(j_command, cmd));
    o.push_back(Pair(j_kindName, kindName));
    o.push_back(Pair(j_objName, objectName));
    o.push_back(Pair(j_attrName, attributeName));
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
                                                const Identifier &attributeName)
{
    return helperOverridenAttrs(j_cmd_findObjectsOverridingAttrs, kindName, objectName, attributeName);
}

vector<Identifier> JsonApiParser::findNonOverriddenAttrs(const Identifier &kindName, const Identifier &objectName,
                                                   const Identifier &attributeName)
{
    return helperOverridenAttrs(j_cmd_findObjectsNotOverridingAttrs, kindName, objectName, attributeName);
}


void JsonApiParser::helperCreateDeleteObject(const std::string &cmd, const Identifier &kindName, const Identifier &objectName)
{
    Object o;
    o.push_back(Pair(j_command, cmd));
    o.push_back(Pair(j_kindName, kindName));
    o.push_back(Pair(j_objName, objectName));
    sendJsonObject(o);

    bool gotCmdId = false;
    bool gotData = false;
    bool gotKindName = false;
    bool gotObjectName = false;

    BOOST_FOREACH(const Pair& node, readJsonObject()) {
        JSON_BLOCK_CHECK_COMMAND(cmd)
        JSON_BLOCK_CHECK_BOOL_RESULT(cmd, gotData)
        JSON_BLOCK_CHECK_KINDNAME
        JSON_BLOCK_CHECK_OBJNAME
        JSON_BLOCK_CHECK_ELSE
    }

    JSON_REQUIRE_CMD_DATA_KINDNAME;
    JSON_REQUIRE_OBJNAME;
}

void JsonApiParser::deleteObject( const Identifier &kindName, const Identifier &objectName )
{
    helperCreateDeleteObject(j_cmd_deleteObject, kindName, objectName);
}

void JsonApiParser::createObject( const Identifier &kindName, const Identifier &objectName )
{
    helperCreateDeleteObject(j_cmd_createObject, kindName, objectName);
}

void JsonApiParser::renameObject( const Identifier &kindName, const Identifier &oldName, const Identifier &newName )
{
    Object o;
    o.push_back(Pair(j_command, j_cmd_renameObject));
    o.push_back(Pair(j_kindName, kindName));
    o.push_back(Pair(j_objName, oldName));
    o.push_back(Pair(j_newObjectName, newName));
    sendJsonObject(o);

    bool gotCmdId = false;
    bool gotData = false;
    bool gotKindName = false;
    bool gotObjectName = false;
    bool gotNewObjectName = false;

    // Setup an alias for the JSON_BLOCK_CHECK_OBJNAME macro. It is ugly, but I feel like having "oldName" as the argument
    // name is beneficial.
    const Identifier &objectName = oldName;

    BOOST_FOREACH(const Pair& node, readJsonObject()) {
        JSON_BLOCK_CHECK_COMMAND(j_cmd_renameObject)
        JSON_BLOCK_CHECK_BOOL_RESULT(j_cmd_renameObject, gotData)
        JSON_BLOCK_CHECK_KINDNAME
        JSON_BLOCK_CHECK_OBJNAME
        else if (node.name_ == j_newObjectName) {
            if (node.value_.get_str() != newName) { \
                throw JsonParseError("newObjectName doesn't match"); \
            } \
            gotNewObjectName = true;
        }
        JSON_BLOCK_CHECK_ELSE
    }

    JSON_REQUIRE_CMD_DATA_KINDNAME;
    JSON_REQUIRE_OBJNAME;
    if (!gotNewObjectName) \
        throw JsonParseError("Response doesn't contain newObjectName identification");
}

void JsonApiParser::removeAttribute(const Identifier &kindName, const Identifier &objectName, const Identifier &attributeName)
{
    Object o;
    o.push_back(Pair(j_command, j_cmd_removeAttribute));
    o.push_back(Pair(j_kindName, kindName));
    o.push_back(Pair(j_objName, objectName));
    o.push_back(Pair(j_attrName, attributeName));
    sendJsonObject(o);

    bool gotCmdId = false;
    bool gotData = false;
    bool gotKindName = false;
    bool gotObjectName = false;
    bool gotAttrName = false;

    BOOST_FOREACH(const Pair& node, readJsonObject()) {
        JSON_BLOCK_CHECK_COMMAND(j_cmd_removeAttribute)
        JSON_BLOCK_CHECK_BOOL_RESULT(j_cmd_removeAttribute, gotData)
        JSON_BLOCK_CHECK_KINDNAME
        JSON_BLOCK_CHECK_OBJNAME
        JSON_BLOCK_CHECK_ATTRNAME
        JSON_BLOCK_CHECK_ELSE
    }

    JSON_REQUIRE_CMD_DATA_KINDNAME;
    JSON_REQUIRE_OBJNAME;
    JSON_REQUIRE_ATTRNAME;
}

void JsonApiParser::setAttribute(const Identifier &kindName, const Identifier &objectName, const Identifier &attributeName,
                           const Value &value)
{
    Object o;
    o.push_back(Pair(j_command, j_cmd_setAttribute));
    o.push_back(Pair(j_kindName, kindName));
    o.push_back(Pair(j_objName, objectName));
    o.push_back(Pair(j_attrName, attributeName));
    json_spirit::Value jsonAttrValue = boost::apply_visitor(DeskaValueToJsonValue(), value);
    o.push_back(Pair(j_attrData, jsonAttrValue));
    sendJsonObject(o);

    bool gotCmdId = false;
    bool gotData = false;
    bool gotKindName = false;
    bool gotObjectName = false;
    bool gotAttrName = false;
    bool gotAttrData = false;

    BOOST_FOREACH(const Pair& node, readJsonObject()) {
        JSON_BLOCK_CHECK_COMMAND(j_cmd_setAttribute)
        JSON_BLOCK_CHECK_BOOL_RESULT(j_cmd_setAttribute, gotData)
        JSON_BLOCK_CHECK_KINDNAME
        JSON_BLOCK_CHECK_OBJNAME
        JSON_BLOCK_CHECK_ATTRNAME
        else if (node.name_ == j_attrData) {
            // Oh yeah, json_spirit::Value doesn't implement operator!=. Well, at least it has operator== :).
            if (!(node.value_ == jsonAttrValue)) {
                throw JsonParseError("Returned value of attributeData is different than what we requested");
            }
            gotAttrData = true;
        }
        JSON_BLOCK_CHECK_ELSE
    }

    JSON_REQUIRE_CMD_DATA_KINDNAME;
    JSON_REQUIRE_OBJNAME;
    JSON_REQUIRE_ATTRNAME;
    if (!gotAttrData)
        throw JsonParseError("Response did not specify attributeData");
}


struct Field
{
    bool isForSending;
    bool isRequiredToReceive;
    bool isAlreadyReceived;
    bool valueShouldMatch;
    std::string jsonFieldRead, jsonFieldWrite;
    json_spirit::Value jsonValue;
    Revision *e_Revision;

    Field(const std::string &name):
        isForSending(false), isRequiredToReceive(true), isAlreadyReceived(false), valueShouldMatch(false),
        jsonFieldRead(name), jsonFieldWrite(name), e_Revision(0)
    {
    }

    Field &optional()
    {
        isRequiredToReceive = false;
        return *this;
    }

    Field &extractRevision(Revision *where)
    {
        e_Revision = where;
        return *this;
    }
};

class JsonHandler
{
public:
    JsonHandler(JsonApiParser *api): p(api)
    {
    }

    void send()
    {
        json_spirit::Object o;
        BOOST_FOREACH(const Field &f, fields) {
            if (f.isForSending) {
                o.push_back(json_spirit::Pair(f.jsonFieldWrite, f.jsonValue));
            }
        }
        p->sendJsonObject(o);
    }

    void receive()
    {
        using namespace boost::phoenix;
        using namespace arg_names;

        BOOST_FOREACH(const Pair& node, p->readJsonObject()) {

            // At first, find a matching rule for this particular key
            std::vector<Field>::iterator rule =
                    std::find_if(fields.begin(), fields.end(), bind(&Field::jsonFieldRead, arg1) == node.name_);

            if (rule == fields.end()) {
                // No such rule
                std::ostringstream s;
                s << "Unhandled JSON field '" << node.name_ << "'";
                throw JsonParseError(s.str());
            }

            if (rule->isAlreadyReceived) {
                // Duplicate rule
                std::ostringstream s;
                s << "Duplicate JSON field '" << node.name_ << "'";
                throw JsonParseError(s.str());
            }

            // Check the value
            if (rule->valueShouldMatch) {
                // Oh yeah, json_spirit::Value doesn't implement operator!=. Well, at least it has operator== :).
                if (!(node.value_ == rule->jsonValue)) {
                    std::ostringstream s;
                    s << "JSON value mismatch for field '" << rule->jsonFieldRead << "'";
                    throw JsonParseError(s.str());
                }
            }

            // Store revision
            if (rule->e_Revision) {
                *(rule->e_Revision) = node.value_.get_int64();
            }

            // Mark this field as "processed"
            rule->isAlreadyReceived = true;
        }

        std::vector<Field>::iterator rule =
                std::find_if(fields.begin(), fields.end(),
                ! bind(&Field::isAlreadyReceived, arg1) && bind(&Field::isRequiredToReceive, arg1) );
        if ( rule != fields.end() ) {
            std::ostringstream s;
            s << "Mandatory field '" << rule->jsonFieldRead << "' not present in the response";
            throw JsonParseError(s.str());
        }
    }

    void work()
    {
        send();
        receive();
    }

    Field &command(const std::string &cmd)
    {
        Field f(j_command);
        f.jsonFieldRead = j_response;
        f.jsonValue = cmd;
        f.isForSending = true;
        f.valueShouldMatch = true;
        fields.push_back(f);
        return *(--fields.end());
    }

    Field &write(const std::string &name, const std::string &value)
    {
        Field f(name);
        f.jsonValue = value;
        f.isForSending = true;
        f.valueShouldMatch = true;
        fields.push_back(f);
        return *(--fields.end());
    }

    Field &write(const std::string &name, const Revision value)
    {
        Field f(name);
        f.jsonValue = static_cast<int64_t>(value);
        f.isForSending = true;
        f.valueShouldMatch = true;
        fields.push_back(f);
        return *(--fields.end());
    }

    Field &read(const std::string &name)
    {
        Field f(name);
        fields.push_back(f);
        return *(--fields.end());
    }

private:
    JsonApiParser *p;
    std::vector<Field> fields;
};

Revision JsonApiParser::helperStartCommitChangeset(const std::string &cmd)
{
    Revision revision = 0;
    JsonHandler h(this);
    h.command(cmd);
    h.read(j_revision).extractRevision(&revision);
    h.work();
    return revision;
}

Revision JsonApiParser::startChangeset()
{
    return helperStartCommitChangeset(j_cmd_startChangeset);
}

Revision JsonApiParser::commitChangeset()
{
    return helperStartCommitChangeset(j_cmd_commitChangeset);
}

Revision JsonApiParser::rebaseChangeset(const Revision oldRevision)
{
    Revision revision = 0;
    JsonHandler h(this);
    h.command(j_cmd_rebaseChangeset);
    h.write(j_currentRevision, oldRevision);
    h.read(j_revision).extractRevision(&revision);
    h.work();
    return revision;
}

vector<Revision> JsonApiParser::pendingChangesetsByMyself()
{
    Object o;
    o.push_back(Pair(j_command, j_cmd_pendingChangesetsByMyself));
    sendJsonObject(o);

    bool gotCmdId = false;
    bool gotData = false;

    vector<Revision> res;

    BOOST_FOREACH(const Pair& node, readJsonObject()) {
        JSON_BLOCK_CHECK_COMMAND(j_cmd_pendingChangesetsByMyself)
        else if (node.name_ == "revisions") {
            json_spirit::Array data = node.value_.get_array();
            // Copy int64 and store them into a vector<Revision>
            std::transform(data.begin(), data.end(), std::back_inserter(res), std::mem_fun_ref(&json_spirit::Value::get_int64));
            gotData = true;
        }
        JSON_BLOCK_CHECK_ELSE
    }

    JSON_REQUIRE_CMD;
    JSON_REQUIRE_DATA;
    return res;
}

void JsonApiParser::resumeChangeset(const Revision revision)
{
    JsonHandler h(this);
    h.command(j_cmd_resumeChangeset);
    h.write(j_revision, revision);
    h.work();
}

void JsonApiParser::detachFromActiveChangeset()
{
    Object o;
    o.push_back(Pair(j_command, j_cmd_detachFromActiveChangeset));
    sendJsonObject(o);

    bool gotCmdId = false;

    BOOST_FOREACH(const Pair& node, readJsonObject()) {
        JSON_BLOCK_CHECK_COMMAND(j_cmd_detachFromActiveChangeset)
        JSON_BLOCK_CHECK_ELSE
    }

    JSON_REQUIRE_CMD;
}

void JsonApiParser::abortChangeset(const Revision rev)
{
    Object o;
    o.push_back(Pair(j_command, j_cmd_abortChangeset));
    // The following cast is required because the json_spirit doesn't have an overload for uint...
    o.push_back(Pair(j_revision, static_cast<int64_t>(rev)));
    sendJsonObject(o);

    bool gotCmdId = false;
    bool gotRevision = false;

    BOOST_FOREACH(const Pair& node, readJsonObject()) {
        JSON_BLOCK_CHECK_COMMAND(j_cmd_abortChangeset)
        JSON_BLOCK_CHECK_REVISION(j_revision)
        JSON_BLOCK_CHECK_ELSE
    }

    JSON_REQUIRE_CMD;
    JSON_REQUIRE_REVISION;
}

JsonParseError::JsonParseError(const std::string &message): std::runtime_error(message)
{
}

JsonParseError::~JsonParseError() throw ()
{
}

}
