//
// $Id: toycalc_expression.hh,v 1.7 2004/03/09 02:37:56 cholm Exp $ 
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

/**  @file   toycalc_expression.hh
     @author Christian Holm
     @date   Mon Sep 30 00:08:22 2002
     @brief  Declaration of parser class. */

#ifndef toycalc_expression_hh
#define toycalc_expression_hh
#ifndef toycalc_refcounted_hh
#include "toycalc_ref_counted.hh"
#endif
#ifndef __IOSTREAM__
#include <iostream>
#endif

namespace toycalc 
{
  
  /** Base class for all expression 
      @ingroup toycalc
  */
  class expression
  { 
  protected: 
    mutable double _value;	/** The value of the expression */
  public: 
    /** Default construction */
    expression() : _value(0) {}
    /** Copy construction 
	@param e Other expresion to copy */
    expression(const expression& e) 
      : _value(e._value) 
    {}
    /** Destructor */
    virtual ~expression() {}
    /** Evaulate the expression to a value 
	@return the value of the expression */
    virtual double evaluate() const = 0;
    /** Show the expression on a stream 
	@param o stream to print to */
    virtual void show(std::ostream& o) const = 0;
  };

  /** Function to stream out an expression 
      @param o Stream to write to
      @param e The expression 
      @return @a o
      @ingroup toycalc
  */
  inline std::ostream& operator<<(std::ostream& o, const expression& e) 
  {
    e.show(o);
    return o;
  }

  /** Expression representing a plain number
      @ingroup toycalc
  */
  class number : public expression 
  {
  public:
    /** Value construction 
	@param value the value of the expression */
    number(double value) 
    { 
      _value = value; 
    } 
    /** Copy construction 
	@param n Other number to copy */
    number(const number& n) 
      : expression(n)
    { 
      _value =  n._value; 
    }
    /** Evaulate the expression to a value 
	@return the value of the expression */
    double evaluate() const 
    { 
      return _value; 
    }
    /** Write out the number stored
	@param o stream to write to */
    void show(std::ostream& o) const 
    { 
      o << _value; 
    }
  };
  
  /// 
  /** Abstract base class for binary expressions   
      @ingroup toycalc
  */  
  class binary : public expression 
  { 
  public: 
    typedef ref_counted<expression> sub_type;
  protected: 
    const sub_type _lhs;	/** Left hand side of the expression  */
    const sub_type _rhs;	/** Right hand side of the expression */
  public:
    /** Construction with left and right hand side 
	@param lhs Left hand side
	@param rhs Right hand side */
    binary(const sub_type& lhs, const sub_type& rhs) 
      : _lhs(lhs), _rhs(rhs) 
    {}
    /** Copy constructor, that copies the two expressions 
	@param b Binary to copy from */
    binary(const binary& b) 
      : expression(b), _lhs(b._lhs), _rhs(b._rhs) 
    {}
    /** Deallocation, deletees contained expressions  */
    virtual ~binary() 
    { 
      // delete _lhs, _rhs; 
    }
  };

  /// 
  /** Abstract base class for unary expressions   
      @ingroup toycalc
  */  
  class unary : public expression 
  { 
  public: 
    typedef ref_counted<expression> sub_type;
  protected: 
    const sub_type _oper;	/** The operand */
  public:
    /** Construction with an operand 
	@param oper operand */
    unary(const sub_type& oper) 
      : _oper(oper) 
    {}
    /** Copy construction, copies the operand 
	@param u unary expression to copy from  */
    unary(const unary& u) 
      : expression(u), _oper(u._oper) 
    {} 
    /** Deallocation, frees the operand  */
    virtual ~unary() 
    { 
      // delete _oper; 
    }
  };
  
