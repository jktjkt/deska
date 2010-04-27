//
// $Id: examples.cc,v 1.6 2003/06/29 11:29:20 cholm Exp $
//
//   examples.cc
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
/** @file   doc/examples.cc
    @author Christian Holm
    @date   Sun Sep 15 16:29:24 2002
    @brief  Examples. */


/** @page examples Examples
 */

/** @example simple.cc
    @par Simple example 
    This example parses numbers input by the user, and echos them
    right back.  Everything else is ignored. 
*/
/** @example simple_parser.hh
    @par Simple example - parser declaration
    The declaration of the parser class.  It holds a reference to the
    scanner used.  When a new token is needed, it forwards the call to
    the referenced scanner.  The member function @c result takes care
    of echoing the number back to the user.  If something else is
    given by the user, the member function @c fatal is invoked. 
*/
/** @example simple_scanner.hh
    @par Simple example - scanner declaration
    The scanner reads numbers from input as a string, and turns them
    into integer values.  Newlines in the input are signalled back to
    the parser.  Everything else flags an error, and the user is
    alerted. 
*/
/** @example simple_parser.yy
    @par Simple example - parser specification
    This contains the grammar parser for an integer grammar
*/
/** @example simple_scanner.ll
    @par Simple example - scanner specification 
    This contains the lexical scanner rules for an integer
    parser. Only integers and newlines are recognised. Everything else
    is ignored. 
*/

/** @example toycalc.cc
    @par Toycalc example. 
    This example is a simple command line calculator.  
*/
/** @example toycalc_parser.hh
    @par Toycalc example - parser declaration
    Constructs a parse tree of toycalc::expression objects.  Each of
    the handling member functions return a new expression, which is
    added to the parse tree.  The member function
    toycalc::parser::result, which is called at a newline, evaluates
    the parse tree recursively, and show the result on the standard
    output.  
*/
/** @example toycalc_scanner.hh
    @par Toycalc example - scanner declaration
    Reads in tokens from the input buffer, and returns the token ID
    for the particular type.  The handling member functions constructs
    new toycalc::expression objects, which is then picked up by the
    parser and put in the parse tree. 
*/
/** @example toycalc_ref_counted.hh
    @par Toycalc example - Reference counted smart pointer.  This is
    used by the various derived classed of @c toycalc::expression that
    contains references to other @c toycalc::expression objects. 
*/
/** @example toycalc_expression.hh
    @par Toycalc example - Expression (token type) declaration. 
    Definitions of the class representation of various kind of
    expressions.  Objects of these classes are created by the scanner
    and stored in the parse tree by the parser.  They can be evaluated
    and printed recursively. 
*/
/** @example toycalc_parser.yy
    @par Toycalc example - parser specification
    Recognises simple aritmatic expression followed by newlines.  An 
    expression can be a toycalc::number or arithmatic combination of
    numbers.  Valid arithmatic combinations are the binary operators
    '+', '-', '*', and '/', as well as the unary '-', and the predence
    operation of '(...)'.  Everything else is ignored. 
*/
/** @example toycalc_scanner.ll
    @par Toycalc example - scanner specification 
    Recognises integer and floating point literals, as well as the
    operators '+', '-', '*', and '/', and the precedence start '(' and
    end ')'.  A newline in the input triggers a message to the
    parser. Everything else is an error.  Floating point numbers may
    use exponentional notation, like '1e-34'. 
*/

/** @example util.hh
    @par Utilities used by the examples. 
    This header defines the functions @c usage, @c version, and 
    @c getargs, to help the examples do some common tasks related to
    parsing the command line. 
*/
#error This file is not for compilation
//
// EOF
//
