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

#ifndef DESKA_JSONHANDLER_H
#define DESKA_JSONHANDLER_H

#include <boost/optional.hpp>
#include <tr1/memory>
#include "deska/db/Objects.h"
#include "deska/db/ObjectModification.h"
#include "deska/db/Revisions.h"
#include "json_spirit/json_spirit_value.h"

namespace Deska {
namespace Db {

class JsonApiParser;
class JsonExtractor;

/** @short Helper class for type-safe parsing of JSON to a map of Deska object attributes */
struct JsonWrappedAttributeMap
{
    std::vector<KindAttributeDataType> dataTypes;
    std::map<Identifier, Value> attributes;
    JsonWrappedAttributeMap(const std::vector<KindAttributeDataType> dataTypes);
};

/** @short Helper class for type-safe parsing of JSON into a representaiton usable for resolvedObjectData() DBAPI method */
struct JsonWrappedAttributeMapWithOrigin
{
    std::vector<KindAttributeDataType> dataTypes;
    std::map<Identifier, std::pair<Identifier, Value> > attributes;
    JsonWrappedAttributeMapWithOrigin(const std::vector<KindAttributeDataType> dataTypes);
};

/** @short Helper class for type-safe parsing of JSON to Deska object attribute */
struct JsonWrappedAttribute
{
    Type dataType;
    Identifier attrName;
    Value value;
    JsonWrappedAttribute(const Type dataType_, const Identifier &attrName_);
};

/** @short Helper class extending the JsonWrappedAttribute with information about the origin of the attribute value */
struct JsonWrappedAttributeWithOrigin: public JsonWrappedAttribute
{
    Identifier origin;
    JsonWrappedAttributeWithOrigin(const Type dataType_, const Identifier &attrName_);
};

/** @short Helper class for adding attribute datatype information into object modification record */
struct JsonWrappedObjectModification
{
    const std::map<Identifier, std::vector<KindAttributeDataType> > *dataTypesOfEverything;

    // No default constructor, so we have to make it optional
    boost::optional<ObjectModification> diff;

    JsonWrappedObjectModification(const std::map<Identifier, std::vector<KindAttributeDataType> > *dataTypesOfEverything_);

    JsonWrappedAttribute wrappedAttribute(const Identifier &kindName, const Identifier &attributeName) const;
};

/** @short Helper class adding attribute type information to the list of object modifications */
struct JsonWrappedObjectModificationSequence
{
    const std::map<Identifier, std::vector<KindAttributeDataType> > *dataTypesOfEverything;
    std::vector<ObjectModification> diff;
    JsonWrappedObjectModificationSequence(const std::map<Identifier, std::vector<KindAttributeDataType> > *dataTypesOfEverything_);
};

/** @short Expecting/requiring/checking/sending one JSON record */
struct JsonField
{
    bool isForSending;
    bool isRequiredToReceive;
    bool isAlreadyReceived;
    bool valueShouldMatch;
    std::string jsonFieldRead, jsonFieldWrite;
    json_spirit::Value jsonValue;
    std::tr1::shared_ptr<JsonExtractor> extractor;

    JsonField(const std::string &name);

    template<typename T> JsonField &extract(T *where);
};

/** @short Manager controlling the JSON interaction

The JsonHandler class is used to drive conversions between the JSON serialized objects and their C++ representation.
It deals with JSON *objects* only, ie. it is usable just for a colection of (key, value) pairs.

Use the write() method for specifying a field which shall be converted to JSON, and read() for expecting a value under
a particular name. Both of these methods return a reference to JsonField, a class which can be used to fine-tune the
behavior of the JSON convertor. Note that these fields are internally stored in a vector, which is explicitly free to
re-order them, so don't store a reference to a JsonField anywhere, as it could easily become a dangling one.

By default, the JsonHandler will throw an exception when it sees a field which has not been defined. Use the
failOnUnknownFields() function to silently accept them.

As described in the Deska documentation, the rules for object serialization are derived from the type of the target
object. See the documentation of internal templated traits class JsonConversionTraits for how this conversion works,
and consult JsonExtractor/SpecializedExtractor for wrappers that actually save stuff.
*/
class JsonHandler
{
public:
    JsonHandler();
    virtual ~JsonHandler();

    /** @short Parser this value and process the response */
    void parseJsonObject(const json_spirit::Object &jsonObject);

    /** @short Register a JSON field which will be sent and its presence required and value checked upon arrival */
    JsonField &write(const std::string &name, const std::string &value);

    /** @short Register a JSON field which will be sent and its presence required and value checked upon arrival */
    JsonField &write(const std::string &name, const RevisionId value);

    /** @short Register a JSON field which will be sent and its presence required and value checked upon arrival */
    JsonField &write(const std::string &name, const TemporaryChangesetId value);

    /** @short Register a JSON field which will be sent and its presence required and value checked upon arrival */
    JsonField &write(const std::string &name, const Deska::Db::Value &value);

    /** @short Register a JSON field which will be sent and its presence required and value checked upon arrival */
    JsonField &write(const std::string &name, const std::vector<Deska::Db::ObjectModification> &value);

    /** @short Expect a required value in the JSON */
    JsonField &read(const std::string &name);

    /** @short Tell this handler whether it should throw an exception when it sees an unrecognized field */
    void failOnUnknownFields(const bool shouldThrow);

    /** @short If the passed revision is not zero, forward write(), otherwise return empty optional */
    boost::optional<JsonField&> writeIfNotZero(const std::string &name, const RevisionId value);

    /** @short If the passed temporary changeset is not zero, forward write(), otherwise return empty optional */
    boost::optional<JsonField&> writeIfNotZero(const std::string &name, const TemporaryChangesetId value);

protected:
    std::vector<JsonField> fields;
    bool m_failOnUnknownFields;
};

/** @short Specialization of Jsonhandler which tightly integrates with the JSON API for I/O operations

This class is set up with a few convenience functions which allow for seamless integration with the JsonApiParser class,
most notably for the IO operations.  Its constructor is also extended with a "command" specifier, which automatically
sets up a field for the command/response parsing.
*/
class JsonHandlerApiWrapper: public JsonHandler
{
public:
    JsonHandlerApiWrapper(const JsonApiParser * const api, const std::string &cmd);

    /** @short Create JSON string and send it as a command */
    void send();

    /** @short Request, read and parse the JSON string and process the response */
    void receive();

    /** @short Send and receive the JSON data */
    void work();

    /** @short Register a special JSON field for command/response identification */
    void command(const std::string &cmd);

    /** @short Check the JSON object for a possible exception embedded in the response */
    void processPossibleException(const json_spirit::Object &jsonObject);

private:
    const JsonApiParser * const p;
};


}
}

#endif // DESKA_DESKA_JSONHANDLER_H
