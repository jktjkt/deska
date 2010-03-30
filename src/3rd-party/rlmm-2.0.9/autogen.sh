#!/usr/bin/make -f
# -*- mode: makefile -*- 
# $Id: autogen.sh,v 1.8 2005/08/12 14:11:26 cholm Exp $
#
#   General C++ parser and lexer
#   Copyright (C) 2002  Christian Holm Christensen <cholm@nbi.dk>
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
PREFIX		= $(HOME)/tmp
CONFFLAGS	= --prefix=$(PREFIX)
PACKAGE		:= $(strip $(shell grep AC_INIT configure.ac |\
			sed 's/.*([^,]*,\([^,]*\),[^,]*,\(.*\)).*/\2/'))
VERSION		:= $(strip $(shell grep AC_INIT configure.ac |\
			sed 's/.*([^,]*,\([^,]*\),[^,]*,\(.*\)).*/\1/'))

all:	setup
	./configure $(CONFFLAGS)

debug:	setup
	./configure --disable-optimization $(CONFFLAGS)

setup: 	ChangeLog configure

dists:
	$(MAKE) dist
	$(MAKE) tar-ball -C doc 

make:	all
	$(MAKE) -f Makefile 

aclocal.m4: configure.ac acinclude.m4 support/rlmm.m4
	aclocal -I . -I support

config/config.hh.in: aclocal.m4 
	autoheader 

Makefile.in: config/config.hh.in 
	automake -a -c 

configure: configure.ac Makefile.in 
	autoconf

ChangeLog:
	rm -f $@
	touch $@
	rcs2log > $@

show:
	@echo "$(PACKAGE) version $(VERSION)"
clean: 
	find . -name Makefile 		| xargs rm -f 
	find . -name Makefile.in 	| xargs rm -f 
	find . -name "*~" 		| xargs rm -f 
	find . -name core 		| xargs rm -f 
	find . -name .libs 		| xargs rm -rf 
	find . -name .deps 		| xargs rm -rf 
	find . -name "*.lo" 		| xargs rm -f 
	find . -name "*.o" 		| xargs rm -f 
	find . -name "*.la" 		| xargs rm -rf
	find . -name "*.log" 		| xargs rm -rf

	rm -f 	config/missing       	\
	  	config/mkinstalldirs 	\
	  	config/ltmain.sh     	\
	  	config/config.guess 	\
		config/config.sub 	\
		config/install-sh 	\
		config/ltconfig 	\
		config/config.hh 	\
		config/config.hh.in 	\
		config/stamp-h		\
		config/stamp-h.in	\
		config/stamp-h1		\
		config/depcomp

	rm -rf 	aclocal.m4 		\
		autom4te.cache		\
		config.cache 		\
		config.status  		\
		config.log 		\
		configure 		\
		INSTALL 		\
		ChangeLog		\
		configure-stamp		\
		build-stamp		\
		libtool

	rm -rf  debian/*.debhelper	\
		debian/*substvars	\
		debian/librlmm-doc	\
		debian/librlmm-dev	\
		debian/librlmm1		\
		debian/tmp		\
		debian/files

	rm -rf  doc/html 	\
		doc/latex 	\
		doc/man 	\
		doc/doxyconfig 	\
		doc/$(PACKAGE).tags

	rm -rf  test/basic	\
		test/file	\
		test/fileman	\
		test/rlmmtest	\
		test/simple	\
		test/streamer	

	rm -rf  $(PACKAGE)-*.tar.gz 

#
# EOF
#
