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
    fake->attrs["host"].push_back( KindAttributeDataType( "hardware_id", TYPE_IDENTIFIER ) );
    fake->attrs["host"].push_back( KindAttributeDataType( "name", TYPE_STRING ) );

    fake->relations["interface"].push_back( ObjectRelation::embedInto("host") );


    Deska::CLI::Parser parser( fake );

    std::string test;

    // unit test: error_in_datatype_of_first_inline
    test = "hardware abcde id xx name \"jmeno\" price 1234.5";
    parser.parseLine( test );
    parser.clearContextStack();

    std::cout << std::endl;

    // unit test: error_in_first_attr_name_inline
    test = "hardware abcde isd 123 name \"jmeno\" price 1234.5";
    parser.parseLine( test );
    parser.clearContextStack();

    std::cout << std::endl;

    // unit test: error_toplevel_name
    test = "haware abcde id 123 name \"jmeno\" price 1234.5";
    parser.parseLine( test );
    parser.clearContextStack();

    std::cout << std::endl;

    // unit test: nested_interface
    test = "host abcde";
    parser.parseLine( test );
    test = "name \"as123\"";
    parser.parseLine( test );
    test = "interface eth0";
    parser.parseLine( test );
    test = "mac \"nejakamac\"";
    parser.parseLine( test );
    parser.clearContextStack();

    std::cout << std::endl;
    
    test = "hardware abcde id 123 name \"jmeno\" interface eth0 mac \"nejakamac\" price 1234.5";
    parser.parseLine( test );
    parser.clearContextStack();

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
