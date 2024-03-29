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

#ifndef DESKA_DB_OBJECTS_H
#define DESKA_DB_OBJECTS_H

#include <iosfwd>
#include <set>
#include <string>
#include <boost/asio/ip/address_v4.hpp>
#include <boost/asio/ip/address_v6.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include "MacAddress.h"
#include "deska/db/AdditionalValueStreamOperators.h"

namespace Deska {
namespace Db {

/** @short Convenience typedef for Identifier, ie. something that refers to anything in the DB */
typedef std::string Identifier;

/** @short INTERNAL: variant forming the core of the Deska::Db::Value

You're supposed to use the Value typedef in your code.
*/
typedef boost::variant<
    // Primitive types
    std::string, double, int, bool,
    // Network addresses
    boost::asio::ip::address_v4, boost::asio::ip::address_v6, MacAddress,
    // Date and time
    boost::posix_time::ptime, boost::gregorian::date,
    // identifier_set
    std::set<Identifier>
> NonOptionalValue;

/** @short Value of an object's attribute
 *
 * This is the definition that should be extended when adding more supported
 * formats for attribute values.
 * */
typedef boost::optional<NonOptionalValue> Value;

/** @short Return a Python string representation of a Deska::Db::Value */
std::string repr_NonOptionalValue(const NonOptionalValue &v);
std::string repr_Value(const Deska::Db::Value &v);
std::string str_Value(const Deska::Db::Value &v);

/** @short Type of an object's attribute */
typedef enum {
    /** @short An identifier */
    TYPE_IDENTIFIER,
    /** @short A set of identifiers */
    TYPE_IDENTIFIER_SET,
    /** @short A string of any form */
    TYPE_STRING,
    /** @short Integer */
    TYPE_INT,
    /** @short Double */
    TYPE_DOUBLE,
    /** @short Boolean value */
    TYPE_BOOL,
    /** @short IPv4 address */
    TYPE_IPV4_ADDRESS,
    /** @short IPv6 address */
    TYPE_IPV6_ADDRESS,
    /** @short MAC address */
    TYPE_MAC_ADDRESS,
    /** @short Date like YYYY-MM-DD */
    TYPE_DATE,
    /** @short Timestamp like YYYY-MM-DD HH:MM:SS */
    TYPE_TIMESTAMP
} Type;

std::ostream& operator<<(std::ostream &stream, const Type t);

/** @short Description of an attribute of a Kind object 
 *
 * This struct is a tuple of <name,datatype>, representing one attribute of a Kind object. Each Kind
 * will typically have multiple attributes.
 *
 * FIXME: rename to AttributeScheme?
 * */
struct KindAttributeDataType
{
    KindAttributeDataType(Identifier _name, Type _type): name(_name), type(_type)
    {
    }

    Identifier name;
    Type type;
};

bool operator==(const KindAttributeDataType &a, const KindAttributeDataType &b);
bool operator!=(const KindAttributeDataType &a, const KindAttributeDataType &b);
bool operator<(const KindAttributeDataType &a, const KindAttributeDataType &b);
std::ostream& operator<<(std::ostream &stream, const KindAttributeDataType &k);

/** @short Table relations -- are these objects somehow related, and should their representation be merged in the CLI? */
typedef enum {
    /** @short This object should be embeddable into the other one
     *
     * This object doesn't make much sense alone, it really has to "belong" into another one, but
     * the "parent" object could very well exist without the embedded thing.
     *
     * A typical example is a network interface.
     * */
    RELATION_EMBED_INTO,

    /** @short There's a relation between the two kinds
     *
     * The current kind contains a reference to the other kind, using the name of the other kind as the attribute name.
     * */
    RELATION_REFERS_TO,

    /** @short This object's values should be combined from its parent's values to derive a full set
     *
     * This objects supports incremental setting of values, that is, data which are not defined at
     * this level shall be looked up at the parent.
     * */
    RELATION_TEMPLATIZED,

    /** @short This object can "include" the referenced instance as a part in the composition pattern */
    RELATION_CONTAINS,

    /** @short This object is embeddable into another object */
    RELATION_CONTAINABLE,

    RELATION_INVALID /**< Last, invalid item */
} ObjectRelationKind;


/** @short A tuple of (kind-of-relation, table, column)
 *
 * This is how templates work:
 * (RELATION_TEMPLATIZED, "hw-template", "template") -- for the "hw" kind
 *
 * Whereas for the "interface":
 * (RELATION_EMBED_INTO, "host", "host")
 *
 * Finally, to model generic relations (just a foreign key in the database table), use a RELATION_REFERS_TO. For example, if a "hw"
 * table has a foregin key "vendor" which references the "vendor" table, the "hw" kind will have the following relation record:
 * (RELATION_REFERS_TO, "vendor", "manufacturer")
 * */
struct ObjectRelation
{
    /** @short Construct a RELATION_EMBED_INTO */
    static ObjectRelation embedInto(const Identifier &target, const Identifier &column);

    /** @short COnstruct a RELATION_REFERS_TO */
    static ObjectRelation refersTo(const Identifier &target, const Identifier &column);

    /** @short Construct a RELATION_TEMPLATIZED */
    static ObjectRelation templatized(const Identifier &target, const Identifier &column);

    /** @short Construct a RELATION_CONTAINS */
    static ObjectRelation contains(const Identifier &target, const Identifier &column);

    /** @short Construct a RELATION_CONTAINABLE */
    static ObjectRelation containable(const Identifier &target, const Identifier &column);

    /** @short Kind of relation */
    ObjectRelationKind kind;
    /** @short Name of the target table this relation refers to */
    Identifier target;
    /** @short Column in the source table which provides this reference */
    Identifier column;

private:
    ObjectRelation(const ObjectRelationKind _kind, const Identifier &_target, const Identifier &_column);
};

bool operator==(const ObjectRelation &a, const ObjectRelation &b);
bool operator!=(const ObjectRelation &a, const ObjectRelation &b);
bool operator<(const ObjectRelation &a, const ObjectRelation &b);
std::ostream& operator<<(std::ostream &stream, const ObjectRelation &o);

}
}

#endif // DESKA_DB_OBJECTS_H
