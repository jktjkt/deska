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

/** @short Variant visitor for converting from Deska::Db::ObjectModification to json_spirit::Value */
struct ObjectModificationToJsonValue: public boost::static_visitor<json_spirit::Value>
{
    template <typename T>
    result_type operator()(const T&) const;
};

/** @short Variant visitor for converting Deska::Db::Filter to json_spirit::Value */
struct DeskaFilterToJsonValue: public boost::static_visitor<json_spirit::Value>
{
    template <typename T>
    result_type operator()(const T&) const;
};


/** @short Define how to extract a custom JSON type into C++ class */
template<typename T>
struct JsonConversionTraits {
    static T extract(const json_spirit::Value &v);
    /** @short Specialization for certain types for converting to JSON */
    static json_spirit::Value toJson(const T& value);
};

/** @short Got to provide partial specialization in the header for RHEL5 */
template<>
inline json_spirit::Value JsonConversionTraits<RevisionId>::toJson(const RevisionId &revision)
{
    std::ostringstream ss;
    ss << revision;
    return ss.str();
}

/** @short Got to provide partial specialization in the header for RHEL5 */
template<>
inline json_spirit::Value JsonConversionTraits<TemporaryChangesetId>::toJson(const TemporaryChangesetId &revision)
{
    std::ostringstream ss;
    ss << revision;
    return ss.str();
}


// This specialization has to be mentioned in this header file
template<> struct JsonConversionTraits<RemoteDbError> {
    static void extract(const json_spirit::Value &v);
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
    virtual void extract(const json_spirit::Value &value);
};

}
}

#endif // DESKA_DB_JSON_EXTRACTION_H
