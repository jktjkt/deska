//____________________________________________________________________
//
// $Id: toycalc_ref_counted.hh,v 1.2 2004/10/30 10:52:49 cholm Exp $
//
#ifndef toycalc_ref_counted_hh
#define toycalc_ref_counted_hh
/** @file    ref_counted.hh
    @author  Christian Holm Christensen <cholm@@nbi.dk>
    @date    Thu Nov 20 16:21:28 2003
    @brief   Ref_Counted pattern classes
*/
#ifndef __FUNCTIONAL__
# include <functional>
#endif
#ifndef __ALGORITHM__
# include <algorithm>
#endif
#ifndef __CASSERT__
# include <cassert>
#endif
#ifndef _DEBUG
#define _DEBUG
#endif

namespace toycalc 
{
#if 0
  template <typename Type>
  struct by_reference 
  {
    typedef Type data_type;
    by_reference(data_type& v) : _value(v) {}
    operator data_type&() { return _value; }    
    operator const data_type&() const { return _value; }    
  private:
    by_reference& operator=(const by_reference& rhs);
    data_type& _value;
  };
#endif

  /** @defgroup Ref_Counted Reference counted smart pointer
      @brief Reference counted smart pointer
  */

  //__________________________________________________________________
  /** @class single_thread ref_counted.hh <tests/ref_counted.hh>
      @brief Default thread model for smart pointers.
      @ingroup Ref_Counted
  */
  struct single_thread
  {
    /** The integer type */
    typedef int integer_type;
    /** Automically increment an integer
	@param c The integer variable to increment. 
	@return  The value of @c c after increment. */
    static integer_type atomic_increment(volatile integer_type& c)
    {
      return c += 1;
    }
    /** Automically decrement an integer
	@param c The integer variable to decrement. 
	@return  The value of @c c after decrement. */
    static integer_type atomic_decrement(volatile integer_type& c)
    {
      return c -= 1;
    }
    /** Automically set an integer to a value
	@param c The integer variable to set
	@param val The value to set @c c to
	@return  The value of @c c after the assignment. */
    static integer_type atomic_set(volatile integer_type& c, int val)
    {
      return c = val;
    }
  };
#ifdef HAVE_ATOMIC_H
#ifndef ATOMIC_H
# include <asm/atomic.h>
#endif

  struct multi_thread 
  {
    /** The integer type */
    typedef atomic_t integer_type;
    /** Automically increment an integer
	@param c The integer variable to increment. 
	@return  The value of @c c after increment. */
    static integer_type atomic_increment(volatile integer_type& c)
    {
      return atomic_inc_and_test(&c);
    }
    /** Automically decrement an integer
	@param c The integer variable to decrement. 
	@return  The value of @c c after decrement. */
    static integer_type atomic_decrement(volatile integer_type& c)
    {
      return atomic_dec_and_test(&c);
    }
    /** Automically set an integer to a value
	@param c The integer variable to set
	@param val The value to set @c c to
	@return  The value of @c c after the assignment. */
    static integer_type atomic_set(volatile integer_type& c, int val)
    {
      return atomic_set(&c,i);
    }
  };
#endif
  //____________________________________________________________________
  /** @class ref_counted ref_counted.hh <toycalc/ref_counted.hh>
      @brief Reference counted smart pointer
      @ingroup Ref_Counted
      @see toycalc::basic_ref_counted 
  */
  template <typename Type, typename Thread=single_thread>
  class ref_counted 
  {
  public:
    /** The pointer type */
    typedef Type* pointer_type;
    /** The reference type */
    typedef Type& reference_type;
  private: 
    /** The thread-policy type */
    typedef Thread thread_type;
    /** Pointer to the atomic integer type */
    typedef typename thread_type::integer_type* counter_type;
    /** The atomic integer type */
    typedef typename thread_type::integer_type bare_counter_type;
    /** The counter */
    counter_type _counter;
    /** The reference */
    pointer_type _pointee;
    /** Initialize the counter. */
    void init_counter() 
    {
      _counter = new bare_counter_type;
      thread_type::atomic_set(*_counter,1);
    }

