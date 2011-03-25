#include <iostream>
#include <string>
#include "deska/db/FakeApi.h"
#include "Parser.h"



int main()
{
    using namespace Deska::Db;
    using namespace Deska::Cli;

    FakeApi *fake = new FakeApi();

    fake->attrs["hardware"].push_back( KindAttributeDataType( "id", TYPE_INT ) );
    fake->attrs["hardware"].push_back( KindAttributeDataType( "name", TYPE_STRING ) );
    fake->attrs["hardware"].push_back( KindAttributeDataType( "price", TYPE_DOUBLE ) );
    fake->attrs["interface"].push_back( KindAttributeDataType( "ip", TYPE_STRING ) );
    fake->attrs["interface"].push_back( KindAttributeDataType( "mac", TYPE_STRING ) );
    fake->attrs["host"].push_back( KindAttributeDataType( "hardware_id", TYPE_IDENTIFIER ) );
    fake->attrs["host"].push_back( KindAttributeDataType( "name", TYPE_STRING ) );

    fake->relations["interface"].push_back( ObjectRelation::embedInto("host") );


    Parser parser( fake );


    std::string str;
    std::cout << "> ";
    std::vector<ContextStackItem> context;
    while ( getline( std::cin, str ) ) {
        if ( str.size() == 1 && ( str[ 0 ] == 'q' || str[ 0 ] == 'Q' ) )
            break;
        
        parser.parseLine( str );

        context = parser.currentContextStack();
        for( std::vector<ContextStackItem>::iterator it = context.begin(); it != context.end(); ++it ) {
            if ( it != context.begin() )
                std::cout << "/";
            std::cout << it->kind << " " << it->name;
        }
        std::cout << "> ";
    }
    

    delete fake;

    return 0;
}
