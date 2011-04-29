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
        ${CMAKE_CURRENT_SOURCE_DIR}/sql/util-run-testcase.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/sql/test_${fname}.sql
        )
endmacro(deska_sql_test)

set(ParserTestHelpers_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/MockParserEvent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ParserTestFixture.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/JsonApiTestFixture.cpp
)

add_library(ParserTestHelpers ${ParserTestHelpers_SRCS})
target_link_libraries(ParserTestHelpers DeskaCliParser DeskaDb)

deska_test(parser-mock-objects)
deska_test(mock_streambuf)
deska_test(parser-top-level-objects)
deska_test(json_api_parser)
deska_test(json_caching_api)

if(DEFINED RUN_SQL_TESTS)
    deska_sql_test(fn-get-schema)
    deska_sql_test(versioning-one-user)
    #deska_sql_test(versioning-two-users)
endif(DEFINED RUN_SQL_TESTS)
