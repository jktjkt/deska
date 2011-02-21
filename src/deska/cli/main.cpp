#include <string>
#include "deska/db/FakeApi.h"
#include "Parser.h"



int main()
{
    using namespace Deska;

    FakeApi *fake = new FakeApi();

    fake->attrs["hardware"].push_back( KindAttributeDataType( "id", "integer" ) );
    fake->attrs["hardware"].push_back( KindAttributeDataType( "name", "quoted_string" ) );
    fake->attrs["hardware"].push_back( KindAttributeDataType( "price", "double" ) );
    fake->attrs["interface"].push_back( KindAttributeDataType( "ip", "quoted_string" ) );
    fake->attrs["interface"].push_back( KindAttributeDataType( "mac", "quoted_string" ) );
    fake->attrs["host"].push_back( KindAttributeDataType( "hardware", "identifier" ) );
    fake->attrs["host"].push_back( KindAttributeDataType( "name", "quoted_string" ) );

    Deska::CLI::Parser<CLI::iterator_type> parser( fake );

    std::string test = "hardware abcde id 1243 name \"jmeno\" price 1234.5";
    parser.parseLine( test );

    std::cout << std::endl;

    test = "hardware abcde id xx name \"jmeno\" price 1234.5";
    parser.parseLine( test );
    
    /*
    while ( getline( std::cin, str ) ) {
        if ( str.empty() || str[ 0 ] == 'q' || str[ 0 ] == 'Q' )
            break;
        
        parser.parseLine( str );
    }
    */

    return 0;
}
