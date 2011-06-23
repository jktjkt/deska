// There's some nondetrministic woodoo going on in how the UTF handles SIGCHLD. I have no idea if
// I should blame the boost::process or whether it's the UTF which is faulty, but let's just stop
// caring about the SIGCHLD altogether.
// However, this means that we cannot use the prebuilt version, but have to use the headers
// directly.  This means slightly slower compilation and a changed build system.
#define BOOST_TEST_IGNORE_SIGCHLD
#include <boost/test/included/unit_test.hpp>
