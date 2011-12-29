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
SET(DESKA_VERSION 0.11)
SET(DESKA_SOVERSION 0.10)

cmake_minimum_required(VERSION 2.6)

SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -ggdb -pipe")
# boost::optional fails with strict aliasing, unfortunately.
# http://boost.2283326.n4.nabble.com/optional-Strict-Aliasing-Warnings-on-Trunk-td2644713i20.html
# http://gcc.gnu.org/bugzilla/show_bug.cgi?id=41874
SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-strict-aliasing")
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
FIND_PACKAGE(Boost 1.41.0 REQUIRED COMPONENTS system date_time unit_test_framework python program_options)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif(Boost_FOUND)
# we do not want to link with whole boost here; there's simply no point in our libraries bringing the test framework in
set(DESKA_BOOST_LIBS ${Boost_SYSTEM_LIBRARY} ${Boost_DATE_TIME_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY})

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
if(READLINE_FOUND)
    include_directories(${READLINE_INCLUDE_DIR})
endif()

# We have to bring in Python explicitly
find_package(PythonLibs REQUIRED)
# yay, different versions of this package use different variable names for increased fun
include_directories(${PYTHON_INCLUDE_DIR} ${PYTHON_INCLUDE_PATH})
#link_directories(${PYTHON_LIBRARIES})

if(PYTHON_SITE_PACKAGES)
    message(STATUS "Python site-packages dir specified by hand: ${PYTHON_SITE_PACKAGES}")
else()
    # We also require the interpreter to be able to tell the PYTHONPATH
    find_package(PythonInterp REQUIRED)
    if(PYTHONINTERP_FOUND)
        execute_process(COMMAND ${PYTHON_EXECUTABLE} -c
            "import distutils.sysconfig; import sys; sys.stdout.write(distutils.sysconfig.get_python_lib())"
            OUTPUT_VARIABLE PYTHON_SITE_PACKAGES)
        message(STATUS "Python site-packages target: ${PYTHON_SITE_PACKAGES}")
    else()
        message(FATAL "Couldn't determine the PYTHON_SITE_PACKAGES directory")
    endif()
endif()

if(PYTHON_SITE_PACKAGES_ARCH)
    message(STATUS "Python arch-specific site-packages dir specified by hand: ${PYTHON_SITE_PACKAGES_ARCH}")
else()
    # We also require the interpreter to be able to tell the PYTHONPATH
    find_package(PythonInterp REQUIRED)
    if(PYTHONINTERP_FOUND)
        execute_process(COMMAND ${PYTHON_EXECUTABLE} -c
            "import distutils.sysconfig; import sys; sys.stdout.write(distutils.sysconfig.get_python_lib(1))"
            OUTPUT_VARIABLE PYTHON_SITE_PACKAGES_ARCH)
        message(STATUS "Python arch-specific site-packages target: ${PYTHON_SITE_PACKAGES_ARCH}")
    else()
        message(FATAL "Couldn't determine the PYTHON_SITE_PACKAGES_ARCH directory")
    endif()
endif()


INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
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
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/db/JsonException.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/db/JsonExtraction.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/db/CachingJsonApi.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/db/IOSocket.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/db/ProcessIO.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/db/UnixFdIO.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/db/Connection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/db/Connection_p.cpp
)

set(libDeskaCli_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/PredefinedRules.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/InfoExtractor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/ContextStack.cpp
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
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/CliConfigBase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/CliObjects.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/DbInteraction.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/ParserSignals.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/CliCommands.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/CliCommands_Log.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/CliCommands_DiffRebase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/UserInterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/UserInterfaceIO.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/ReadlineWrapper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/ChildProcess.cpp
)

set(deskaCliApp_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/CliConfig.cpp
)

set(libLowLevelPyDeska_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/LowLevelPyDeska/Module.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/LowLevelPyDeska/Value.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/LowLevelPyDeska/Filter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/LowLevelPyDeska/DateTimeConversions.cpp
)

add_library(DeskaDb SHARED ${libDeskaDb_SRCS})
target_link_libraries(DeskaDb ${DESKA_BOOST_LIBS})
set_target_properties(DeskaDb PROPERTIES
    VERSION ${DESKA_SOVERSION}
    SOVERSION ${DESKA_SOVERSION}
)

add_library(DeskaCli SHARED ${libDeskaCli_SRCS})
target_link_libraries(DeskaCli DeskaDb ${READLINE_LIBRARY})
set_target_properties(DeskaCli PROPERTIES
    VERSION ${DESKA_SOVERSION}
    SOVERSION ${DESKA_SOVERSION}
)

add_executable(deska-cli ${deskaCliApp_SRCS})
target_link_libraries(deska-cli DeskaCli)

find_file(GIT_DOT_GIT_HEAD HEAD PATHS ${CMAKE_CURRENT_SOURCE_DIR}/.git
    NO_DEFAULT_PATH NO_CMAKE_ENVIRONMENT_PATH NO_CMAKE_PATH
    NO_CMAKE_ENVIRONMENT_PATH NO_SYSTEM_PATH)
if(GIT_DOT_GIT_HEAD)
    add_custom_target(gitversion
        "${CMAKE_CURRENT_SOURCE_DIR}/git-generate-version.sh" "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/src/deska/cli/gitversion-include.cpp"
        )
    add_dependencies(deska-cli gitversion)
    set(DESKA_HAVE_GITVERSION 1)
else()
    set(DESKA_HAVE_GITVERSION 0)
endif()

SET(DESKA_VERSION_STR "${DESKA_VERSION}")
SET(DESKA_EXECUTABLE deska-cli)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/src/deska/cli)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/cli-config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/src/deska/cli/cli-config.h)

add_library(LowLevelPyDeska SHARED ${libLowLevelPyDeska_SRCS})
target_link_libraries(LowLevelPyDeska DeskaDb ${DESKA_BOOST_LIBS} ${Boost_PYTHON_LIBRARY} ${PYTHON_LIBRARIES})

install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/db/Api.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/db/AdditionalValueStreamOperators.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/db/Connection.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/db/Filter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/db/JsonException.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/db/MacAddress.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/db/Objects.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/db/ObjectModification.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/db/Revisions.h
    DESTINATION include/deska/db
)
install(TARGETS DeskaDb DeskaCli DESTINATION lib${LIB_SUFFIX})
install(TARGETS deska-cli DESTINATION bin)
install(DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/server/app/
    DESTINATION bin
    USE_SOURCE_PERMISSIONS
    PATTERN "/deska-server"
    )
install(DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/server/app/deska_server_utils
    DESTINATION ${PYTHON_SITE_PACKAGES})

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/python/deska
    DESTINATION ${PYTHON_SITE_PACKAGES})
install(TARGETS LowLevelPyDeska
    DESTINATION ${PYTHON_SITE_PACKAGES_ARCH}/deska)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/install
    DESTINATION share/deska/install-scripts
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/server/db
    DESTINATION share/deska/install-scripts/src/deska/server
    PATTERN "*/data_generator/*" EXCLUDE
)

include(CTest)
add_subdirectory(tests)

if(BUILD_QML2IMAGE)
    message(STATUS "Will build the qml2image tool")
    add_subdirectory(src/qml2image)
else()
    message(STATUS "Will skip the qml2image tool")
endif()

if(BUILD_DOCS)
    message(STATUS "Will build PDF documentation")
    add_subdirectory(doc/technical)
else()
    message(STATUS "Will not build PDF documentation")
endif(BUILD_DOCS)