    /** @class tester 
	@brief Helper for @c if(ref_counted)  */
    struct tester 
    {
    public:
      tester() {}
    private:
      /** Not defined. */
      void operator delete(void*);
    };
  public:
    /** Constructor. */
    ref_counted() 
      : _pointee(0)
    {
      init_counter();
    }
    /** Constructor with assignment. 
	@param p The raw pointer to assign from */
    ref_counted(const pointer_type& p) 
      : _pointee(p) 
    {
      init_counter();
    }
    /** Copy constructor
	@param rhs The smart pointer to copy from. */
    ref_counted(const ref_counted& rhs) 
      : _counter(rhs._counter)
    {
      _pointee = clone(rhs._pointee);
    }
    /** Copy constructor
	@param rhs The smart pointer to copy from. */
    template <typename T, typename Th> 
    ref_counted(const ref_counted& rhs) 
      : _counter(reinterpret_cast<ref_counted<Type,Thread>&>(rhs)._counter)
    {
      _pointee = clone(rhs._pointee);
    }    
    /** Destructor */
    ~ref_counted() 
    {
      if (release(_pointee))
	delete _pointee;
    }

#if 0    
    ref_counted(by_reference<ref_counted> rhs) 
      : _counter(reinterpret_cast<ref_counted<Type,Thread>&>(rhs)._counter)
    {
      _pointee = clone(rhs._pointee);
    }
    /** Conversion to a by_reference 
	@return This by reference */
    operator by_reference<ref_counted>() 
    {
      return by_reference<ref_counted>(*this);
    }
#endif

    /** Create a new instance (really just return a reference) 
	@return  a new reference to the data */   
    pointer_type clone(const pointer_type& v) 
    {
      thread_type::atomic_increment(*_counter);
      return v;
    }
    /** Release the reference. 
	@param v Not used. 
	@return True if there are no more references */
    bool release(const pointer_type& v) 
    {
      if (!thread_type::atomic_decrement(*_counter)) {
	delete _counter;
	return true;
      }
      return false;
    }
    /** Swap the stored data with passed storage policy 
	@param rhs The policy to swap with. */    
    void swap(ref_counted& rhs) 
    {
      std::swap(_counter, rhs._counter);
      std::swap(_pointee, rhs._pointee);
    }
    /** Pointer access operator
	@return The bare pointer  */
    pointer_type operator->() 
    {
      assert(_pointee);
      return _pointee;
    }
    /** Access the data stored as a pointer type 
	@return stored data as a pointer  */
    pointer_type operator->() const 
    {
      assert(_pointee);
      return _pointee; 
    }
    /** Access the data stored as a reference type 
	@return stored data as a reference  */
    reference_type operator*()
    { 
      assert(_pointee);
      return *_pointee; 
    }    
    /** Access the data stored as a reference type 
	@return stored data as a reference  */
    reference_type operator*() const 
    {
      assert(_pointee);
      return *_pointee; 
    }    

    /**@{*/
    /** @name Assignment operators */
    /** Assignment
	@param rhs The smart pointer to copy from 
	@return this smart pointer */    
    ref_counted& operator=(const ref_counted& rhs) 
    {
      ref_counted t(rhs);
      t.swap(*this);
      return *this;
    }
    /** Assignment
	@param rhs The smart pointer to copy from 
	@return this smart pointer */
    template <typename T, typename H>
    ref_counted& operator=(const ref_counted<T,H>& rhs) 
    {
      ref_counted t(rhs);
      t.swap(*this);
      return *this;
    }
    /** Assignment
	@param rhs The smart pointer to copy from 
	@return this smart pointer */
    template <typename T,typename H>    
    ref_counted& operator=(ref_counted<T,H>& rhs) 
    {
      ref_counted t(rhs);
      t.swap(*this);
      return *this;
    }
    /**@}*/
    
