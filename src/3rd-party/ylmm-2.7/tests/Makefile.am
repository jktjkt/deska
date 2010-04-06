#
# $Id: Makefile.am,v 1.8 2003/06/21 20:22:52 cholm Exp $ 
#  
#  Copyright (C) 2002 Christian Holm Christensen <cholm@nbi.dk> 
#
#  This library is free software; you can redistribute it and/or 
#  modify it under the terms of the GNU Lesser General Public License 
#  as published by the Free Software Foundation; either version 2.1 
#  of the License, or (at your option) any later version. 
#
#  This library is distributed in the hope that it will be useful, 
#  but WITHOUT ANY WARRANTY; without even the implied warranty of 
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU 
#  Lesser General Public License for more details. 
# 
#  You should have received a copy of the GNU Lesser General Public 
#  License along with this library; if not, write to the Free 
#  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 
#  02111-1307 USA 
#
AM_CPPFLAGS	= -I$(top_srcdir) 
AM_YFLAGS	= -d -v
AM_LFLAGS	=
EXTRA_DIST      = testsuite.at testsuite $(srcdir)/package.m4
TESTSUITE       = $(srcdir)/testsuite
AUTOM4TE        = autom4te
AUTOTEST        = $(AUTOM4TE) --language=autotest

DISTCLEANFILES  = atconfig atlocal $(TESTSUITE) 
CLEANFILES	= *~ *_scanner.cc *_parser.cc *_parser.h testsuite.log \
		  *.output
noinst_HEADERS	= toycalc_parser.hh 		\
		  toycalc_scanner.hh 		\
		  toycalc_expression.hh 	\
		  toycalc_ref_counted.hh 	\
		  simple_parser.hh 		\
		  simple_scanner.hh 		\
		  util.hh

noinst_PROGRAMS	= toycalc simple 

toycalc_SOURCES	= toycalc_parser.yy 	\
		  toycalc_scanner.ll	\
		  toycalc.cc
simple_SOURCES	= simple_parser.yy 	\
		  simple_scanner.ll 	\
		  simple.cc

check-local: atconfig $(TESTSUITE) # atlocal 
	$(SHELL) $(TESTSUITE) AUTOTEST_PATH=tests:src

clean-local:
	rm -rf testsuite.dir

$(TESTSUITE):	$(srcdir)/testsuite.at $(srcdir)/simple.at \
		$(srcdir)/toycalc.at $(srcdir)/package.m4 
	$(AUTOTEST) -I $(srcdir) $@.at -o $@.tmp
	mv $@.tmp $@


#
# EOF
#
