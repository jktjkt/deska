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
#include "deska/db/Revisions.h"
#include "3rd-party/json_spirit_4.04/json_spirit/json_spirit_value.h"

namespace Deska {
namespace Db {

class JsonApiParser;
class JsonExtractor;

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

/** @short Manager controlling the JSON interaction */
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

    /** @short Expect a required value in the JSON */
    JsonField &read(const std::string &name);

    /** @short Require a JSON value with value of true */
    JsonField &expectTrue(const std::string &name);

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

private:
    const JsonApiParser * const p;
};


}
}

#endif // DESKA_DESKA_JSONHANDLER_H