  /** Addition of two expressions 
      @ingroup toycalc
  */
  class add : public binary 
  { 
  public: 
    /** Construction with left and right hand side 
	@param lhs left hand side
	@param rhs Right hand side */
    add(const sub_type& lhs, const sub_type& rhs) 
      : binary(lhs, rhs) 
    {}
    /** Evalaute the addition 
	@return @f$ lhs + rhs @f$ */
    double evaluate() const 
    { 
      return _value = _lhs->evaluate() + _rhs->evaluate(); 
    }
    /** Show the addition 
	@param o output stream */
    void show(std::ostream& o) const 
    { 
      o << *_lhs << " + " << *_rhs; 
    }
  }; 	

  /** Subtract to expression     
      @ingroup toycalc
  */
  class subtract : public binary 
  { 
  public: 
    /** Construction with left and right hand side 
	@param lhs left hand side
	@param rhs Right hand side */
    subtract(const sub_type& lhs, const sub_type& rhs) 
      : binary(lhs, rhs) 
    {}
    /** Evaluate the subtraction 
	@return @f$ lhs - rhs @f$ */
    double evaluate() const 
    { 
      return _value = _lhs->evaluate() - _rhs->evaluate(); 
    }
    /** Show the substraction 
	@param o output stream */
    void show(std::ostream& o) const 
    { 
      o << *_lhs << " - " << *_rhs; 
    }
  }; 	
    
  /** Multiply two expressions 
      @ingroup toycalc
  */
  class multiply : public binary 
  {
  public:
    /** Construction with left and right hand side 
	@param lhs left hand side
	@param rhs Right hand side */
    multiply(const sub_type& lhs, const sub_type& rhs) 
      : binary(lhs, rhs) 
    {}
    /** Evaluate the multiplication 
	@return @f$ lhs \times rhs @f$ */
    double evaluate() const 
    { 
      return _value = _lhs->evaluate() * _rhs->evaluate(); 
    }
    /** Show the mulitplication 
	@param o output stream */
    void show(std::ostream& o) const 
    { 
      o << *_lhs << " * " << *_rhs; 
    }
  }; 	

  /** Divide to expressions     
      @ingroup toycalc
  */
  class divide : public binary 
  {
  public:
    /** Construction with left and right hand side 
	@param lhs left hand side
	@param rhs Right hand side */
    divide(const sub_type& lhs, const sub_type& rhs) 
      : binary(lhs, rhs) 
    {}
    /** Evaluate the division 
	@return @f$ lhs / rhs @f$ */
    double evaluate() const 
    { 
      return _value = _lhs->evaluate() / _rhs->evaluate(); 
    }
    /** Show the division 
	@param o Output stream */
    void show(std::ostream& o) const 
    { 
      o << *_lhs << " / " << *_rhs; 
    }
  };

  /** Negation of an expression 
      @ingroup toycalc
  */
  class minus : public unary 
  {
  public:
    /** Construction with an operand 
	@param oper operand */
    minus(const sub_type& oper) 
      : unary(oper) 
    {}
    /** Evaluate to minus the operand 
	@return @f$ - operand @f$ */
    double evaluate() const 
    { 
      return _value = -_oper->evaluate(); 
    }
    /** Show the negation 
	@param o output stream */
    void show(std::ostream& o) const 
    { 
      o << "-" << *_oper; 
    }
  };

  /** Precedence of expressions 
      @ingroup toycalc
  */
  class precedence : public expression 
  {
  public: 
    typedef ref_counted<expression> sub_type;
  protected:
    const sub_type _inner;	/** The inner expression */
  public:
    /** Construction with an operand 
	@param oper operand */
    precedence(const sub_type& inner) 
      : _inner(inner) 
    {}
    /** Deallocation.  Free's nothing */
    virtual ~precedence() 
    {}
    /** Evaluete the contained expression. 
	@return @f$ inner @f$ */
    double evaluate() const 
    { 
      return _value = _inner->evaluate(); 
    }
    /** Show the precedence 
	@param o output stream */
    void show(std::ostream& o) const 
    { 
      o << "(" << *_inner << ")"; 
    }
  };
}


#endif
//____________________________________________________________________
//
// EOF
//
