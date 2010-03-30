dnl
dnl $Id: configure.ac,v 1.25 2005/08/23 13:53:20 cholm Exp $ 
dnl  
dnl  Copyright (C) 2002 Christian Holm Christensen <cholm@nbi.dk> 
dnl
dnl  This library is free software; you can redistribute it and/or 
dnl  modify it under the terms of the GNU Lesser General Public License 
dnl  as published by the Free Software Foundation; either version 2.1 
dnl  of the License, or (at your option) any later version. 
dnl
dnl  This library is distributed in the hope that it will be useful, 
dnl  but WITHOUT ANY WARRANTY; without even the implied warranty of 
dnl  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
dnl  Lesser General Public License for more details. 
dnl 
dnl  You should have received a copy of the GNU Lesser General Public 
dnl  License along with this library; if not, write to the Free 
dnl  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 
dnl  02111-1307 USA 
dnl

AC_INIT([Readline--], [2.0.9], [cholm@nbi.dk], [rlmm])
AC_PREREQ(2.53)
AC_COPYRIGHT(GNU Lesser General Public License)
AC_REVISION($Revision: 1.25 $)
AC_CONFIG_AUX_DIR(config)
AM_INIT_AUTOMAKE([$PACKAGE_TARNAME], [$PACKAGE_VERSION])
AM_CONFIG_HEADER(config/config.hh)

dnl
dnl Compilers
dnl
AC_PROG_CC
AC_PROG_CXX

dnl 
dnl We want GCC to stop at errors 
dnl
if test "$ac_cv_cxx_compiler_gnu" = "xyes" ; then 
   CXXFLAGS="$CXXFLAGS -Wall -Werror"
fi

dnl
dnl Debug and optimisation
dnl
AC_DEBUG
AC_OPTIMIZATION

dnl
dnl Functions, libraries, headers 
dnl 
AC_CHECK_FUNCS([isxdigit])
AC_CHECK_HEADERS([unistd.h])
AC_READLINE
_RLMM_HAVE_STRING_COPY
 
dnl 
dnl Documentation
dnl
AC_PATH_PROG(PERL, perl)
AC_PATH_PROG(DOXYGEN, doxygen)
AM_CONDITIONAL(HAVE_DOXYGEN, test ! "x$DOXYGEN" = "x")

HAVE_DOT=NO
DOT_PATH=
AC_PATH_PROG(DOT, dot)
if ! test "x$DOT" = "x" ; then 
   HAVE_DOT=YES
   DOT_PATH=`dirname $DOT` 
fi
AC_SUBST([HAVE_DOT])
AC_SUBST([DOT_PATH])

dnl
dnl Output 
dnl
AC_CONFIG_FILES([Makefile 
		 rlmm.spec
                 rlmm/Makefile
		 support/rlmm-config
		 support/Makefile
		 doc/header.html
		 doc/doxyconfig
		 doc/Makefile
                 tests/Makefile])
                 
AC_OUTPUT

dnl
dnl EOF
dnl 
