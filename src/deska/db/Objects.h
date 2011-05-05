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
#include <boost/variant.hpp>

namespace Deska {
namespace Db {

/** @short @short Value of an object's attribute
 *
 * This is the definition that should be extended when adding more supported
 * formats for attribute values.
 * */
typedef boost::variant<std::string,double,int> Value;

/** @short Type of an object's attribute */
typedef enum {
    /** @short An identifier */
    TYPE_IDENTIFIER,
    /** @short A string of any form */
    TYPE_STRING,
    /** @short Integer */
    TYPE_INT,
    /** @short Double */
    TYPE_DOUBLE
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
 * (RELATION_MERGE_WITH, "hw", "hw")
 * ...which means that the "host" records shall contain a reference to the "hw" table, and the reference shall be formed by a
 * column named "hw" which points to the name of the object in the "hw" table. We do not define the name of the target column,
 * simply because we always point to its identifier.
 *
 * In this situation, the record will be accompanied by the corresponding relation for the "hw" object kind:
 * (RELATION_MERGE_WITH, "host", "host")
 *
 * This is how templates work:
 * (RELATION_TEMPLATIZED, "hw-template", "template") -- for the "hw" kind
 * (RELATION_IS_TEMPLATE, "hw") -- for the "hw-template" kind
 *
 * Whereas for the "interface":
 * (RELATION_EMBED_INTO, "host")
 * */
struct ObjectRelation
{
    /** @short Construct a RELATION_MERGE_WITH */
    static ObjectRelation mergeWith(const Identifier &targetTableName, const Identifier &sourceAttribute);

    /** @short Construct a RELATION_EMBED_INTO */
    static ObjectRelation embedInto(const Identifier &into);

    /** @short Construct a RELATION_IS_TEMPLATE */
    static ObjectRelation isTemplate(const Identifier &toWhichKind);

    /** @short Construct a RELATION_TEMPLATIZED */
    static ObjectRelation templatized(const Identifier &byWhichKind, const Identifier &sourceAttribute);

    /** @short Kind of relation */
    ObjectRelationKind kind;
    /** @short Name of the target table this relation refers to */
    Identifier targetTableName;
    /** @short From which attribute shall we match */
    Identifier sourceAttribute;

private:
    /** @short Private constructor for creating a half-baked object

    This is very much needed for ObjectRelation::embedInto.
    */
    ObjectRelation();
    ObjectRelation(const ObjectRelationKind _kind, const Identifier &_targetTableName, const Identifier &_sourceAttribute);
};

bool operator==(const ObjectRelation &a, const ObjectRelation &b);
bool operator!=(const ObjectRelation &a, const ObjectRelation &b);
std::ostream& operator<<(std::ostream &stream, const ObjectRelation &o);

}
}

#endif // DESKA_DB_OBJECTS_H
