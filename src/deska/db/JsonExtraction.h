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
    template <typename T>
    result_type operator()(const T&) const;
};


/** @short Define how to extract a custom JSON type into C++ class */
template<typename T>
struct JsonConversionTraits {};

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
