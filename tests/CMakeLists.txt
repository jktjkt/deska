# Copyright (C) 2011 Jan Kundrát <kundratj@fzu.cz>
#
# This file is part of the Deska, a tool for central administration of a grid site
# http://projects.flaska.net/projects/show/deska
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or the version 3 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
# Boston, MA 02110-1301, USA.

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

macro(deska_test fname)
    set(test_${fname}_SRCS test_${fname}.cpp)
    add_definitions(-DBOOST_TEST_DYN_LINK -DBOOST_TEST_MAIN)
    add_executable(test_${fname} ${test_${fname}_SRCS})
    target_link_libraries(test_${fname} ParserTestHelpers ${Boost_LIBRARIES})
    add_test(test_${fname} ${CMAKE_CURRENT_BINARY_DIR}/test_${fname})
endmacro(deska_test)

macro(deska_sql_test fname)
    add_test(test_sql_${fname}
        ${CMAKE_CURRENT_BINARY_DIR}/sql/util-run-testcase.sh
        sql
        ${CMAKE_CURRENT_BINARY_DIR}/sql/test_${fname}.sql
        )
endmacro(deska_sql_test)

macro(deska_dbapi_test fname)
    add_test(test_dbapi_${fname}
        ${CMAKE_CURRENT_BINARY_DIR}/sql/util-run-testcase.sh
        dbapi ${fname})
endmacro(deska_dbapi_test)

# for persistent tests
macro(deska_dbapi_persist_test fname)
    add_test(test_dbapi_${fname}
        ${CMAKE_CURRENT_BINARY_DIR}/sql/util-run-testcase.sh
        persist ${fname})
endmacro(deska_dbapi_persist_test)

macro(deska_cli_test fname)
    # See comments in tests/fix_boost_test_sigchld.h for why this is different
    # from regular boost::test usage
    set(test_cli_${fname}_SRCS test_cli_${fname}.cpp)
    add_executable(test_cli_${fname} ${test_cli_${fname}_SRCS})
    target_link_libraries(test_cli_${fname} ParserTestHelpers CliTestHelpers)
    add_test(test_cli_${fname}
        ${CMAKE_CURRENT_BINARY_DIR}/sql/util-run-testcase.sh
        run ${CMAKE_CURRENT_BINARY_DIR}/test_cli_${fname})
endmacro(deska_cli_test)

set(ParserTestHelpers_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/MockParserEvent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ParserTestFixture.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/JsonApiTestFixture.cpp
)

set(CliTestHelpers_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/MockCliEvent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CliTestFixture.cpp
)

add_library(ParserTestHelpers ${ParserTestHelpers_SRCS})
add_library(CliTestHelpers ${CliTestHelpers_SRCS})
target_link_libraries(ParserTestHelpers DeskaCliParser DeskaDb)

deska_test(parser-mock-objects)
deska_test(mock_streambuf)
deska_test(parser-top-level-objects)
deska_test(json_api_parser)
deska_test(json_caching_api)

if(RUN_SQL_TESTS)
    message(STATUS "Will run SQL database tests upon each `make test` run.")
    if( DESKA_SQL_TEST_FAST_BUT_NONDETERMINISTIC)
        message(STATUS "The database re-initialization will not be repeated")
        # careful here, got to use *another* variable name
        set(DESKA_SQL_TEST_FAST_BUT_NONDETERMINISTIC_CMAKE 1)
    else()
        message(STATUS "Will re-initialize the database before each test")
        unset(DESKA_SQL_TEST_FAST_BUT_NONDETERMINISTIC_CMAKE)
    endif()

    # A dummy test at first, just for the DB init, so that it doesn't affect
    # test timings, and to make sure that DB setup works
    deska_sql_test(empty)

    deska_sql_test(fn-get-schema)
    deska_sql_test(versioning-one-user)
    #deska_sql_test(versioning-two-users)
    deska_sql_test(versioning-pendingchangeset)
    deska_sql_test(versioning-pendingchangeset_data)
    deska_sql_test(versioned-data)
    deska_sql_test(set_attribute)
    deska_sql_test(undel)
    deska_sql_test(set_name)

    deska_dbapi_test(dbScheme)
    deska_dbapi_test(pendingChangesets)
    deska_dbapi_test(createObjects)

    # unit tests which work on any reasonable state of the DB, no matter what
    # changesets are already present, etc
    deska_dbapi_test(persist_kindInstances)
    deska_dbapi_test(persist_metadata)
    deska_dbapi_test(persist_versioning)

    # these will be removed later on
    deska_dbapi_persist_test(test_2_filter)
    deska_dbapi_persist_test(test_2_diff)
    deska_dbapi_persist_test(test_2_objectData)

    # Tests for interaction between the CLI and the DB parts
    deska_cli_test(io)
endif()

# Copy files from source directory to destination directory, substituting any
# variables.  Create destination directory if it does not exist.
macro(deska_copy_files srcDir destDir)
    message(STATUS "Copying directory ${destDir}")
    make_directory(${destDir})

    file(GLOB templateFiles RELATIVE ${srcDir} ${srcDir}/*)
    foreach(templateFile ${templateFiles})
        set(srcTemplatePath ${srcDir}/${templateFile})
        if(NOT IS_DIRECTORY ${srcTemplatePath})
            configure_file(
                    ${srcTemplatePath}
                    ${destDir}/${templateFile}
                    @ONLY)
        endif(NOT IS_DIRECTORY ${srcTemplatePath})
    endforeach(templateFile)
endmacro(deska_copy_files)

deska_copy_files( ${CMAKE_CURRENT_SOURCE_DIR}/sql/ ${CMAKE_CURRENT_BINARY_DIR}/sql )
message(STATUS "Writing out test configuration")

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/sql/util-cmake-options-template.sh
    ${CMAKE_CURRENT_BINARY_DIR}/sql/util-cmake-options.sh
)
