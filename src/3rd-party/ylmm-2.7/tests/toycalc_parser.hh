//
// $Id: toycalc_parser.hh,v 1.11 2004/03/09 02:37:56 cholm Exp $ 
//  
//  yaccmm.hh
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
/** @file   toycalc_parser.hh
    @author Christian Holm
    @date   Sat Dec 07 20:15:40 2002
    @brief  Parser class declaration */
#ifndef toycalc_parser_hh
#define toycalc_parser_hh

#ifndef toycalc_expression_hh
#include "toycalc_expression.hh"
#endif
#ifndef toycalc_refcounted_hh
#include "toycalc_ref_counted.hh"
#endif
#ifndef YLMM_basic_parser 
#include <ylmm/basic_parser.hh>
#endif
#ifndef YLMM_basic_scanner
#include <ylmm/basic_scanner.hh>
#endif
#ifndef __IOSTREAM__
#include <iostream>
#endif

namespace toycalc 
{
  /** Parser class for toy calculator 
      @ingroup toycalc 
  */
  class parser : public ylmm::basic_parser<ref_counted<expression> > 
  {
  public:
    typedef ref_counted<expression> token_type;
    typedef ylmm::basic_scanner<ref_counted<expression> > scanner_type;
  private:
    /** reference to scanner */    
    scanner_type& _scanner;
  public:
    /** Constuctor
	@param s reference to scanner  */
    parser(scanner_type& s) : _scanner(s) { prompt(); }
    virtual ~parser() {}
    /** Scan input
	@return The next token ID */
    int scan(void*) 
    { 
      if (need_where()) 
	return _scanner.next(*_token,*_location); 
      return _scanner.next(*_token);
    }
    /** show the prompt 
	@param nl Whether to preceed the prompt with a new line */
    void prompt(bool nl=false) 
    { 
      if (_scanner.current_buffer()->interactive())
        _messenger->message("%sprompt> ", (nl ? "\n" : "")); 
    }
    /** On errors, advance one line on error stream and show the
	prompt.  
	@param msg The message to print */
    void fatal(const char* msg) 
    { 
      ylmm::basic_parser<token_type>::fatal(msg); 
      prompt(true); 
    }
    /*@{*/
    /** @name Expression processing */
    /** Process a simple number 
	@param num A number 
	@return @a num */
    token_type number(const token_type& num) 
    { 
      return const_cast<token_type&>(num); 
    }
    /** Process an addition 
	@param lhs Left hand side 
	@param rhs Right hand side 
	@return A new toycalc::add object */
    token_type addition(const token_type& lhs, const token_type& rhs) 
    { 
      return new add(lhs, rhs); 
    }
    /** Process a subtraction 
	@param lhs Left hand side 
	@param rhs Right hand side 
	@return A new toycalc::subtract object */
    token_type subtraction(const token_type& lhs, const token_type& rhs) 
    { 
      return new subtract(lhs, rhs); 
    }
    /** Process a multiplication
	@param lhs Left hand side 
	@param rhs Right hand side 
	@return A new toycalc::multiply object */
    token_type multiplication(const token_type& lhs, const token_type& rhs) 
    { 
      return token_type(new multiply(lhs, rhs)); 
    }
    /** Process a division 
	@param lhs Left hand side 
	@param rhs Right hand side 
	@return A new toycalc::divide object */
    token_type division(const token_type& lhs, const token_type& rhs) 
    { 
      return token_type(new divide(lhs, rhs)); 
    }
    /** Process a minus 
	@param oper The operand expression
	@return a new toycalc::minus object */
    token_type negation(const token_type& oper) 
    { 
      return token_type(new minus(oper)); 
    }
    /** Process a precedence (pair of parentheis) 
	@param expr Expression to take precedence
	@return a new toycalc::precedence object */
    token_type precedence(const token_type& expr) 
    {
      return token_type(new toycalc::precedence(expr)); 
    }
    /** Process an expression 
	Evaluates the expression, and if an output messenger is
	defined, show the expression and the result on the messengers
	output stream 
	@param expr The expression to evaluate 
	@return @a expr */
    token_type result(const token_type& expr=0) { 
      if (expr) {
	double val = expr->evaluate(); 
	if (_messenger) 
	  _messenger->message_stream() << *expr << " => " 
					    << val << std::endl;
      }
      prompt();
      return const_cast<token_type&>(expr); 
    }
    /*@}*/
  };
}

#endif
//____________________________________________________________________
//
// EOF
//
