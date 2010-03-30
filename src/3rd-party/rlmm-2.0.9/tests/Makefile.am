#
# $Id: Makefile.am,v 1.3 2005/07/31 10:43:54 cholm Exp $ 
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
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
#  Lesser General Public License for more details. 
# 
#  You should have received a copy of the GNU Lesser General Public 
#  License along with this library; if not, write to the Free 
#  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 
#  02111-1307 USA 
#

AM_CPPFLAGS		= -I$(top_srcdir)
CLEANFILES		= $(wildcard .*_history)
LDADD			= $(EXTRA_LIBS)

noinst_PROGRAMS		= simple basic rlmmtest fileman streamer callback

simple_SOURCES		= simple.cc
basic_SOURCES		= basic.cc
rlmmtest_SOURCES	= rlmmtest.cc
fileman_SOURCES		= fileman.cc
streamer_SOURCES	= streamer.cc
callback_SOURCES	= callback.cc

#
# EOF
#
