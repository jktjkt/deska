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
#include <string>
#include <vector>
#include <boost/optional.hpp>
#include <boost/variant.hpp>

namespace Deska {
namespace Db {

/** @short @short Value of an object's attribute
 *
 * This is the definition that should be extended when adding more supported
 * formats for attribute values.
 * */
// FIXME: Allow more types in the Value
typedef boost::optional<boost::variant<std::string, double, int/*, unsigned int*/> > Value;

/** @short Type of an object's attribute */
typedef enum {
    /** @short An identifier */
    TYPE_IDENTIFIER,
    /** @short A string of any form in quotes*/
    TYPE_QUOTED_STRING,
    /** @short A string without quotes and whitespaces */
    TYPE_SIMPLE_STRING,
    /** @short A string of any form */
    TYPE_STRING,
    /** @short Integer */
    TYPE_INT,
    /** @short Unsigned integer */
    TYPE_UINT,
    /** @short Double */
    TYPE_DOUBLE,
    /** @short IPv4 address */
    TYPE_IPV4_ADDRESS,
    /** @short IPv6 address */
    TYPE_IPV6_ADDRESS,
    /** @short IPv4 or IPv6 address */
    TYPE_IP_ADDRESS,
    /** @short MAC address */
    MAC_ADDRESS,
    /** @short Date like YYYY-MM-DD */
    TYPE_DATE
} Type;

std::ostream& operator<<(std::ostream &stream, const Type t);

/** @short Convenience typedef for Identifier, ie. something that refers to anything in the DB */
typedef std::string Identifier;

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
std::ostream& operator<<(std::ostream &stream, const KindAttributeDataType &k);

/** @short Table relations -- are these objects somehow related, and should their representation be merged in the CLI? */
typedef enum {
    /** @short This object should be merged with the other one, if one exists
     *
     * This object and the one referred are closely related, and would typically have a {0,1}:{0,1}
     * mapping, and therefore it makes sense to deal with both of them as if they were just one, at
     * least from the CLI.
     *
     * An example of such objects is "host" and "hw" -- on most machines, there would be a 1:1
     * mapping, and it makes much sense to group them together in the CLI.
     * */
    RELATION_MERGE_WITH,

    /** @short This object should be embeddable into the other one
     *
     * This object doesn't make much sense alone, it really has to "belong" into another one, but
     * the "parent" object could very well exist without the embedded thing.
     *
     * A typical example is a network interface.
     * */
    RELATION_EMBED_INTO,

    /** @short This object is a template
     *
     * This objects acts as a template, that is, it can provide partial defaults for instances of the kind defined by the
     * matching RELATION_TEMPLATIZED objects.  The template relation is a relation between two kinds of objects, and the
     * RELATION_IS_TEMPLATE and RELATION_TEMPLATIZED is used to describe both sides of that.  Users of the API can expect
     * to always find both of them.
     *
     * @see RELATION_TEMPLATIZED
     * */
    RELATION_IS_TEMPLATE,

    /** @short This object's values should be combined from its parent's values to derive a full set
     *
     * This objects supports incremental setting of values, that is, data which are not defined at
     * this level shall be looked up at the parent.
     *
     * @see RELATION_IS_TEMPLATE
     * */
    RELATION_TEMPLATIZED,

    RELATION_INVALID /**< Last, invalid item */
} ObjectRelationKind;


/** @short A pair of (kind-of-relation, table)
 *
 * Examples for a "host" would be:
 * (RELATION_MERGE_WITH, "hw")
 * ...which means that the "host" records shall contain a reference to the "hw" table, and the reference shall be formed by a
 * column named "hw" which points to the name of the object in the "hw" table. We do not define the name of the target column,
 * simply because we always point to its identifier.
 *
 * In this situation, the record will be accompanied by the corresponding relation for the "hw" object kind:
 * (RELATION_MERGE_WITH, "host")
 *
 * This is how templates work:
 * (RELATION_TEMPLATIZED, "hw-template") -- for the "hw" kind
 * (RELATION_IS_TEMPLATE, "hw") -- for the "hw-template" kind
 *
 * Whereas for the "interface":
 * (RELATION_EMBED_INTO, "host")
 * */
struct ObjectRelation
{
    /** @short Construct a RELATION_MERGE_WITH */
    static ObjectRelation mergeWith(const Identifier &target);