    /**@{*/
    /** @name Boolean operators */
    /** Whether this is a valid wrapper 
	@return  @c true if this contains a valid pointer, @c false
	otherwise */
    bool operator!() const // Enables "if (!sp) ..."
    { 
      return _pointee == 0; 
    }
    /** Equality operator 
	@param lhs The ref_counted to compare to
	@param rhs The bare pointer address to compare to 
	@return @c true if @a _pointee and @a rhs are at the same
	address */
    inline friend bool operator==(const ref_counted& lhs, 
				  const pointer_type rhs)
    {
      return lhs._pointee == rhs;
    }
    /** Equality operator 
	@param lhs The bare pointer address to compare to 
	@param rhs The ref_counted to compare to
	@return @c true if @a lhs and @a rhs->_pointee are at the same
	address */
    inline friend bool operator==(const pointer_type lhs, 
				  const ref_counted& rhs)
    {
      return rhs == lhs;
    }
    /** Non-equality operator 
	@param lhs The ref_counted to compare to
	@param rhs The bare pointer address to compare to 
	@return @c true if @a lhs->_pointee and @a rhs are not at the
	same address */        
    inline friend bool operator!=(const ref_counted& lhs, 
				  const pointer_type rhs)
    {
      return !(lhs == rhs);
    }
    /** Non-equality operator 
	@param lhs The bare pointer address to compare to 
	@param rhs The ref_counted to compare to
	@return @c true if @a lhs and @a rhs->_pointee are not at the
	same address */        
    inline friend bool operator!=(const pointer_type lhs, 
				  const ref_counted& rhs)
    {
      return rhs != lhs;
    }

    /** Equality operator (ambiguity buster)
	@param rhs The smart pointer to copy from 
	@return @c true if @a rhs points to the same address as this
	wrapper */
    template <typename T, typename H>
    bool operator==(ref_counted<T,H>& rhs) 
    {
      return *this == rhs._pointee;
    }
    /** Non-equality operator (ambiguity buster)
	@param rhs The smart pointer to copy from 
	@return @c true if @a rhs does not points to the same address
	as this wrapper */
    template <typename T, typename H>
    bool operator!=(ref_counted<T,H>& rhs) 
    {
      return !(*this == rhs);
    }
    /** Ordering operator (ambiguity buster)
	@param rhs The smart pointer to copy from 
	@return @c true if @a rhs points to an address after the
	address of this wrapper */
    template <typename T, typename H>
    bool operator<(ref_counted<T,H>& rhs) 
    {
      return *this < rhs._pointee;
    }
#if 0
    /** Conversion to tester operator.  This is to make 
	@c if(ref_counted), the point is, that we convert to a pointer
	value 
	@return an address if this is a valid wrapper, 0 otherwise. */
    operator tester*() const 
    {
      if (!*this) return 0;
      static tester tl;
      return &t;
    }
#endif
    /**@}*/
    
    /** Convert to a bare pointer if so allowed. */
    operator pointer_type() const 
    {
      return _pointee;
    }
  };

