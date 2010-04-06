//
// $Id: toycalc_scanner.hh,v 1.9 2004/03/09 02:37:56 cholm Exp $ 
//  
//  toycalc_scanner.hh
//  Copyright (C) 2002 Christian Holm Christensen <cholm@nbi.dk> 
//
//  This library is free software; you can redistribute it and/or 
//  modify it under the terms of the GNU Lesser General Public License 
//  as published by the Free Software Foundation; either version 2.1 
//  of the License, or (at your option) any later version. 
//
//  This library is distributed in the hope that it will be useful, 
//  but WITHOUT ANY WARRANTY; without even the implied warranty of 
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
//  Lesser General Public License for more details. 
// 
//  You should have received a copy of the GNU Lesser General Public 
//  License along with this library; if not, write to the Free 
//  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 
//  02111-1307 USA 
//
/** @file   toycalc_scanner.hh
    @author Christian Holm
    @date   Sat Dec 07 20:17:18 2002
    @brief  Scanner declaration */

#ifndef toycalc_scanner_hh
#define toycalc_scanner_hh

#ifndef YLMM_basic_scanner
#include <ylmm/basic_scanner.hh>
#endif
#ifndef toycalc_expression_hh
#include "toycalc_expression.hh"
#endif
#ifndef toycalc_refcounted_hh
#include "toycalc_ref_counted.hh"
#endif
#ifndef BISON_TOYCALC_PARSER_H
#include "toycalc_parser.h"
#endif
#ifndef __SSTREAM__
# include <sstream>
#endif
#ifndef __STRING__
# include <string>
#endif

namespace toycalc 
{
  /** Example of a scanner class
      @ingroup toycalc
  */
  class scanner : public ylmm::basic_scanner<ref_counted<expression> > 
  {
  public:
    typedef ref_counted<expression> token_type;
    typedef ylmm::basic_scanner<token_type> base_type;
    /** Constructor 
	@param buf The buffer to read from  */
    scanner(ylmm::basic_buffer* buf=0) 
      : base_type(buf) 
    {
      _current->auto_increment(true);
    }
    virtual ~scanner() {}
    /*@{*/
    /** @name Token processing */
    /** Process a integer literal 
	@param str integer literal as a string
	@param len length of @a str
	@return The number literal token ID @c NUM */
    int integer(const char* str, int len) 
    { 
      std::stringstream s(str);
      long n;
      s >> n;
      _token = new number(n); 
      return NUM;
    }
    /** Process a floating point literal 
	@param str floating point literal as a string
	@param len length of @a str
	@return The number literal token ID @c NUM */
    int floating(const char* str, int len) 
    {
      std::stringstream s(str);
      double n;
      s >> n;
      _token = new  number(n); 
      return NUM;
    }
    /** Process a left parenthesis 
        @param  str The left parenthesis  as a string
        @param  len The lenght of @a str
        @return The left parenthesis  token ID @c LPAREN */
    int lparen(const char* str, int len) { return LPAREN; }
    /** Process a right parenthesis 
        @param  str The right parenthesis  as a string
        @param  len The lenght of @a str
        @return The right parenthesis  token ID @c RPAREN */
    int rparen(const char* str, int len) { return RPAREN; }
    /** Process an addition sign
        @param  str The addition sign as a string
        @param  len The lenght of @a str
        @return The addition sign token ID @c PLUS */
    int plus(const char* str, int len) { return PLUS; }
    /** Process a minus or subtraction sign 
        @param  str The minus or subtraction sign as a string
        @param  len The lenght of @a str
        @return The minus or subtraction sign token ID @c MINUS */
    int minus(const char* str, int len) { return MINUS; }
    /** Process a multiplication sign
        @param  str The multiplication sign as a string
        @param  len The lenght of @a str
        @return The multiplication sign token ID @c STAR */
    int star(const char* str, int len) { return STAR; }
    /** Process a division sign
        @param  str The division sign as a string
        @param  len The lenght of @a str
        @return The division sign token ID @c SLASH */
    int slash(const char* str, int len) { return SLASH; }
    /** Process a newline
        @param  str The newline as a string
        @param  len The lenght of @a str
        @return The newline token ID @c NEWLINE */
    int newline(const char* str, int len) { return NEWLINE; }
    /*@}*/
    /** Do nothing */
    void echo(const char*,int) {}
    /** Do nothing */
    void echo(const char) {}
  }; 
}

#endif
//____________________________________________________________________
//
// EOF
//
