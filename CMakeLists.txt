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


project(deska)

cmake_minimum_required(VERSION 2.6)

SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -ggdb -pipe")
# STL debugging breaks ABI compatibility with virtualy any library that uses STL, hence disabled
# add_definitions(-D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC)
#SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weffc++")
#SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -Wnon-virtual-dtor -Wold-style-cast -Woverloaded-virtual -fdiagnostics-show-option" )

set(CMAKE_CONFIGURATION_TYPES ${CMAKE_CONFIGURATION_TYPES} Coverage )
set(CMAKE_CXX_FLAGS_COVERAGE "${CMAKE_CXX_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")
set(CMAKE_C_FLAGS_COVERAGE "${CMAKE_C_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")
set(CMAKE_EXE_LINKER_FLAGS_COVERAGE "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
      "Choose the type of build"
      FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

add_definitions(-DCMAKE_CURRENT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

# RHEL5 with EPEL Boost
SET(BOOST_INCLUDEDIR /usr/include/boost141)
SET(BOOST_LIBRARYDIR /usr/lib64/boost141)
FIND_PACKAGE(Boost 1.41.0 REQUIRED COMPONENTS system date_time unit_test_framework python)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif(Boost_FOUND)
# we do not want to link with whole boost here; there's simply no point in our libraries bringing the test framework in
set(DESKA_BOOST_LIBS ${Boost_SYSTEM_LIBRARY} ${Boost_DATE_TIME_LIBRARY})

FIND_PATH(LIBEBT_PATH libebt/libebt.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/src/3rd-party/libebt-1.3.0)
if(NOT LIBEBT_PATH)
    message(FATAL "Cannot find libebt, and the internal copy seems unusable")
else()
    message(STATUS "Using libebt from ${LIBEBT_PATH}")
    include_directories(${LIBEBT_PATH})
endif()

FIND_PATH(JSON_SPIRIT_PATH json_spirit/json_spirit_value.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/3rd-party/json_spirit_4.04)
if(NOT JSON_SPIRIT_PATH)
    message(FATAL "Cannot find json_spirit, and the internal copy seems unusable")
else()
    message(STATUS "Using json_spirit from ${JSON_SPIRIT_PATH}")
    include_directories(${JSON_SPIRIT_PATH})
endif()

FIND_PATH(BOOST_PROCESS_PATH boost/process.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/3rd-party/process)
if(NOT BOOST_PROCESS_PATH)
    message(FATAL "Cannot find boost::process, and the internal copy seems unusable")
else()
    message(STATUS "Using boost::process from ${BOOST_PROCESS_PATH}")
    include_directories(${BOOST_PROCESS_PATH})
endif()

SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
FIND_PACKAGE(Readline REQUIRED)
if(EXTRA_READLINE_LIBRARY)
    # support extra library to link with due to the RHEL5 borkage
    # https://bugzilla.redhat.com/show_bug.cgi?id=499837
    set(READLINE_LIBRARY ${READLINE_LIBRARY} ${EXTRA_READLINE_LIBRARY})
endif()

# We have to bring in Python explicitly
find_package(PythonLibs REQUIRED)
# yay, different versions of this package use different variable names for increased fun
include_directories(${PYTHON_INCLUDE_DIR} ${PYTHON_INCLUDE_PATH})
#link_directories(${PYTHON_LIBRARIES})

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set(libDeskaConfig_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/CliConfig.cpp
)

set(libDeskaDb_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/db/Objects.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/db/MacAddress.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/db/Revisions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/db/ObjectModification.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/db/Filter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/db/Api.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/db/FakeApi.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/db/JsonApi.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/db/JsonHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/db/JsonExtraction.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/db/CachingJsonApi.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/db/IOSocket.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/db/ProcessIO.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/db/Connection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/db/Connection_p.cpp
)

set(libDeskaCliParser_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/ContextStack.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/CliObjects.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/PredefinedRules.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/InfoExtractor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/Parser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/Parser_p.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/Parser_p_AttributeRemovalsParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/Parser_p_AttributesSettingParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/Parser_p_IdentifiersSetsParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/Parser_p_AttributesParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/Parser_p_FunctionWordsParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/Parser_p_KindsOnlyParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/Parser_p_KindsConstructParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/Parser_p_FilterExpressionsParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/Parser_p_FiltersParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/Parser_p_KindsFiltersParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/Parser_p_WholeKindParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/Parser_p_KindsParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/Parser_p_TopLevelParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/ParserErrors.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/Exceptions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/DbInteraction.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/ParserSignals.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/CliCommands.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/CliCommands_Log.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/CliCommands_Rebase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/UserInterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/UserInterfaceIO.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/ReadlineWrapper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/ChildProcess.cpp
)

set(deskaCli_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/main.cpp
)

set(libLowLevelPyDeska_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/LowLevelPyDeska/Module.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/LowLevelPyDeska/Value.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/LowLevelPyDeska/Filter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/LowLevelPyDeska/DateTimeConversions.cpp
)

add_library(DeskaConfig SHARED ${libDeskaConfig_SRCS})
target_link_libraries(DeskaConfig ${DESKA_BOOST_LIBS})
add_library(DeskaDb SHARED ${libDeskaDb_SRCS})
target_link_libraries(DeskaDb ${DESKA_BOOST_LIBS})
add_library(DeskaCliParser ${libDeskaCliParser_SRCS})

add_executable(deska-cli ${deskaCli_SRCS})
target_link_libraries(deska-cli DeskaCliParser DeskaDb ${READLINE_LIBRARY})

add_library(LowLevelPyDeska SHARED ${libLowLevelPyDeska_SRCS})
target_link_libraries(LowLevelPyDeska DeskaDb ${DESKA_BOOST_LIBS} ${Boost_PYTHON_LIBRARY} ${PYTHON_LIBRARIES})

include(CTest)
add_subdirectory(tests)
