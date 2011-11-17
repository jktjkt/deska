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


include(../../UseLATEX.cmake)

file(GLOB texfiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
    "*.tex" "img-*.pdf"
)
foreach(fname ${texfiles})
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/${fname}
        ${CMAKE_CURRENT_BINARY_DIR}/${fname}
        COPYONLY
    )
endforeach()

set(PDFLATEX_COMPILER_FLAGS "-shell-escape" "${PDFLATEX_COMPILER_FLAGS}")
add_latex_document(deska.tex MANGLE_TARGET_NAMES DEFAULT_PDF)
