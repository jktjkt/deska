//
// $Id: simple_scanner.hh,v 1.10 2004/03/03 12:56:18 cholm Exp $ 
//  
//  scanner.hh
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
#ifndef YLMM_test_scanner
#define YLMM_test_scanner
/** @file   simple_scanner.hh
    @author Christian Holm
    @date   Tue Jan 28 01:46:17 2003
    @brief   */

#ifndef YLMM_basic_scanner
#include <ylmm/basic_scanner.hh>
#endif
#ifndef BISON_SIMPLE_PARSER_H
#include "simple_parser.h"
#endif
#ifndef __SSTREAM__
#include <sstream>
#endif

/** Example of a scanner class
    @ingroup simple
*/
class simple_scanner : public ylmm::basic_scanner<int>
{
public:
  /** Constructor 
      @param buf The buffer to read from. */
  simple_scanner(ylmm::basic_buffer* buf=0) 
    : ylmm::basic_scanner<int>(buf) 
  { 
    _current->auto_increment(true); 
  }
  /** Destructor */
  virtual ~simple_scanner() {}
  /** Send a message to the user
      @param text The text to write.
      @param len  The length of @a text */
  void output(const char* text, int len) {
    if (_messenger)
      _messenger->error("At %d,%d: unrecognised token '%s'\n",
		 _current->line(), _current->column(), text);
  }
  /** Send a message to the user
      @param t The character to write.*/
  void output(const char t) {
    if (_messenger)
      _messenger->error("At %d,%d: unrecognised token '%c'\n",
		 _current->line(), _current->column(), t);
  }
  /** Process a integer literal 
      @param str The integer literal as a string
      @param len The length of the string
      @return integer literal token ID number @c NUM */
  int integer(const char* str, int len) 
  { 
    std::stringstream s(str);
    s >> _token;
    return NUM;
  }
  /** Process a newline
      @param str The new line string
      @param len The length of @a str
      @return The newline token ID @c NEWLINE */
  int newline(const char* str, int len) { return NEWLINE; }
}; 

#endif
//____________________________________________________________________
//
// EOF
//
