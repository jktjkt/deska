#include <iostream>
#include <string>
#include "deska/db/FakeApi.h"
#include "CliInteraction.h"
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

    CliInteraction cli(fake, &parser);
    cli.run();

    delete fake;

    return 0;
}
