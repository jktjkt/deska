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
// The Phoenix is rather prone to missing includes. The compilation is roughly 10% slower
// when including everything, but it's a worthwhile sacrifice, as it prevents many nasty
// errors which are rather hard to debug.
#include <boost/spirit/include/phoenix.hpp>
#include "JsonHandler.h"
#include "JsonApi.h"

using namespace std;
using json_spirit::Object;
using json_spirit::Pair;

static std::string j_command = "command";
static std::string j_response = "response";

namespace Deska {
namespace Db {

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



/** @short Convert JSON into Deska::RevisionId */
template<>
void SpecializedExtractor<RevisionId>::extract(const json_spirit::Value &value)
{
    *target = RevisionId::fromJson(value.get_str());
}

/** @short Convert JSON into Deska::TemporaryChangesetId */
template<>
void SpecializedExtractor<TemporaryChangesetId>::extract(const json_spirit::Value &value)
{
    *target = TemporaryChangesetId::fromJson(value.get_str());
}

/** @short Convert JSON into a vector of Deska::RevisionId */
template<>
void SpecializedExtractor<std::vector<TemporaryChangesetId> >::extract(const json_spirit::Value &value)
{
    using namespace boost::phoenix;
    using arg_names::_1;
    json_spirit::Array data = value.get_array();
    // Extract the int64_t, convert them into a TemporaryChangesetId and store them into a vector
    std::transform(data.begin(), data.end(), std::back_inserter(*target),
                   bind(&TemporaryChangesetId::fromJson, bind(&json_spirit::Value::get_str, _1))
                   );
}

/** @short Convert JSON into a vector of Deska::Identifier */
template<>
void SpecializedExtractor<std::vector<Identifier> >::extract(const json_spirit::Value &value)
{
    json_spirit::Array data = value.get_array();
    std::transform(data.begin(), data.end(), std::back_inserter(*target), std::mem_fun_ref(&json_spirit::Value::get_str));
}

/** @short Convert JSON into a vector of attribute data types */
template<>
void SpecializedExtractor<std::vector<KindAttributeDataType> >::extract(const json_spirit::Value &value)
{
    BOOST_FOREACH(const Pair &item, value.get_obj()) {
        std::string datatype = item.value_.get_str();
        if (datatype == "string") {
            target->push_back(KindAttributeDataType(item.name_, TYPE_STRING));
        } else if (datatype == "int") {
            target->push_back(KindAttributeDataType(item.name_, TYPE_INT));
        } else if (datatype == "identifier") {
            target->push_back(KindAttributeDataType(item.name_, TYPE_IDENTIFIER));
        } else if (datatype == "double") {
            target->push_back(KindAttributeDataType(item.name_, TYPE_DOUBLE));
        } else {
            std::ostringstream s;
            s << "Unsupported data type \"" << datatype << "\" for attribute \"" << item.name_ << "\"";
            throw JsonParseError(s.str());
        }
    }
}

/** @short Convert JSON into a vector of object relations */
template<>
void SpecializedExtractor<std::vector<ObjectRelation> >::extract(const json_spirit::Value &value)
{
    BOOST_FOREACH(const json_spirit::Value &item, value.get_array()) {
        json_spirit::Array relationRecord = item.get_array();
        switch (relationRecord.size()) {
        // got to enclose the individual branches in curly braces to be able to use local variables...
        case 2:
        {
            // EMBED_INTO, IS_TEMPLATE
            std::string kind = relationRecord[0].get_str();
            if (kind == "EMBED_INTO") {
                target->push_back(ObjectRelation::embedInto(relationRecord[1].get_str()));
            } else if (kind == "IS_TEMPLATE") {
                target->push_back(ObjectRelation::isTemplate(relationRecord[1].get_str()));
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
                target->push_back(ObjectRelation::mergeWith(relationRecord[1].get_str(), relationRecord[2].get_str()));
            } else if (kind == "TEMPLATIZED") {
                target->push_back(ObjectRelation::templatized(relationRecord[1].get_str(), relationRecord[2].get_str()));
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
}

/** @short Convert JSON into a special data structure representing all attributes of an object */
template<>
void SpecializedExtractor<std::map<Identifier,Value> >::extract(const json_spirit::Value &value)
{
    BOOST_FOREACH(const Pair &item, value.get_obj()) {
        // FIXME: check type information for the attributes, and even attribute existence. This will require already cached kindAttributes()...
        (*target)[item.name_] = jsonValueToDeskaValue(item.value_);
    }
}

/** @short Convert JSON into a special data structure representing all attributes of an object along with information where their values come from */
template<>
void SpecializedExtractor<std::map<Identifier,pair<Identifier,Value> > >::extract(const json_spirit::Value &value)
{
    BOOST_FOREACH(const Pair &item, value.get_obj()) {
        json_spirit::Array a = item.value_.get_array();
        if (a.size() != 2) {
            throw JsonParseError("Malformed record of resolved attribute");
        }
        // FIXME: check type information for the attributes, and even attribute existence. This will require already cached kindAttributes()...
        (*target)[item.name_] = std::make_pair(a[0].get_str(), jsonValueToDeskaValue(a[1]));
    }
}

/** @short Require specialization for all target types during compilation of this translation unit */
template<typename T>
void SpecializedExtractor<T>::extract(const json_spirit::Value &value)
{
    // If you get this error, there's no extractor from JSON to the desired type.
    BOOST_STATIC_ASSERT(sizeof(T) == 0);
}

JsonField::JsonField(const std::string &name):
    isForSending(false), isRequiredToReceive(true), isAlreadyReceived(false), valueShouldMatch(false),
    jsonFieldRead(name), jsonFieldWrite(name), extractor(0)
{
}

JsonField::~JsonField()
{
    delete extractor;
}

/** @short Register this field for future extraction to the indicated location */
template<typename T>
JsonField &JsonField::extract(T *where)
{
    extractor = new SpecializedExtractor<T>(where);
    return *this;
}

JsonHandler::JsonHandler(const JsonApiParser * const api, const std::string &cmd): p(api)
{
    command(cmd);
}

void JsonHandler::send()
{
    json_spirit::Object o;
    BOOST_FOREACH(const JsonField &f, fields) {
        if (f.isForSending) {
            o.push_back(json_spirit::Pair(f.jsonFieldWrite, f.jsonValue));
        }
    }
    p->sendJsonObject(o);
}

void JsonHandler::receive()
{
    using namespace boost::phoenix;
    using namespace arg_names;

    BOOST_FOREACH(const Pair& node, p->readJsonObject()) {

        // At first, find a matching rule for this particular key
        std::vector<JsonField>::iterator rule =
                std::find_if(fields.begin(), fields.end(), bind(&JsonField::jsonFieldRead, arg1) == node.name_);

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

        // Extract the value from JSON
        if (rule->extractor) {
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
        throw JsonParseError(s.str());
    }
}

void JsonHandler::work()
{
    send();
    receive();
}

void JsonHandler::command(const std::string &cmd)
{
    JsonField f(j_command);
    f.jsonFieldRead = j_response;
    f.jsonValue = cmd;
    f.isForSending = true;
    f.valueShouldMatch = true;
    fields.push_back(f);
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

JsonField &JsonHandler::read(const std::string &name)
{
    JsonField f(name);
    fields.push_back(f);
    return *(--fields.end());
}

JsonField &JsonHandler::expectTrue(const std::string &name)
{
    JsonField f(name);
    fields.push_back(f);
    f.valueShouldMatch = true;
    f.jsonValue = true;
    return *(--fields.end());
}


// Template instances for the linker
template JsonField& JsonField::extract(vector<TemporaryChangesetId>*);
template JsonField& JsonField::extract(RevisionId*);
template JsonField& JsonField::extract(TemporaryChangesetId*);
template JsonField& JsonField::extract(vector<Identifier>*);
template JsonField& JsonField::extract(vector<KindAttributeDataType>*);
template JsonField& JsonField::extract(vector<ObjectRelation>*);
template JsonField& JsonField::extract(map<Identifier,Value>*);
template JsonField& JsonField::extract(map<Identifier,pair<Identifier,Value> >*);

}
}
