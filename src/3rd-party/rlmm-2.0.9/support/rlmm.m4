dnl -*- mode: Autoconf -*- 
dnl
dnl $Id: rlmm.m4,v 1.3 2005/08/12 14:11:26 cholm Exp $ 
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
AC_DEFUN([AC_READLINE],
[
  AC_ARG_ENABLE(termcap, 
    [AC_HELP_STRING([--enable-termcap],[Enable use of termcap library])])
  AC_ARG_ENABLE(curses, 
    [AC_HELP_STRING([--enable-curses],[Enable use of (n)curses library])])
  EXTRA_LIBS=
  if test "x$enable_termcap" = "xyes" ; then 
    AC_CHECK_LIB(termcap,tgetent,EXTRA_LIBS="-ltermcap $EXTRA_LIBS")
  fi 
  if test "x$enable_curses" = "xyes" ; then 
    AC_CHECK_LIB(curses,initscr,EXTRA_LIBS="-lcurses $EXTRA_LIBS", 
      AC_CHECK_LIB(ncurses,initscr,EXTRA_LIBS="-lncurses $EXTRA_LIBS"))
  fi 
  AC_SUBST(EXTRA_LIBS)      
  AC_CHECK_HEADERS(readline/readline.h readline/history.h,,rlmm_nohead=yes) 
  AC_CHECK_LIB(readline, readline,,rlmm_nolib=yes,$EXTRA_LIBS)
  AC_CHECK_LIB(history, using_history,,rlmm_nolib=yes,$EXTRA_LIBS)
  if test "x$rlmm_nohead" = "xyes" || test "x$rlmm_nolib" = "xyes" ; then
    AC_MSG_ERROR([readline not installed - bailing out
Your libreadline may need to link with libtermcap, libncurses, or libcurses.
Use either `--enable-termcap' or `--enable-curses' options to `configure' to
check for these libraries])
  fi
  _RLMM_HAVE_HIST_ENTRY_TIMESTAMP
])   

dnl ==================================================================
AC_DEFUN([_RLMM_HAVE_STRING_COPY],
[
  AC_REQUIRE([AC_PROG_CXX])
  AC_LANG_PUSH([C++])
  AC_MSG_CHECKING([whether std::string has member function copy])
  have_string_copy=yes
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <string>]],
				     [[char dest[128];
				      std::string orig("Hello, world");
				      orig.copy(dest,0,orig.size());]])],
	  	    [have_string_copy=yes],[have_string_copy=no])
  AC_MSG_RESULT($have_string_copy)
  AH_TEMPLATE([HAVE_STRING_COPY],
	      [Whether std::string has copy member function])
  if test "x$have_string_copy" = "xyes" ; then 
    AC_DEFINE(HAVE_STRING_COPY)
  fi
  AC_LANG_POP([C++])
])

dnl ==================================================================
AC_DEFUN([_RLMM_HAVE_HIST_ENTRY_TIMESTAMP],
[	
  dnl AC_REQUIRE([AC_READLINE])
  have_hist_entry_timestamp=yes
  AH_TEMPLATE([HAVE_HIST_ENTRY_TIMESTAMP],
	      [Whether HIST_ENTRY has timestamp member])
  AC_CHECK_MEMBER([HIST_ENTRY.timestamp],
		  [AC_DEFINE([HAVE_HIST_ENTRY_TIMESTAMP])],,
		  [#include <readline/history.h>])
])

dnl ==================================================================
dnl AM_PATH_RLMM([MINIMUM-VERSION 
dnl                   [,ACTION-IF_FOUND 
dnl                    [, ACTION-IF-NOT-FOUND]])
AC_DEFUN([AM_PATH_RLMM],
[
    AC_REQUIRE([AC_READLINE])
    AC_ARG_WITH(rlmm-prefix,
        [AC_HELP_STRING([--with-rlmm-prefix],
			[Prefix where Readline-- is installed])],
        rlmm_prefix=$withval, rlmm_prefix="")

    if test "x${RLMM_CONFIG+set}" != xset ; then 
        if test "x$rlmm_prefix" != "x" ; then 
	    RLMM_CONFIG=$rlmm_prefix/bin/rlmm-config
	fi
    fi   
    AC_PATH_PROG(RLMM_CONFIG, rlmm-config, no)
    rlmm_min_version=ifelse([$1], ,2.0,$1)
    
    AC_MSG_CHECKING(for Readline-- version >= $rlmm_min_version)

    rlmm_found=no    
    if test "x$RLMM_CONFIG" != "xno" ; then 
       RLMM_CPPFLAGS=`$RLMM_CONFIG --cppflags`
       
       rlmm_version=`$RLMM_CONFIG -V` 
       rlmm_vers=`echo $rlmm_version | \
         awk 'BEGIN { FS = " "; } \
	   { printf "%d", ($''1 * 1000 + $''2) * 1000 + $''3;}'`
       rlmm_regu=`echo $rlmm_min_version | \
         awk 'BEGIN { FS = " "; } \
	   { printf "%d", ($''1 * 1000 + $''2) * 1000 + $''3;}'`
       if test $rlmm_vers -ge $rlmm_regu ; then 
            rlmm_found=yes
       fi
    fi
    AC_MSG_RESULT($rlmm_found - is $rlmm_version) 
   
    if test "x$rlmm_found" = "xyes" ; then 
        ifelse([$2], , :, [$2])
    else 
        ifelse([$3], , :, [$3])
    fi
    AC_SUBST(RLMM_CPPFLAGS)
    _RLMM_HAVE_STRING_COPY
])


dnl
dnl EOF
dnl 
