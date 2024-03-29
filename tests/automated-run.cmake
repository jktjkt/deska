# File for automated nightly builds of Deska, along with test execution and Valgrind coverage
# Adjust to taste.

SET($ENV{LC_MESSAGES} "en_EN")
SET(CTEST_START_WITH_EMPTY_BINARY_DIRECTORY TRUE)

SET(CTEST_SOURCE_DIRECTORY "path-to-/deska_src")
SET(CTEST_BINARY_DIRECTORY "path-to-/deska_build")

set(CTEST_PROJECT_NAME "Deska")
set(CTEST_CMAKE_GENERATOR "Unix Makefiles")

set(WITH_MEMCHECK TRUE)
set(WITH_COVERAGE TRUE)

#######################################################################

ctest_empty_binary_directory(${CTEST_BINARY_DIRECTORY})

find_program(CTEST_GIT_COMMAND NAMES git)
find_program(CTEST_COVERAGE_COMMAND NAMES gcov)
find_program(CTEST_MEMORYCHECK_COMMAND NAMES valgrind)

#set(CTEST_MEMORYCHECK_SUPPRESSIONS_FILE ${CTEST_SOURCE_DIRECTORY}/tests/valgrind.supp)

if(NOT EXISTS "${CTEST_SOURCE_DIRECTORY}")
  set(CTEST_CHECKOUT_COMMAND "${CTEST_GIT_COMMAND} clone http://repo.or.cz/r/deska.git ${CTEST_SOURCE_DIRECTORY}")
endif()

set(CTEST_UPDATE_COMMAND "${CTEST_GIT_COMMAND}")

set(CTEST_CONFIGURE_COMMAND "${CMAKE_COMMAND} -DWITH_TESTING:BOOL=ON ${CTEST_BUILD_OPTIONS}")
set(CTEST_CONFIGURE_COMMAND "${CTEST_CONFIGURE_COMMAND} \"-G${CTEST_CMAKE_GENERATOR}\"")
# Running the SQL tests currently requires PgSQL roles to be set up. You're
# supposed to `cd tests/sql/; ./util-create-shared-roles.sh` exactly once to
# create the deska_admin and deska_user roles.
set(CTEST_CONFIGURE_COMMAND "${CTEST_CONFIGURE_COMMAND} -DRUN_SQL_TESTS=1")
set(CTEST_CONFIGURE_COMMAND "${CTEST_CONFIGURE_COMMAND} \"${CTEST_SOURCE_DIRECTORY}\"")

ctest_start("Nightly")
ctest_update()
ctest_configure()
ctest_build()
ctest_test()
if (WITH_MEMCHECK AND CTEST_COVERAGE_COMMAND)
  ctest_coverage()
endif (WITH_MEMCHECK AND CTEST_COVERAGE_COMMAND)
if (WITH_MEMCHECK AND CTEST_MEMORYCHECK_COMMAND)
  ctest_memcheck()
endif (WITH_MEMCHECK AND CTEST_MEMORYCHECK_COMMAND)
ctest_submit()
