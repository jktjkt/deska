#
# $Id: Makefile.am,v 1.11 2005/08/23 13:53:20 cholm Exp $
#
#   General C++ parser and lexer
#   Copyright (C) 2002	Christian Holm Christensen <cholm@nbi.dk>
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public License 
#   as published by the Free Software Foundation; either version 2 of 
#   the License, or (at your option) any later version.	 
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details. 
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free
#   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
#   MA 02111-1307  USA	
#
#
EXTRA_DIST		= header.html.in footer.html style.css doxyconfig.in \
			  mainpage.cc examples.cc changelog.cc news.cc \
			  favicon.png logo.png lgpl.cc 
DOXY_NAME		= $(shell cat $(CONFIG_HEADER) | \
			  sed -n 's/.*PACKAGE_NAME *\"\([^\"]*\)\"/\1/p')
DOXY_NUMBER		= $(shell cat $(CONFIG_HEADER) | \
			  sed -n 's/.*PACKAGE_VERSION *\"\([^\"]*\)\"/\1/p')
DOT_PATH		= $(dir $(DOT))
pkgdocdir		= ${datadir}/doc/${PACKAGE}
CLEANFILES		= $(PACKAGE).tags

if HAVE_DOXYGEN
pkgdoc_DATA		= $(PACKAGE).tags
HTML			= html/index.html doxyconfig.in
else 
HTML			=
pkgdoc_DATA		=
endif

all-local:$(HTML) $(pkgdoc_DATA)
install-data-local:	$(HTML) $(pkgdoc_DATA)
	$(mkinstalldirs) $(DESTDIR)$(pkgdocdir)/html
	@list='$(wildcard html/*)'; for p in $$list; do \
	  if test -f "$$p"; then d=; else d="$(srcdir)/"; fi; \
	  f="`echo $$p | sed -e 's|^.*/||'`"; \
	  echo " $(pkgdocDATA_INSTALL) $$d$$p $(DESTDIR)$(pkgdocdir)/html/$$f"; \
	  $(pkgdocDATA_INSTALL) $$d$$p $(DESTDIR)$(pkgdocdir)/html/$$f; \
	done

uninstall-local:
	@echo "rm -rf $(DESTDIR)$(pkgdocdir)/html"
	rm -rf $(DESTDIR)$(pkgdocdir)/html

clean-local:
	rm -rf *~ html latex man

$(PACKAGE).tags:html/index.html
	if test -f $@ ; then : ; else rm -f $< ; $(MAKE) $<; fi

html/index.html: doxyconfig header.html footer.html style.css $(EXTRA_DIST)
	$(DOXYGEN) $<  
	if test -f logo.png ; then cp logo.png html ; fi
	if test -f favicon.png ; then cp favicon.png html ; fi

tar-ball:$(HTML)
	if test -f html/index.html ; then mv html $(PACKAGE) ; \
	  tar -czvf ../$(PACKAGE)-$(VERSION)-doc.tar.gz $(PACKAGE) ; \
	  mv $(PACKAGE) html ; fi 
#
# EOF
#
