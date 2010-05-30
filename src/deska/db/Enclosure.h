#ifndef DESKA_DB_ENCLOSURE_H
#define DESKA_DB_ENCLOSURE_H

#include <string>
#include "../Flags.h"

namespace Deska {

namespace DB {

/** @short Flags for ordering of bays inside an enclosure */
typedef enum {
    BOTTOM_TO_UP=1, /**< @short Number one at the bottom */
    UP_TO_BOTTOM=2, /**< @short No. 1 at the top */
    LEFT_TO_RIGHT=4, /**< @short No. 1 on left */
    RIGHT_TO_LEFT=8, /**< @short No. 1 on right */
    FRONT_TO_BACK=16, /**< @short No. 1 at the front side */
    BACK_TO_FRONT=32 /**< @short No. 1 at the bottom of the box */
} BayOrdering;

DESKA_DECLARE_FLAGS(BayOrderingFlags, BayOrdering)

/** @short Representation of the "enclosure" statement from the CLI
 *
 * Instances of the Enclosure class are managed by the Database object. Users
 * are not permitted to create, copy or destroy them.  The Database will create
 * them when needed and won't destroy them until the database itself ceases to
 * exist.
 * */
class Enclosure {
public:
    /** @short Returns the name of the enclosure kind */
    std::string name() const;
    /** @short Returns a pointer to an enclosure into which this one fits */
    const Enclosure* fitsIn() const;

    /** @short Returns true if the enclosure dimensions are specified in absolute values */
    bool hasAbsoluteDimensions() const;
    /** @short Returns width of the enclosure in milimeters
     *
     * Should the dimension be specified in "bay units", the
     * hasAbsoluteDimensions() returns false, the hasRelativeDimensions()
     * returns true and functions width(), height() and depth() return -1.
     */
    int width() const;
    /** @short Returns height of the enclosure in milimeters
     *
     * @see width()
     * */
    int height() const;
    /** @short Returns depth of the enclosure in milimeters
     *
     * @see width()
     * */
    int depth() const;

    /** @short Returns true if the outer dimensions of an enclosure are specified in bay units */
    bool hasRelativeDimensions() const;
    /** @short Returns number of occupied bays in the "width" direction of the parent rack */
    int occupiedBaysWidth() const;
    /** @short Returns number of occupied bays in the "height" direction of the parent rack */
    int occupiedBaysHeight() const;
    /** @short Returns number of occupied bays in the "depth" direction of the parent rack */
    int occupiedBaysDepth() const;

    /** @short Returns true if this enclosure has inner bay structure */
    bool hasInnerBays() const;
    /** @short Number of bays in the "width" direction or -1 if hasInnerBays() is false */
    int baysWidth() const;
    /** @short Number of bays in the "height" direction or -1 if hasInnerBays() is false */
    int baysHeight() const;
    /** @short Number of bays in the "width" direction or -1 if hasInnerBays() is false */
    int baysDepth() const;
    /** @short Order for the bay numbering */
    BayOrderingFlags bayOrdering() const;

private:
    // we want to control the lifetime of these objects
    Enclosure();
    Enclosure( const Enclosure& );
    ~Enclosure();
};

}

}

#endif // DESKA_DB_ENCLOSURE_H
