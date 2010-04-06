dnl -*- mode: Autoconf -*- 
dnl
dnl $Id: ylmm.m4,v 1.5 2004/10/30 10:58:28 cholm Exp $ 
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

dnl AC_PATH_YLMM([MINIMUM-VERSION 
dnl                   [,ACTION-IF_FOUND 
dnl                    [, ACTION-IF-NOT-FOUND]])
AC_DEFUN([AC_PATH_YLMM],
[
    AC_REQUIRE([AC_PROG_YACC])
    AC_REQUIRE([AM_PROG_LEX])
    
    AC_ARG_WITH(ylmm-prefix,
      [AC_HELP_STRING([--with-ylmm-prefix],
		      [Prefix where Yacc/Lex-- is installed])])

    if test "x${YLMM_CONFIG+set}" != xset ; then 
        if test "x$with_ylmm_prefix" != "x" ; then 
	    YLMM_CONFIG=$with_ylmm_prefix/bin/ylmm-config
	fi
    fi   

    case $YACC in 
    bison*)  ;;
    *)       AC_MSG_WARN(Yacc/Lex-- need bison to work properly) ;;
    esac 

    case $LEX in 
    flex*) ;;
    *)     AC_MSG_WARN(Yacc/Lex-- need flexc to work properly) ;;
    esac 

    AC_PATH_PROG(YLMM_CONFIG, ylmm-config, no)
    ylmm_min_version=ifelse([$1], ,0.1,$1)
    
    AC_MSG_CHECKING(for Yacc/Lex-- version >= $ylmm_min_version)

    ylmm_found=no    
    if test "x$YLMM_CONFIG" != "xno" ; then 
       YLMM_CPPFLAGS=`$YLMM_CONFIG --cppflags`
       
       ylmm_version=`$YLMM_CONFIG -V` 
       ylmm_vers=`echo $ylmm_version | \
         awk 'BEGIN { FS = " "; } \
	   { printf "%d", ($''1 * 1000 + $''2) * 1000 + $''3;}'`
       ylmm_regu=`echo $ylmm_min_version | \
         awk 'BEGIN { FS = " "; } \
	   { printf "%d", ($''1 * 1000 + $''2) * 1000 + $''3;}'`
       if test $ylmm_vers -ge $ylmm_regu ; then 
            ylmm_found=yes
       fi
    fi
    AC_MSG_RESULT($ylmm_found - is $ylmm_version) 
   
    if test "x$ylmm_found" = "xyes" ; then 
        ifelse([$2], , :, [$2])
    else 
        ifelse([$3], , :, [$3])
    fi
    AC_SUBST(YLMM_CPPFLAGS)
])

dnl ------------------------------------------------------------------
AC_DEFUN([AC_CHECK_CXXCPP_VARIADIC], 
[
  AC_REQUIRE([AC_PROG_CXXCPP])
  AH_TEMPLATE([YLMM_CXXCPP_STD_VARIADIC], [Preprocessor understands ...])
  AH_TEMPLATE([YLMM_CXXCPP_GNU_VARIADIC], [Preprocessor understands a...])
  AC_LANG_PUSH([C++])
  AC_CACHE_CHECK([whether $CXXCPP understands variadic arguments],	 
		 [ylmm_cv_cxxcpp_variadic],[
    ylmm_cv_cxxcpp_variadic="no"
    AC_PREPROC_IFELSE(
      [AC_LANG_PROGRAM([[#define HELLO(...) printf(__VA_ARGS__)]],
	 	       [[HELLO("The answer: %d\n", 42);]])],
      [ylmm_cv_cxxcpp_variadic=std],
      [AC_PREPROC_IFELSE(
        [AC_LANG_PROGRAM([[#define HELLO(a...) printf(a)]],
	  	         [[HELLO("The answer: %d\n", 42);]])],
	  [ylmm_cv_cxxcpp_variadic=gnu])],
	  [ylmm_cv_cxxcpp_variadic=no])
  ])
  if test "x$ylmm_cv_cxxcpp_variadic" = "xstd" ; then 
     AC_DEFINE([YLMM_CXXCPP_STD_VARIADIC])
  elif test "x$ylmm_cv_cxxcpp_variadic" = "xgnu" ; then 
     AC_DEFINE([YLMM_CXXCPP_GNU_VARIADIC])
  fi
  AC_LANG_POP([C++])
])

dnl
dnl EOF
dnl 
