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
include(../../UseLATEX.cmake)

# There does not appear to be any reasonable way of ensuring that:
# a) the pdflatex does not prompt for input at all,
# b) the process stops on error immediately,
# c) no .pdf output is ever produced in case of an error
#
# The current situation (-interaction=nonstopmode) prevents interactive prompts,
# but still produces .pdf files and therefore masks the real issue.
# A reasonable workaround for debugging is to temporarily comment out inheriting
# the PDFLATEX_COMPILER_FLAGS below.
set(PDFLATEX_COMPILER_FLAGS "-shell-escape"
    "${PDFLATEX_COMPILER_FLAGS}"
)

set(deska_doc_tex
    acknowledgement.tex
    intro.tex
    users-intro.tex
    objects-and-relations.tex
    building.tex
    admin-dbscheme.tex
    cli-usage.tex
    dev-structure.tex
    dbapi-protocol.tex
    prog-cpp-deskadb.tex
    cli-app.tex
    python-bindings.tex
    deska-server.tex
    cfg-generators.tex
    existing-tools.tex
    fzu-setup.tex
    automated-tests.tex
    patches.tex
    conclusion.tex
)

set(deska_doc_images
    img-deska-components.pdf
    img-deska-workflow.pdf
    img-deska-server-components.pdf
)

add_latex_document(deska.tex
    INPUTS ${deska_doc_tex}
    IMAGES ${deska_doc_images}
    BIBFILES references.bib
    MANGLE_TARGET_NAMES
    DEFAULT_PDF)

if(SKIP_INSTALL_DOCS)
else()
    if(NOT DOCDIR)
        set(DOCDIR share/doc/${PROJECT_NAME})
    endif()
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/deska.pdf
        DESTINATION ${DOCDIR})
endif()
