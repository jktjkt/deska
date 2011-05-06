# CMake will set this variable if the unit tests are supposed to skip full DB
# drop/recreate operation before each test case is run.
# Enabling this option has certain implications; first of all, it will make the
# tests run much, much faster, especially on machines with rotating disks. On
# the other hand, it will preserve the state of the database between test cases,
# and therefore could introduce certain non-determinism to the tests. It's up to
# you to decide whether to enable this flag or not.
#
# If your test database gets hosed and you want to force a re-initialization,
# please have a look at the util-run-testcase.sh for what's the slower mode of
# operation and reproduce that by hand.
DESKA_SQL_TEST_FAST_BUT_NONDETERMINISTIC=${DESKA_SQL_TEST_FAST_BUT_NONDETERMINISTIC}

# Path to the top-level srcdir
# CMake writes this file from the context of the test directory, hence the
# requirement of going one level up.
DESKA_SOURCES=`realpath ${CMAKE_CURRENT_SOURCE_DIRECTORY}/..`
