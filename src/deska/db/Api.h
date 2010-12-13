#ifndef DESKA_API_H
#define DESKA_API_H

#include <string>
/*
 * TODO items for the DB API:
 *
 * - Decide on proper Value data type -- boost::variant?
 * - Good representation of the Type thing -- enum?
 * - Think about how to retrieve older revisions from the DB (is the default Revision=0
 *   enough/suitable?)
 * - Exceptions -- current idea is that all Deska::Api operations throw an exception upon any error
 * - Reorganize namespace and class names?
 *
 *
 * */


namespace Deska {

typedef std::string Value; // FIXME: should probably be a Variant of some kind?

typedef std::string Type; // FIXME: something like an extensible enum?

/** @short Convenience typedef for Identifier, ie. something that refers to anything in the DB */
typedef std::string Identifier;

/** @short An identification of a persistent revision in the DB */
typedef unisgned int Revision;

/** @short Description of an attribute of a Kind object 
 *
 * This struct is a tuple of <name,datatype>, representing one attribute of a Kind object. Each Kind
 * will typically have multiple attributes.
 *
 * FIXME: rename to AttributeScheme?
 * */
struct KindAttributeDataType {
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

    /** @short This object should be embeddedable into the other one
     *
     * This object doesn't make much sense alone, it really has to "belong" into another one, but
     * the "parent" object could very well exist withtout the embedded thing.
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
struct ObjectRelation {
    /** @short Kind of relation */
    ObjectRelationKind kind;
    /** @short Name of a table this relation refers to */
    Identifier tableName;
    /** @short From which attribute shall we match */
    Identifier sourceAttribute;
    /** @short To which attribute shall we match */
    Indetifier destinationAttribute;
};

/** @short Class representing the database API
 *
 * This class should contain all functionality required for working with the Deska DB.
 * */
class Api {
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
    virtual std::vector<Identifier> kindInstances( const Identifier &kindName, const Revision=0 ) const = 0;

    /** @short Get all attributes for a named object of a particular kind */
    virtual std::map<Identifier, Value> objectData( const Identifier &kindName, const Identifier &objectName, const Revision=0 ) = 0;



    // Manipulating objects

    /** @short Delete an item from one of the lists of objects */
    virtual void deleteObject( const Identifier &kindName, const Identifier &objectName ) = 0;

    /** @short Create new object */
    virtual void createObject( const Identifier &kindName, const Identifier &objectname ) = 0;

    /** @short Remove an attribute from one instance of an object */
    virtual void removeAttribute( const Identifier &kindName, const Identifier &objectName,
            const Identifier &attributeName ) = 0;

    /** @short Set an attribute that belongs to some object to a new value */
    virtual void setAttribute( const Identifier &kindName, const Identifier &objectName,
            const Identifier &attributeName, const Value &value ) = 0;



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
