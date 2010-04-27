//
// $Id: simple_parser.hh,v 1.10 2004/03/03 12:56:18 cholm Exp $ 
//  
//  simple_parser.hh
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
#ifndef simple_parser_hh
#define simple_parser_hh
/** @file   simple_parser.hh
    @author Christian Holm
    @date   Tue Jan 28 01:46:29 2003
    @brief   */

#ifndef YLMM_basic_parser 
#include <ylmm/basic_parser.hh>
#endif
#ifndef YLMM_basic_scanner
#include <ylmm/basic_scanner.hh>
#endif
#ifndef YLMM_basic_location
#include <ylmm/basic_location.hh>
#endif
#ifndef __IOSTREAM__
#include <iostream>
#endif

/** Parse numbers, and echo them back to user
    @ingroup simple
*/
class simple_parser : public ylmm::basic_parser<int> 
{
private:
  ylmm::basic_scanner<int>& _scanner; /** Reference to scanner */
public:
  /** Constructor
      @param s Reference to scanner */
  simple_parser(ylmm::basic_scanner<int>& s) : _scanner(s) 
  {}
  /** Destructor  */
  virtual ~simple_parser() {}
  /** Scan the input via forwarded call to the scanner 
      @param arg Optional argument. 
      @return the scanner token ID */
  int scan(void* arg=0) 
  { 
    return _scanner.next(token()); 
  }
  /** On errors, advance one line on error stream and show the prompt.  
      @param m The message to print */
  void fatal(const char* m) 
  { 
    ylmm::basic_parser<int>::fatal(m); 
    if (_messenger) _messenger->error_stream() << std::endl; 
  }
  /** Process an expression 
      @param val The value of the message 
      @return @a val */
  int result(int val=0) 
  { 
    message("\t'%d'\n",  val); return val; 
  }
};

#endif
//____________________________________________________________________
//
// EOF
//
