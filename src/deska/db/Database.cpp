#include "Database.h"

namespace Deska {

namespace DB{

Database::~Database(){	
}

void Database::startTransaction(){
	connection = PQconnectdb("host = localhost port = 5432 dbname = deska user = common password = common");

	ConnStatusType status = PQstatus(connection);

	if(status == CONNECTION_OK)
		cout<<"database connection is ok"<<endl;
	else {
		cout<<"database was not connected"<<endl;
		PQfinish(connection);
		cout<<connection<<endl;		
	}
}

void Database::commit(){
}

    /** @short Get an informal diff between the upstream and local DB state */
std::string Database::diff() const{
	return "";
}




/** @short Returns a list of all valid "boxmodel" identifiers */
vector<string> Database::getBoxModelNames() const{
	vector<string>	 names;
	ConnStatusType status = PQstatus(connection);
	if(status == CONNECTION_OK){
		PGresult *result = PQexec(connection, "select name from deska_dev.boxmodel");
		ExecStatusType exec_status = PQresultStatus(result);
		if(exec_status != PGRES_TUPLES_OK){
			cout<<"no results for this command"<<endl;
			cout<<PQresStatus(exec_status)<<endl;
		}
		else{

			unsigned int num_tuples = PQntuples(result);
			unsigned int num_fields = PQnfields(result); 
			
			for(int i = 0; i < num_tuples; ++i){
				for(int j = 0; j < num_fields; ++j){
					string name = string(PQgetvalue(result, i, j));
					cout<<name<<endl;
					names.push_back(name);
				}
			}
			PQclear(result);
		}
	}
return names;
}

/** @short Retreive a BoxModel instance for a specified identifier */
BoxModel* Database::getBoxModel( const std::string& name ) const{
return NULL;
}

    /** @short Create a new, empty BoxModel with the specified name */
BoxModel* Database::createBoxModel( const std::string& name ){
return NULL;
}

    /** @short Return a list of all boxmodels which inherit from a particular BoxModel template */
std::vector<std::string> Database::getInheritedBoxModels( const std::string& name ) const{
	vector<string>	 vs;
return vs;
}


std::vector<std::string> Database::getHwModelNames() const{
	vector<string>	 vs;
return vs;	
}

HwModel* Database::getHwModel( const std::string& name ) const{
return NULL;
}

HwModel* Database::createHwModel( const std::string& name ){
return NULL;
}

std::vector<std::string> Database::getInheritedHwModels( const std::string& name ) const{
	vector<string>	 vs;
return vs;
}

std::vector<std::string> Database::getNetworkNames() const{
	vector<string>	 vs;
return vs;
}
Network* Database::getNetwork( const std::string& name ) const{
return NULL;
}

Network* Database::createNetwork( const std::string& name ){
return NULL;
}


}

}