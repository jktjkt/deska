# Path to the top-level srcdir
# CMake writes this file from the context of the test directory, hence the
# requirement of going one level up.
DESKA_SOURCES=`readlink -f ${CMAKE_CURRENT_SOURCE_DIR}/..`
