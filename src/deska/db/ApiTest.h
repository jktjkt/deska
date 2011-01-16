#ifndef DESKA_API_TEST_H
#define DESKA_API_TEST_H


#include "Api.h"


#include <map>
#include <string>
#include <vector>


namespace Deska
{

class ApiTest: public Api
{

    virtual std::vector<Identifier> kindNames() const
    {
    	std::vector< Identifier > testKindNames;
    	
    	testKindNames.push_back("hardware");
    	testKindNames.push_back("interface");
    	testKindNames.push_back("host");
    	
		return testKindNames;
	};

    virtual std::vector<KindAttributeDataType> kindAttributes( const Identifier &kindName ) const
    {
    	std::map< Identifier, std::vector< KindAttributeDataType > > testKindAttirbutes;
    	
		testKindAttirbutes["hardware"].push_back( KindAttributeDataType( "id", "integer" ) );
		testKindAttirbutes["hardware"].push_back( KindAttributeDataType( "name", "string" ) );
		testKindAttirbutes["hardware"].push_back( KindAttributeDataType( "price", "integer" ) );
		
		testKindAttirbutes["interface"].push_back( KindAttributeDataType( "ip", "string" ) );
		testKindAttirbutes["interface"].push_back( KindAttributeDataType( "mac", "string" ) );
		
		testKindAttirbutes["host"].push_back( KindAttributeDataType( "hardware", "identifier" ) );
		testKindAttirbutes["host"].push_back( KindAttributeDataType( "name", "string" ) );
    	
    	return testKindAttirbutes[ kindName ];
	};
	
};


}


#endif