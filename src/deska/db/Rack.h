#ifndef DESKA_DB_RACK_H
#define DESKA_DB_RACK_H

#include <string>

namespace Deska {

namespace DB {

class BoxModel;

class Rack {
public:
    std::string name() const;
    const BoxModel* kind() const;

    int occupiedBay() const;

    int occupiedLocationX() const;
    int occupiedLocationY() const;
    int occupiedLocationZ() const;

    void setOccupiedBay( const int bay );
    void setOccupiedLocationX( const int num );
    void setOccupiedLocationY( const int num );
    void setOccupiedLocationZ( const int num );

private:
    // we want to control the lifetime of these objects
    Rack();
    Rack( const Rack& );
    ~Rack();
};

}

}

#endif // DESKA_DB_RACK_H
