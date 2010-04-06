//
// $Id: scanner.cc,v 1.3 2003/06/21 20:22:28 cholm Exp $
//
//   scanner.cc
//   Copyright (C) 2002  Christian Holm Christensen <cholm@nbi.dk>
//
//   This library is free software; you can redistribute it and/or
//   modify it under the terms of the GNU Lesser General Public License 
//   as published by the Free Software Foundation; either version 2 of 
//   the License, or (at your option) any later version.  
//
//   This library is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//   Lesser General Public License for more details. 
//
//   You should have received a copy of the GNU Lesser General Public
//   License along with this library; if not, write to the Free
//   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
//   MA 02111-1307  USA  
//
//
/** @file   doc/scanner.cc
    @author Christian Holm
    @date   Fri Jan 03 05:00:58 2003
    @brief  Scanner documentation */
/** @page scanner_doc The Scanner Class

    @section scanner_setup Setting up the Scanner Class

    The usage pattern of scanner library is very similar to the usage
    pattern of the parser library (@ref parser_setup).  

    To make a C++ class for the parser, you should have a grammar file
    that looks like
    @code 
    %{ 
    ... // Normal declaration stuff goes here 
    #define YLMM_SCANNER_CLASS scanner
    #include <ylmm/lexmm.hh> 
    %}
    ...// The rest is as usual 
    @endcode 
    The header ylmm/@link lexmm.hh lexmm.hh @endlink redefines the
    macros @c YY_FATAL_ERROR(msg) and @c YY_INPUT(buf,res,max) to
    forward calls to the user defined class. 

    The file @link yaccmm.hh ylmm/yaccmm.hh @endlink
    defines the interface to the generated C function via a C++ class,
    where @c YLMM_SCANNER_CLASS is the name of the user scanner class. 
    The developer can either specify this as a specific instantation
    of ylmm::basic_scanner, or it can be a sub-class of specific
    instantation of ylmm::basic_scanner. 
 
    See also the example 
    @link simple_scanner.ll simple_scanner.ll @endlink
    for and example of usage.  

    @section scanner_file Example Lexical Definition 

    The @b Lex input file is basically not altered by the use of this
    package.  However, the static variable @c _scanner is defined as a
    pointer to the user scanner class, and can be used in the
    definition to process the read tokens.
    
    First we setup the usual stuff (see @ref scanner_setup).
    @dontinclude simple_scanner.ll 
    @skip %{ 
    @until %}

    Then, we define some short-hand regular expression we'll use later
    on 
    @until %% 

    And finally, we make the lexical rules.  Note, that the static 
    variable @c _scanner already has the right type.
    @until %% 

    @subsection scanner_class The Scanner Class 

    Now, we need to make our scanner class. 
    @dontinclude simple_scanner.hh
    @skip class 
    @until Process

    And finally, we define the member functions to process the various
    lexical tokens and pass the identifier on to the parser.  In the
    member functions @c integer and @c floating, we use the data
    member ylmm::basic_scanner::_err_stream to output errors.  The idea
    is that we should setup this stream to be the same as used
    elsewhere, so that all error messages can be handled the same. 
    @until }; 

    The data member @c _current is defined in the base class
    ylmm::basic_scanner . It's a pointer to a ylmm::basic_buffer.
    That class represents an input stream to the lexical scanner.  The
    member functions ylmm::basic_buffer::increment_column and
    ylmm::basic_buffer::increment_line are used to track the input
    position of scanner, so that the parser may use that information
    in error messages (see the @b Bison documentation).

*/ 
#error This file is not for compilation
//
// EOF
//
