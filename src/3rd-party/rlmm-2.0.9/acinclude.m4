dnl -*- mode: Autoconf -*- 
dnl
dnl $Id: acinclude.m4,v 1.3 2005/08/12 14:11:26 cholm Exp $ 
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

dnl ==================================================================
AC_DEFUN(AC_DEBUG, 
[
  AC_REQUIRE([AC_PROG_CC])
  AC_REQUIRE([AC_PROG_CXX])
  AC_MSG_CHECKING(whether to make debug objects)
  AC_ARG_ENABLE(debug, 
    [AC_HELP_STRING([--enable-debug],[Enable debugging symbols in objects])]) 
  if test "x$enable_debug" = "xno" ; then 
    CFLAGS=`echo $CFLAGS | sed 's,-g,,'`
    CXXFLAGS=`echo $CXXFLAGS | sed 's,-g,,'`
  else 
    case $CXXFLAGS in 
    *-g*) ;;
    *)    CXXFLAGS="$CXXFLAGS -g" ;;
    esac
    case $CFLAGS in 
    *-g*) ;;
    *)    CFLAGS="$CFLAGS -g" ;;
    esac
  fi
  AC_MSG_RESULT($enable_debug 'CFLAGS=$CFLAGS')
])

dnl ------------------------------------------------------------------
AC_DEFUN(AC_OPTIMIZATION, [
  AC_REQUIRE([AC_PROG_CC])
  AC_REQUIRE([AC_PROG_CXX])
        
  AC_ARG_ENABLE(optimization, 
    [AC_HELP_STRING([--enable-optimization],[Enable optimization of objects])])

  AC_MSG_CHECKING(for optimiztion level) 
    
  changequote(<<, >>)dnl
  if test "x$enable_optimization" = "xno" ; then 
    CFLAGS=`echo   $CFLAGS   | sed 's,-O\([0-9][0-9]*\|\),,'`
    CXXFLAGS=`echo $CXXFLAGS | sed 's,-O\([0-9][0-9]*\|\),,'`
  elif test "x$enable_optimization" = "xyes" ; then 
    case $CXXFLAGS in 
    *-O*) ;;
    *)    CXXFLAGS="$CXXFLAGS -O2" ;;
    esac
    case $CFLAGS in 
    *-O*) ;;
    *)    CFLAGS="$CXXFLAGS -O2" ;;
    esac
  else 
    CFLAGS=`echo   $CFLAGS   | sed "s,-O\([0-9][0-9]*\|\),-O$enable_optimization,"`
    CXXFLAGS=`echo $CXXFLAGS | sed "s,-O\([0-9][0-9]*\|\),-O$enable_optimization,"`
  fi
  changequote([, ])dnl
  AC_MSG_RESULT($enable_optimization 'CFLAGS=$CFLAGS')
])

dnl ------------------------------------------------------------------
dnl
dnl EOF
dnl 