  //__________________________________________________________________
  /** Equality operator 
      @param lhs The left hand smart pointer
      @param rhs The right hand smart pointer
      @return @c true if @a rhs and @a lhs are the same pointer
      @ingroup SmartPtrOp 
    */
  template <typename T, typename H, typename U>
  inline bool operator==(ref_counted<T,H>& lhs, const U* rhs) 
  {
    return ptr(lhs) == rhs;
  }
  //__________________________________________________________________
  /** Equality operator 
      @param lhs The left hand smart pointer
      @param rhs The right hand smart pointer
      @return @c true if @a rhs and @a lhs are the same pointer
      @ingroup SmartPtrOp 
    */
  template <typename T, typename H, typename U>
  inline bool operator==(const U* lhs, ref_counted<T,H>& rhs) 
  {
      return rhs == lhs;
  }
  //__________________________________________________________________
  /** Non-equality operator 
      @param lhs The left hand smart pointer
      @param rhs The right hand smart pointer
      @return @c true if @a rhs and @a lhs are not the same pointer
      @ingroup SmartPtrOp 
    */
  template <typename T, typename H, typename U>
  inline bool operator!=(ref_counted<T,H>& lhs, const U* rhs) 
  {
    return !(lhs == rhs);
  }
  //__________________________________________________________________
  /** Non-equality operator 
      @param lhs The left hand smart pointer
      @param rhs The right hand smart pointer
      @return @c true if @a rhs and @a lhs are not the same pointer
      @ingroup SmartPtrOp 
    */
  template <typename T, typename H, typename U>
  inline bool operator!=(const U* lhs, ref_counted<T,H>& rhs) 
  {
    return rhs != lhs;
  }
  //__________________________________________________________________
  /** Ordering operator (not defined)
      @param lhs The left hand smart pointer
      @param rhs The right hand smart pointer
      @return nothing (not defined)
      @ingroup SmartPtrOp 
    */
  template <typename T, typename H, typename U>
  inline bool operator<(ref_counted<T,H>& lhs, const U* rhs);
  //__________________________________________________________________
  /** Ordering operator (not defined)
      @param lhs The left hand smart pointer
      @param rhs The right hand smart pointer
      @return nothing (not defined)
      @ingroup SmartPtrOp 
    */
  template <typename T, typename H, typename U>
  inline bool operator<(const U* lhs, ref_counted<T,H>& rhs);
  //__________________________________________________________________
  /** Ordering operator (not defined)
      @param lhs The left hand smart pointer
      @param rhs The right hand smart pointer
      @return nothing (not defined)
      @ingroup SmartPtrOp 
    */
  template <typename T, typename H, typename U>
  inline bool operator>(ref_counted<T,H>& lhs, const U* rhs);
  //__________________________________________________________________
  /** Ordering operator (not defined)
      @param lhs The left hand smart pointer
      @param rhs The right hand smart pointer
      @return nothing (not defined)
      @ingroup SmartPtrOp 
    */
  template <typename T, typename H, typename U>
  inline bool operator>(const U* lhs, ref_counted<T,H>& rhs);
  //__________________________________________________________________
  /** Ordering operator
      @param lhs The left hand smart pointer
      @param rhs The right hand smart pointer
      @return @c true if @a rhs is at a lower than or same address as @a rhs
      @ingroup SmartPtrOp 
    */
  template <typename T, typename H, typename U>
  inline bool operator<=(ref_counted<T,H>& lhs, const U* rhs)
  {
    return !(rhs < lhs);
  }
  //__________________________________________________________________
  /*  Ordering operator
      @param rhs The smart pointer to copy from 
      @return @c true if @a rhs is at a lower than or same address as @a rhs
      @ingroup SmartPtrOp 
    */
  template <typename T, typename H, typename U>
  inline bool operator<(const U* lhs, ref_counted<T,H>& rhs)
  {
    return !(rhs < lhs);
  }
  //__________________________________________________________________
  /*  Ordering operator
      @param rhs The smart pointer to copy from 
      @return @c true if @a rhs is at a greater than or same address
      as @a rhs 
      @ingroup SmartPtrOp
  */ 
  template <typename T, typename H, typename U>
  inline bool operator>(ref_counted<T,H>& lhs, const U* rhs)
  {
    return !(lhs < rhs);
  }
  //__________________________________________________________________
  /*  Ordering operator
      @param rhs The smart pointer to copy from 
      @return @c true if @a rhs is at a greater than or same address
      as @a rhs 
      @ingroup SmartPtrOp
  */
  template <typename T, typename H, typename U>
  inline bool operator>(const U* lhs, ref_counted<T,H>& rhs)
  {
    return !(lhs < rhs);
  }
}


namespace std
{
/** Ordering function
    @param T The type
    @param H The tHread policy
    @param rhs The smart pointer to copy from 
    @return @c true if @a rhs is at a greater than or same address
    as @a rhs */
  template <typename T, typename H>
  struct less<toycalc::ref_counted<T,H> > : 
    public binary_function<toycalc::ref_counted<T,H>, 
			   toycalc::ref_counted<T,H>,bool>
  {
    typedef toycalc::ref_counted<T,H> arg_type;
    bool operator()(const arg_type& lhs, const arg_type& rhs) 
    {
      return less<T*>()(ptr(lhs), ptr(rhs));
    }
  };
}
#endif 
//____________________________________________________________________
//
// EOF
//
