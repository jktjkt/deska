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

#ifndef DESKA_API_H
#define DESKA_API_H

#include <map>
#include <string>
#include <vector>
#include <boost/variant.hpp>

/*
 * TODO items for the DB API:
 *
 * - Think about how to retrieve older revisions from the DB (is the default Revision=0
 *   enough/suitable?)
 * - Exceptions -- current idea is that all Deska::Api operations throw an exception upon any error
 * - Reorganize namespace and class names?
 *
 *
 * */


namespace Deska
{

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

/** @short Convenience typedef for Identifier, ie. something that refers to anything in the DB */
typedef std::string Identifier;

/** @short An identification of a persistent revision in the DB */
typedef unsigned int Revision;

/** @short Description of an attribute of a Kind object 
 *
 * This struct is a tuple of <name,datatype>, representing one attribute of a Kind object. Each Kind
 * will typically have multiple attributes.
 *
 * FIXME: rename to AttributeScheme?
 * */
struct KindAttributeDataType
{
    KindAttributeDataType( Identifier _name, Type _type ): name(_name), type(_type)
    {
    }

    Identifier name;
    Type type;
};

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

    /** @short This object's values should be combined from its parent's values to derive a full set
     *
     * This objects supports incremental setting of values, that is, data which are not defined at
     * this level shall be looked up at the parent.
     * */
    RELATION_TEMPLATE,

    RELATION_INVALID /**< Last, invalid item */
} ObjectRelationKind;


/** @short A pair of (kind-of-relation, table)
 *
 * Examples for a "host" would be:
 * (RELATION_MERGE_WITH, "hw", "hw", "host")
 *
 * Examples for a "hw":
 * (RELATION_MERGE_WITH, "host", "host", "hw")
 * (RELATION_TEMPLATE, "hw", "template", "name")
 *
 * Whereas for the "interface":
 * (RELATION_EMBED_INTO, "host")
 * */
struct ObjectRelation
{
    /** @short Construct a RELATION_EMBED_INTO */
    static ObjectRelation embedInto(const Identifier &into);

    /** @short Kind of relation */
    ObjectRelationKind kind;
    /** @short Name of the target table this relation refers to */
    Identifier targetTableName;
    /** @short From which attribute shall we match */
    Identifier sourceAttribute;
    /** @short To which attribute shall we match */
    Identifier destinationAttribute;

private:
    ObjectRelation();
    ObjectRelation(const ObjectRelationKind _kind, const Identifier &_targetTableName,
                   const Identifier &_sourceAttribute, const Identifier &_destinationAttribute);
};

/** @short Class representing the database API
 *
 * This class should contain all functionality required for working with the Deska DB.
 * */
class Api
{
public:
    virtual ~Api();

    // Querying schema definition

    /** @short Return list of names of configured top-level Kinds 
     *
     * Top-level Kinds are entities like enclosure etc, that is, object representing types.  These
     * Kinds could then be instantiated.
     * */
    virtual std::vector<Identifier> kindNames() const = 0;

    /** @short Return Attributes which are defined for a particular Kind 
     *
     * The returned data is a list of <name, datatype> pairs.
     * */
    virtual std::vector<KindAttributeDataType> kindAttributes( const Identifier &kindName ) const = 0;


    /** @short Retrieve relations between different Kinds
     *
     * This function returns a list of relations for the specified kind of entities -- for more
     * details and examples, see the ObjectRelation struct.
     * */
    virtual std::vector<ObjectRelation> kindRelations( const Identifier &kindName ) const = 0;


    // Returning data for existing objects

    /** @short Get identifiers of all concrete objects of a given Kind */
    virtual std::vector<Identifier> kindInstances( const Identifier &kindName, const Revision = 0 ) const = 0;

    /** @short Get all attributes for a named object of a particular kind
     *
     * Templates: this function should not have any knowledge of "templates"; see the
     * resolvedObjectData() for template support.
     * */
    virtual std::map<Identifier, Value> objectData(
        const Identifier &kindName, const Identifier &objectName, const Revision = 0 ) = 0;

    /** @short Get all attributes, including the inherited ones
     *
     * This function walks through the template hierarchy (@see RELATION_TEMPLATE's documentation)
     * from the bottom up and if a value of an attribute is not explicitly set at the current level,
     * it shall continue upwards until a match for all values is known.
     *
     * The return value is a map indexed by the attribute name, with values being a pair; the second
     * value in this pair is the actual attribute value, while the first one is the identifier of an
     * object which defined said attribute. Example:
     *
     * Requesting data for the "hw DL360":
     *      power_consumption: (DL360, 500W)
     *      height: (template-1U, 1)
     *      ...
     * */
    virtual std::map<Identifier, std::pair<Identifier, Value> > resolvedObjectData(
        const Identifier &kindName, const Identifier &objectName, const Revision = 0 ) = 0;

    /** @short Get a list of identifiers of objects which explicitly override a given attribute 
     *
     * This function walks the inheritance tree (@see RELATION_TEMPLATE) and checks the hierarchy for objects which
     * explicitly override a declaration of an attribute value which happened at the specified template level by a new
     * definition in the object itself.
     *
     * An example is a template "boxmodel generic-1u" which specifies the height to 1, and a derived "dl360" which has
     * anything in the "height" attribute. This function's result will include the "dl360" when asked for "what gets
     * affected by a change of the "height" attribute of the "generic-1u" boxmodel.
     *
     * Note that calling this function could be very expensive.
     *
     * @see findNonOverriddenAttrs()
     *
     * */
    virtual std::vector<Identifier> findOverriddenAttrs(
        const Identifier &kindName, const Identifier &objectName, const Identifier &attrName ) = 0;

    /** @short Get a list of identifiers of objects which would be affected by a change in an attribute
     *
     * This function serves a similar role to the findOverriddenAttrs, but looks for objects which do not specify any
     * value for the attribute in question.
     *
     * Note that calling this function could be very expensive and it could very easily return vast amounts of data.
     *
     * @see findOverriddenAttrs()
     * */
    virtual std::vector<Identifier> findNonOverriddenAttrs(
        const Identifier &kindName, const Identifier &objectName, const Identifier &attrName ) = 0;

    // Manipulating objects

    /** @short Delete an item from one of the lists of objects */
    virtual void deleteObject( const Identifier &kindName, const Identifier &objectName ) = 0;

    /** @short Create new object */
    virtual void createObject( const Identifier &kindName, const Identifier &objectname ) = 0;

    /** @short Change object's name */
    virtual void renameObject( const Identifier &kindName, const Identifier &oldName, const Identifier &newName ) = 0;

    /** @short Remove an attribute from one instance of an object */
    virtual void removeAttribute(
        const Identifier &kindName, const Identifier &objectName, const Identifier &attributeName ) = 0;

    /** @short Set an attribute that belongs to some object to a new value */
    virtual void setAttribute(
        const Identifier &kindName, const Identifier &objectName, const Identifier &attributeName, const Value &value ) = 0;



    // SCM-like operation and transaction control

    /** @short Create a temporary changeset and allow modifying the DB 
     *
     * All changes affect just the temporary revision, nothing touches the live data until the
     * commit() succeeds.
     * */
    virtual void startChangeset() = 0;

    /** @short Commit current in-progress changeset 
     *
     * This operation will commit the temporary changeset (ie. everything since the corresponding
     * startChangeset()) into the production DB, creating an identifiable revision in the process.
     * */
    virtual void commit() = 0;

    /** @short Make current in-progress changeset appear as a child of a specified revision */
    virtual void rebaseTransaction( const Revision rev ) = 0;
};

}

#endif // DESKA_API_H
