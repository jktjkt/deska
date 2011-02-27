#include <iostream>
#include <string>
#include "deska/db/FakeApi.h"
#include "Parser.h"



int main()
{
    using namespace Deska;

    FakeApi *fake = new FakeApi();

    fake->attrs["hardware"].push_back( KindAttributeDataType( "id", TYPE_INT ) );
    fake->attrs["hardware"].push_back( KindAttributeDataType( "name", TYPE_STRING ) );
    fake->attrs["hardware"].push_back( KindAttributeDataType( "price", TYPE_DOUBLE ) );
    fake->attrs["interface"].push_back( KindAttributeDataType( "ip", TYPE_STRING ) );
    fake->attrs["interface"].push_back( KindAttributeDataType( "mac", TYPE_STRING ) );
    fake->attrs["host"].push_back( KindAttributeDataType( "hardware", TYPE_IDENTIFIER ) );
    fake->attrs["host"].push_back( KindAttributeDataType( "name", TYPE_STRING ) );

    Deska::CLI::Parser parser( fake );

    std::string test = "hardware abcde id 1243 name \"jmeno\" price 1234.5";
    parser.parseLine( test );

    std::cout << std::endl;

    test = "hardware abcde id xx name \"jmeno\" price 1234.5";
    parser.parseLine( test );

    std::cout << std::endl;

    test = "hardware abcde isd 123 name \"jmeno\" price 1234.5";
    parser.parseLine( test );

    std::cout << std::endl;

    test = "haware abcde id 123 name \"jmeno\" price 1234.5";
    parser.parseLine( test );
    
    /*
    while ( getline( std::cin, str ) ) {
        if ( str.empty() || str[ 0 ] == 'q' || str[ 0 ] == 'Q' )
            break;
        
        parser.parseLine( str );
    }
    */

    delete fake;

    return 0;
}
