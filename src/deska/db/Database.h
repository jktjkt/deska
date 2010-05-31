#ifndef DESKA_DB_DATABASE_H
#define DESKA_DB_DATABASE_H

#include <string>
#include <vector>

namespace Deska {

namespace DB {

class BasicBoxModel;
class BasicHwModel;
class BoxModel;
class HwModel;
class Network;
class Rack;

class Database {
public:
    virtual ~Database();

    /** @short Start a new transaction for modfying the DB */
    virtual void startTransaction();
    /** @short Commit the changes to the DB */
    virtual void commit();

    /** @short Get an informal diff between the upstream and local DB state */
    virtual std::string diff() const;




    /** @short Returns a list of all valid "boxmodel" identifiers */
    virtual std::vector<std::string> getBoxModelNames() const;

    /** @short Retreive a BoxModel instance for a specified identifier */
    virtual BoxModel* getBoxModel( const std::string& name ) const;

    /** @short Create a new, empty BoxModel with the specified name */
    virtual BoxModel* createBoxModel( const std::string& name );

    /** @short Return a list of all boxmodels which inherit from a particular BoxModel template */
    virtual std::vector<std::string> getInheritedBoxModels( const std::string& name ) const;


    virtual std::vector<std::string> getHwModelNames() const;
    virtual HwModel* getHwModel( const std::string& name ) const;
    virtual HwModel* createHwModel( const std::string& name );
    virtual std::vector<std::string> getInheritedHwModels( const std::string& name ) const;

    virtual std::vector<std::string> getNetworkNames() const;
    virtual Network* getNetwork( const std::string& name ) const;
    virtual Network* createNetwork( const std::string& name );


};

}

}

#endif // DESKA_DB_DATABASE_H
