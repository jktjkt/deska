#include <string>
#include "Parser.h"
#include "deska/db/Api.h"
#include "deska/db/FakeApi.h"

int main()
{
    using namespace Deska;
    using namespace CLI;

    FakeApi* dbApi = new FakeApi();

    dbApi->attrs["hardware"].push_back( KindAttributeDataType( "id", "integer" ) );
    dbApi->attrs["hardware"].push_back( KindAttributeDataType( "name", "quoted_string" ) );
    dbApi->attrs["hardware"].push_back( KindAttributeDataType( "price", "double" ) );
    dbApi->attrs["interface"].push_back( KindAttributeDataType( "ip", "quoted_string" ) );
    dbApi->attrs["interface"].push_back( KindAttributeDataType( "mac", "quoted_string" ) );
    dbApi->attrs["host"].push_back( KindAttributeDataType( "hardware", "identifier" ) );
    dbApi->attrs["host"].push_back( KindAttributeDataType( "name", "quoted_string" ) );

    Parser<iterator_type> parser( dbApi );

    std::string test = "hardware abcde id 123 name \"jmeno\" price 1234.5";

    parser.parseLine( test );
    
    /*
    while ( getline( std::cin, str ) )
    {
        if ( str.empty() || str[ 0 ] == 'q' || str[ 0 ] == 'Q' )
            break;
        
        parser.parseLine( str );
    }
    */

    return 0;
}
