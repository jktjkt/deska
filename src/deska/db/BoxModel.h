#ifndef DESKA_DB_BOXMODEL_H
#define DESKA_DB_BOXMODEL_H

#include <string>
#include "../Flags.h"

namespace Deska {

namespace DB {

/** @short Flags for ordering of bays inside a boxmodel */
typedef enum {
    BOTTOM_TO_UP=1, /**< @short Number one at the bottom */
    UP_TO_BOTTOM=2, /**< @short No. 1 at the top */
    LEFT_TO_RIGHT=4, /**< @short No. 1 on left */
    RIGHT_TO_LEFT=8, /**< @short No. 1 on right */
    FRONT_TO_BACK=16, /**< @short No. 1 at the front side */
    BACK_TO_FRONT=32 /**< @short No. 1 at the bottom of the box */
} BayOrdering;

DESKA_DECLARE_FLAGS(BayOrderingFlags, BayOrdering)

class BoxModel;

/** @short Storage container for the "boxmodel" statement without templating support
 *
 * This class should only store and return information which are explicitly
 * known at this particular level of template derivation.  All inherited data
 * shall be requested via the BoxModel class.
 * */
class BasicBoxModel {
public:
    /** @short Returns the name of the boxmodel kind */
    virtual std::string name() const;
    /** @short Returns a pointer to an boxmodel into which this one fits */
    virtual const BoxModel* fitsIn() const;

    /** @short Returns true if the enclosure dimensions are specified in absolute values */
    virtual bool hasAbsoluteDimensions() const;
    /** @short Returns width of the enclosure in milimeters
     *
     * Should the dimension be specified in "bay units", the
     * hasAbsoluteDimensions() returns false, the hasRelativeDimensions()
     * returns true and functions width(), height() and depth() return -1.
     */
    virtual int width() const;
    /** @short Returns height of the enclosure in milimeters
     *
     * @see width()
     * */
    virtual int height() const;
    /** @short Returns depth of the enclosure in milimeters
     *
     * @see width()
     * */
    virtual int depth() const;

    /** @short Returns true if the outer dimensions of an enclosure are specified in bay units */
    virtual bool hasRelativeDimensions() const;
    /** @short Returns number of occupied bays in the "width" direction of the parent rack */
    virtual int occupiedBaysWidth() const;
    /** @short Returns number of occupied bays in the "height" direction of the parent rack */
    virtual int occupiedBaysHeight() const;
    /** @short Returns number of occupied bays in the "depth" direction of the parent rack */
    virtual int occupiedBaysDepth() const;

    /** @short Returns true if this boxmodel has inner bay structure */
    virtual bool hasInnerBays() const;
    /** @short Number of bays in the "width" direction or -1 if hasInnerBays() is false */
    virtual int baysWidth() const;
    /** @short Number of bays in the "height" direction or -1 if hasInnerBays() is false */
    virtual int baysHeight() const;
    /** @short Number of bays in the "width" direction or -1 if hasInnerBays() is false */
    virtual int baysDepth() const;
    /** @short Order for the bay numbering */
    virtual BayOrderingFlags bayOrdering() const;

    virtual void setFitsIn( const BoxModel* const where );
    virtual void setAbsoluteDimensions( const bool areAbsolute );
    virtual void setWidth( const int num );
    virtual void setHeight( const int num );
    virtual void setDepth( const int num );
    virtual void setOccupiedBaysWidth( const int num );
    virtual void setOccupiedBaysHeight( const int num );
    virtual void setOccupiedBaysDepth( const int num );
    virtual void setBayOrdering( const BayOrderingFlags ordering );

private:
    // we want to control the lifetime of these objects
    BasicBoxModel();
    BasicBoxModel( const BasicBoxModel& );
    ~BasicBoxModel();
};

/** @short Representation of the "boxmodel" statement from the CLI
 *
 * Instances of the BoxModel class are managed by the Database object. Users
 * are not permitted to create, copy or destroy them.  The Database will create
 * them when needed and won't destroy them until the database itself ceases to
 * exist.
 * */
class BoxModel: public BasicBoxModel {
public:
    BoxModel* extends() const;
    void setExtends( const BoxModel* const what );
private:
    // we want to control the lifetime of these objects
    BoxModel();
    BoxModel( const BoxModel& );
    ~BoxModel();
};

}

}

#endif // DESKA_DB_BOXMODEL_H
