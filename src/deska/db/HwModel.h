#ifndef DESKA_DB_HWMODEL_H
#define DESKA_DB_HWMODEL_H

#include <string>

namespace Deska {

namespace DB {

class BoxModel;
class HwModel;

class BasicHwModel {
public:
    /** @short Returns the name of the boxmodel kind */
    virtual std::string name() const;

    /** @short Returns a pointer to an boxmodel into which this one fits */
    virtual const BoxModel* fitsIn() const;
    virtual void setFitsIn( const BoxModel* const into );

    virtual std::string cpuDescription() const;
    virtual void setCpuDescription( const std::string* cpu );
    virtual int socketCount() const;
    virtual void setSocketCount( const int count );
    virtual int coresPerSocket() const;
    virtual void setCoresPerSocket( const int num );

    virtual std::string diskDescription() const;
    virtual void setDiskDescription( const std::string& desc );

    virtual int powerConsumption() const;
    virtual void setPowerConsumption( const int watts );

    virtual int memorySize() const;
    virtual void setMemorySize( const int gigaBytes );

    virtual std::string benchmarkResult() const;
    virtual void setBenchmarkResult( const std::string& result );


private:
    // we want to control the lifetime of these objects
    BasicHwModel();
    BasicHwModel( const BasicHwModel& );
    ~BasicHwModel();
};

/** @short Representation of the "hwmodel" statement from the CLI
 * */
class HwModel: public BasicHwModel {
public:
    HwModel* extends() const;
    void setExtends( const HwModel* const what );
private:
    // we want to control the lifetime of these objects
    HwModel();
    HwModel( const HwModel& );
    ~HwModel();
};

}

}

#endif // DESKA_DB_HWMODEL_H
