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

typedef unsigned int Revision;

class Database {
public:
    ~Database();

    // Transaction control functions and global modifiers

    /** @short Start a new transaction for modfying the DB */
    void startTransaction();

    /** @short Commit the changes to the DB */
    void save();

    /** @short Mark the current change set as a derived of reviison rev */
    void rebaseTransaction( const Revision rev );

    /** @short Change the notion of a current revision for acessor methods */
    void setCurrentRevision( const Revision rev );



    // Accessors: BoxModel

    /** @short Get a list of names of all BoxModels which got changed in a particular revision */
    std::vector<std::string> changedBoxModelNames( const Revision rev ) const;

    /** @short Get a list of all registered data keys for a BoxModel */
    std::vector<std::string> dataKeysBoxModel() const;

    /** @short Set allowed data keys for a BoxModel */
    void setDataKeysBoxModel( const std::vector<std::string>& allowedKeys );

    /** @short Returns a list of all valid "BoxModel" identifiers */
    std::vector<std::string> boxModelNames() const;

    /** @short Retreive a BoxModel instance for a specified identifier */
    BoxModel getBoxModel( const std::string& name ) const;

    /** @short Create a new, empty BoxModel with the specified name */
    BoxModel addBoxModel( const std::string& name );

    /** @short Change a BoxModel's name */
    void renameBoxModel( const std::string& oldName, const std::string& newName );

    /** @short Remove a BoxModel from the database */
    void removeBoxModel( const std::string& name );

    /** @short Return a list of all boxmodels which inherit from a particular BoxModel template */
    std::vector<std::string> getInheritedBoxModels( const std::string& name ) const;



    // FIXME: repeat all of the above for each Object class (BoxModel, Host,...)
};

}

}

#endif // DESKA_DB_DATABASE_H
