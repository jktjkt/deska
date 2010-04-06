//
// $Id: build.cc,v 1.2 2003/06/29 11:29:20 cholm Exp $
//
//    build.cc 
//    Copyright (C) 2002  Christian Holm <cholm@linux.HAL3000> 
//   
//    This library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Lesser General Public
//    License as published by the Free Software Foundation; either
//    version 2.1 of the License, or (at your option) any later
//    version.
//   
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Lesser General Public License for more details.
//   
//    You should have received a copy of the GNU Lesser General Public
//    License along with this library; if not, write to the Free
//    Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
//    MA 02111-1307 USA
//
/** @file   build.cc
    @author Christian Holm
    @date   Sun Jun 29 13:27:00 2003
    @brief  Information for client projects
*/
/** @page build Using Yacc/Lex-- in your project 

    @section build1 How to include Yacc/Lex-- in your project

    @b Yacc/Lex-- consist entirely of declaration (or header) files.
    All you need to do, is to include those file in your project
    code.  You can copy the header files into your source code, or you
    can use them from their installed location - it's entirely up to
    you, though you should read and understand the terms of the
    @link lgpl licence@endlink 

    You need to have @b Yacc and/or @b Lex compatible programs
    installed on your system to use @b Yacc/Lex--.  In fact, if you
    don't there's very litte reason why you would want 
    @b Yacc/Lex-- in the first place. 

    The included file <tt>ylmm.m4</tt> defines the @b Autoconf macro 
    <tt>AC_YLMM_PATH</tt> that you can use in your
    <tt>configure.ac</tt> file.  It defines the substitution variable
    <tt>YLMM_CPPFLAGS</tt> to contain the path to the @b Yacc/Lex--
    header files.  Put that macro in your configuration file 
    @verbatim
    AC_YLMM_PATH
    @endverbatim     
    and in your <tt>Makefile.am</tt>s, do
    @verbatim 
    AM_CPPFLAGS		= $(YLMM_CPPFLAGS) 
    @endverbatim 
    The @b Autoconf macro @c AM_YLMM_PATH will automatically test for
    the presence of @b Yacc and @b Lex compatible programs on your
    system.

    You can optionally pass three arguments to <tt>AC_YLMM_PATH</tt>,
    where the first is the minimum version of @b Yacc/Lex-- you need,
    the second is shell-script code executed if @b Yacc/Lex-- is found
    on the target system, and the third is shell-script code executed
    if @b Yacc/Lex-- isn't found on the target system.  The full
    synopsis is therefor
    @verbatim
    AC_PATH_YLMM([MINIMUM-VERSION [,ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
    @endverbatim

    There's also a macro to check whether the C++ preprocessor
    understand variadic arguments.  You may need to use that if you
    get errors from the preprocessor that it doesn't understand the
    variadic arguments.  Just put 
    @verbatim
    AC_CHECK_CXXCPP_VARIADIC
    @endverbatim 
    in your <tt>configure.ac</tt> file. 

    Alternatively, you can use the included script
    <tt>ylmm-config</tt> to obtain the information. 
*/
#error This file is not for compilation
// 
// EOF
//
