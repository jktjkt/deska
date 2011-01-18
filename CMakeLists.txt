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
#SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weffc++")
#SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -Wnon-virtual-dtor -Wold-style-cast -Woverloaded-virtual -fdiagnostics-show-option" )

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set(libDeskaDb_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/db/FakeApi.cpp
)

set(libDeskaCliParser_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/Parser.cpp
)

set(deskaCliParserDemo_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/deska/cli/main.cpp
)

add_library(DeskaDb ${libDeskaDb_SRCS})
add_library(DeskaCliParser ${libDeskaCliParser_SRCS})

add_executable(deska-cli-parser-demo ${deskaCliParserDemo_SRCS})
target_link_libraries(deska-cli-parser-demo DeskaCliParser)

enable_testing()
add_subdirectory(tests)
