#define BOOST_TEST_MODULE example
#include <boost/test/included/unit_test.hpp>

#include "deska/db/FakeApi.h"

struct F {
    F()
    {
        using namespace Deska;
        Deska::FakeApi *fake = new FakeApi();
        fake->attrs["hardware"].push_back( KindAttributeDataType( "id", "integer" ) );
        fake->attrs["hardware"].push_back( KindAttributeDataType( "name", "string" ) );
        fake->attrs["hardware"].push_back( KindAttributeDataType( "price", "integer" ) );
        fake->attrs["interface"].push_back( KindAttributeDataType( "ip", "string" ) );
        fake->attrs["interface"].push_back( KindAttributeDataType( "mac", "string" ) );
        fake->attrs["host"].push_back( KindAttributeDataType( "hardware", "identifier" ) );
        fake->attrs["host"].push_back( KindAttributeDataType( "name", "string" ) );

        fake->relations["interface"].push_back( ObjectRelation::embedInto("host") );
        db = fake;
    }
    ~F()
    {
        delete db;
    }

    Deska::Api *db;
};


BOOST_FIXTURE_TEST_CASE( parsing_top_level_objects, F )
{

}