    /** @short Construct a RELATION_EMBED_INTO */
    static ObjectRelation embedInto(const Identifier &target);

    /** @short Construct a RELATION_IS_TEMPLATE */
    static ObjectRelation isTemplate(const Identifier &target);

    /** @short Construct a RELATION_TEMPLATIZED */
    static ObjectRelation templatized(const Identifier &target);

    /** @short Kind of relation */
    ObjectRelationKind kind;
    /** @short Name of the target table this relation refers to */
    Identifier target;

private:
    /** @short Private constructor for creating a half-baked object

    This is very much needed for ObjectRelation::embedInto.
    */
    ObjectRelation();
    ObjectRelation(const ObjectRelationKind _kind, const Identifier &_target);
};

bool operator==(const ObjectRelation &a, const ObjectRelation &b);
bool operator!=(const ObjectRelation &a, const ObjectRelation &b);
std::ostream& operator<<(std::ostream &stream, const ObjectRelation &o);


/** @short Structure for pairs kind name - object name. */
struct ObjectDefinition
{
    /** @short Constructor for an empty object for testing purposes. */
    ObjectDefinition();
    /** @short Constructor only assignes the data members.
    *
    *   @param kindName Name of the kind (eg. host)
    *   @param objectName Name of the instance of the kind (eg. hpv2)
    */
    ObjectDefinition(const Identifier &kindName, const Identifier &objectName);

    /** Name of the kind */
    Identifier kind;
    /** Name of the instance of the kind */
    Identifier name;
};

std::ostream& operator<<(std::ostream &stream, const ObjectDefinition &o);
bool operator==(const ObjectDefinition &a, const ObjectDefinition &b);
bool operator!=(const ObjectDefinition &a, const ObjectDefinition &b);


/** @short Structure for pairs attribute name - attribute value. */
struct AttributeDefinition
{
    /** @short Constructor for an empty object for testing purposes. */
    AttributeDefinition();
    /** @short Constructor only assignes the data members.
    *
    *   @param attributeName Name of the attribute (eg. ip)
    *   @param assignedValue Value of the attribute (eg. 192.168.10.56)
    */
    AttributeDefinition(const Identifier &attributeName, const Value &assignedValue);

    /** Name of the attribute */
    Identifier attribute;
    /** Value of the attribute */
    Value value;
};

std::ostream& operator<<(std::ostream &stream, const AttributeDefinition &a);
bool operator==(const AttributeDefinition &a, const AttributeDefinition &b);
bool operator!=(const AttributeDefinition &a, const AttributeDefinition &b);


/** @short Typedef for context stack. */
typedef std::vector<ObjectDefinition> ContextStack;

/** @short Function for converting context stack into name path to the object on the top.
*
*   This function is for obtaining full name from the context stack, so we can pass it to the DB.
*   
*   Example: For context stack [host hpv2, interface eth0] will the result of the function be hpv2->eth0
*
*   @param contextStack Context stack, where the top object is the one for which we want to get the path.
*   @return Identifier of a object composed from single identifiers from the context stack
*/
Identifier contextStackToPath(const ContextStack &contextStack);

/** @short Function for converting context stack into string representation.
*
*   @param contextStack Context stack to convert
*   @return String representation of the context stack composed from single object definitions
*/
std::string contextStackToString(const ContextStack &contextStack);

/** @short Function for converting object path into vector of identifiers.
*
*   @param contextStack Context stack to convert
*   @return Vector of identifiers extracted from the path
*/
std::vector<Identifier> PathToVector(const std::string &path);

}
}

#endif // DESKA_DB_OBJECTS_H
