dnl
dnl $Id: configure.ac,v 1.23 2005/03/09 17:19:43 cholm Exp $ 
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
dnl ------------------------------------------------------------------
AC_DEFUN([AC_DEBUG], 
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
AC_DEFUN([AC_YACC_DEBUG],
[
  AC_REQUIRE([AC_PROG_YACC])
  AC_MSG_CHECKING([whether yacc should give debug info])
  AC_ARG_ENABLE(yacc-debug,   
    [AC_HELP_STRING([--enable-yacc-debug],
		    [Enable Yacc debugging information])])
  if test ! "x$enable_yacc_debug" = "xno" ; then 
    YFLAGS="$YFLAGS -t"
  else 
    case $YFLAGS in 
    *-d*) ;;
    *)    YFLAGS="$YFLAGS -d"
    esac
  fi
  AC_MSG_RESULT($enable_yacc_debug 'YFLAGS=$YFLAGS')
])

dnl ------------------------------------------------------------------
AC_DEFUN([AC_LEX_DEBUG],
[
  dnl AC_REQUIRE([AM_PROG_LEX])
  AC_MSG_CHECKING([whether lex should give debug info])
  AC_ARG_ENABLE(lex-debug,   
    [AC_HELP_STRING([--enable-lex-debug],[Enable Lex debugging information])]) 
  if test ! "x$enable_lex_debug" = "xno" ; then 
    LFLAGS="$LFLAGS -d"
  else 
    case $LFLAGS in 
    *-d*) ;;
    *)    LFLAGS="$LFLAGS -d"
    esac
  fi
  AC_MSG_RESULT($enable_lex_debug 'LFLAGS=$LFLAGS')
])

dnl ------------------------------------------------------------------
AC_DEFUN([AC_OPTIMIZATION], 
[
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
AC_DEFUN([CHOLM_COMPILING],
[
  if test "x$USE_MAINTAINER_MODE" = "xyes" &&
     test "x$ac_cv_cxx_compiler_gnu" = "xyes" ; then
    AC_MSG_CHECKING(whether Christian is compiling)
    AC_ARG_ENABLE([effcxx-warnings],
		  [AC_HELP_STRING([--enable-effcxx-warnings],
                                  [Enable Warnings from Scott Meyer's book])],
		  [if test "x$enableval" = "xyes" ; then 
		     CXXFLAGS="$CXXFLAGS  -Weffc++"
		   fi])
    AC_ARG_ENABLE([Werror],
                  [AC_HELP_STRING([--enable-Werror],
                                  [Make warnings into errors])], 
	 	  [if test "x$enableval" = "xyes" ; then
                     CXXFLAGS="$CXXFLAGS  -Werror"
                   fi])
    dnl
    dnl Rational:
    dnl
    dnl  -fno-elide-constructors:
    dnl     Prevents Return Value Optimisation, which is an optional
    dnl     feature of the C++ standard
    dnl
    dnl -Werror:
    dnl     Do not allow warnings. (Would like this one, but std
    dnl     headers don't comply with -Weffc++ so this would make it
    dnl     impossible to compile the tests!)
    dnl
    dnl -W:
    dnl     Give some more warnings
    dnl
    dnl -ansi -pedantic:
    dnl     Be strictly ANSI (ISO/IEC) compliant
    dnl
    dnl -Wnon-virtual-dtor:
    dnl     Be sure not to make unheritable classes
    dnl
    dnl -Weffc++:
    dnl     Warnings about style from Scott Meyer's book' - make nicer
    dnl     code.
    dnl -Wold-style-cast:
    dnl     Warn about ugly C-style casts.  Better code
    dnl
    dnl -Woverloaded-virtual:
    dnl     We want to overload - not hide, so the signatures should
    dnl     match.
    CXXFLAGS="$CXXFLAGS -fno-elide-constructors -ansi -pedantic -Wall -W \
              -Wnon-virtual-dtor -Wold-style-cast \
              -Woverloaded-virtual"
    AC_MSG_RESULT(CXXFLAGS=$CXXFLAGS)
  fi
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

dnl ==================================================================
dnl
dnl Autoconf initialisation
dnl
AC_INIT([Yacc/Lex--], [2.7], [cholm@nbi.dk], [ylmm])
AC_PREREQ(2.53)
AC_COPYRIGHT(GNU Lesser General Public License)
AC_REVISION($Revision: 1.23 $)
AC_CONFIG_AUX_DIR(config)

dnl
dnl Automake initialisation 
dnl
AM_INIT_AUTOMAKE([$PACKAGE_TARNAME], [$PACKAGE_VERSION])
AM_CONFIG_HEADER(config/config.hh)

dnl
dnl For some extra stuff in maitainer-mode
dnl
dnl AM_MAINTAINER_MODE

dnl
dnl Compilers and other programs
dnl
AC_PROG_CC
AC_PROG_CXX
AC_PROG_YACC
AM_PROG_LEX
AC_CHECK_CXXCPP_VARIADIC

dnl
dnl Functions and headers 
dnl
AC_CHECK_FUNCS([vsnprintf])
AH_TEMPLATE([YY_NO_UNISTD_H],[Define this if we don't have unistd.h])
AC_CHECK_HEADER([unistd.h],,[AC_DEFINE([YY_NO_UNISTD_H])])

dnl
dnl Debug and optimisation
dnl
AC_DEBUG
AC_OPTIMIZATION

dnl
dnl Extra options for GCC
dnl
CHOLM_COMPILING

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
dnl For test suite 
dnl
AC_CONFIG_TESTDIR([tests])
AC_CONFIG_FILES([tests/Makefile tests/package.m4])

dnl
dnl Files to output
dnl
AC_CONFIG_FILES([Makefile 
                 ylmm/Makefile
		 doc/Makefile
		 doc/doxyconfig
		 support/ylmm-config
		 support/Makefile
		 debian/Makefile
		 ide/Makefile])

dnl
dnl Do it!
dnl                 
AC_OUTPUT

dnl
dnl EOF
dnl 
