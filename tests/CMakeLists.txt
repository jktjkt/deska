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
    target_link_libraries(test_${fname} DeskaCliParser DeskaDb boost_unit_test_framework)
    add_test(test_${fname} test_${fname})
endmacro(deska_test)

deska_test(parser-top-level-objects)
