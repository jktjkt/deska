#include <string>
#include "deska/db/FakeApi.h"
#include "Parser.h"



int main()
{
    Deska::FakeApi *dbApi = new Deska::FakeApi();

    dbApi->attrs["hardware"].push_back( Deska::KindAttributeDataType( "id", "integer" ) );
    dbApi->attrs["hardware"].push_back( Deska::KindAttributeDataType( "name", "quoted_string" ) );
    dbApi->attrs["hardware"].push_back( Deska::KindAttributeDataType( "price", "double" ) );
    dbApi->attrs["interface"].push_back( Deska::KindAttributeDataType( "ip", "quoted_string" ) );
    dbApi->attrs["interface"].push_back( Deska::KindAttributeDataType( "mac", "quoted_string" ) );
    dbApi->attrs["host"].push_back( Deska::KindAttributeDataType( "hardware", "identifier" ) );
    dbApi->attrs["host"].push_back( Deska::KindAttributeDataType( "name", "quoted_string" ) );

    Deska::CLI::Parser<Deska::CLI::iterator_type> parser( dbApi );

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
