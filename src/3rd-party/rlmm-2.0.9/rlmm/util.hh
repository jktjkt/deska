//
// $Id: util.hh,v 1.10 2005/08/15 15:42:59 cholm Exp $ 
//  
//  rlmm::completion
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
#ifndef rlmm_util_hh
#define rlmm_util_hh
#ifndef __STRING__
# include <string>
#endif
#ifndef __STDEXCEPT__
# include <stdexcept>
#endif
#define USE_VARARGS   1
#define PREFER_STDARG 1
#ifndef _READLINE_H_
# include <readline/readline.h>
#endif 
#include <stdlib.h>

namespace rlmm 
{
  /** @defgroup UTIL Utilities 
      @ingroup READLINE 
      @brief Utility classes, template classes, functions, and
      template functions. 
  */
  /** @struct single_thread util.hh <rlmm/util.hh>
      @brief Default locking - for single threaded applications.

      This is useful for single threaded applications that need no
      protection of global memory.  For more on this, please refer to
      @ref locking 
      
      @ingroup UTIL
   */
  struct single_thread 
  {
    /** CTOR */
    single_thread() {}
    /** DTOR */
    ~single_thread() {}
    /** Locks the lock.  Does nothing */
    void lock() {}
    /** Unlocks the lock.  Does nothing */
    void unlock() {}
  };

  /** @struct guard util.hh <rlmm/util.hh>
      @brief Utility class to lock a lock in a scope, and unlock it at axit
      of a scope. 
      @ingroup UTIL 

      @code
      {
         guard<Lock> g(lock); // Locks the lock, if possible
	 ...                  // Do stuff  
      } // Calls guard<Lock>::~guard which unlocks the lock
      @endcode 
   */
  template <typename Lock>
  struct guard 
  {
    /** Lock type */
    typedef Lock lock_type;
    /** Reference to lock */
    lock_type& _lock;
    /** Construct the guard. Locks the passed lock @a l 
        @param l Lock to lock. */
    guard(lock_type& l) : _lock(l) { _lock.lock(); }
    /** Destructor.  Unlocks the lock. */
    ~guard() { _lock.unlock(); }
  };


  /** @struct one_at_a_time util.hh <rlmm/util.hh>
      @brief Utility class for common behaviour.    
      @ingroup UTIL 

      Sub-classes of this class has methods to activate and deactivate
      a specific object.  Sub-classes should implement the member
      function @c do_activate and @c do_deactivate.  Preferably these
      member functions should be private or protected - which,
      however, requires that the sub-class befriends this class
      template:
      @code 
      template <typename Lock>
      class one : public one_at_a_time<one,Lock>
      {
        template <template <typename> class C,typename L>
	friend class one_at_a_time;
	void do_activate() { activation code }
	void do_daactivate() { daactivation code }
        ...
      };
      @endcode 
      @param Client The sub class template 
      @param Lock The lock type. 
   */
  template <template <typename> class Client, typename Lock=single_thread> 
  struct one_at_a_time
  {
  protected:
    /** Lock type */
    typedef Lock lock_type;
    /** Client type */
    typedef Client<Lock> client_type;
    /** Guard type */
    typedef guard<Lock>  guard_type;
    /** The current active object, if any. */
    static  client_type* _current;
    /** The default object */
    static  client_type _default;
    /** The lock used */
    static  lock_type    _lock;
    /** Protected constructor. */
    one_at_a_time() {}
    /** Proteced copy constructor */
    one_at_a_time(const one_at_a_time& o) {}
    /** Protected assignment operator */
    one_at_a_time& operator=(const one_at_a_time& o) { return *this; }
    /** Call-back at activation time */
    virtual void do_activate() = 0;
    /** Call-back at deactivation time */
    virtual void do_deactivate() = 0;
  public:
    /** Destructor */
    virtual ~one_at_a_time() {}
    /** Activate an object. 
	@return The old active object. */
    client_type& activate() 
    {
      guard_type g(_lock);
      client_type& old = (_current ? *_current : _default);
      if (old.is_active()) old.do_deactivate();
      do_activate();
      _current = dynamic_cast<client_type*>(this);
      return old;
    }
    /** Deactivate an object. */
    void deactivate() 
    {
      guard_type g(_lock);
      if (!is_active()) return;
      _current = &_default;
      do_deactivate();
      if (this != &_default) _default.do_activate();
    }
    /** Return the active object */
    static client_type& active() 
    {
      client_type& c = (_current ? *_current : _default);
      if (!c.is_active()) c.activate();
      return c;
    }
    /** Test if this oject is the active one */ 
    bool is_active() const 
    {
      return (this == _current);
    }
  };
  
  // Define one at a time lock
  template <template <typename> class Client, typename Lock>
  Lock one_at_a_time<Client, Lock>::_lock;

  // Define one at a time instance
  template <template <typename> class Client, typename Lock> 
  Client<Lock>*  one_at_a_time<Client, Lock>::_current = 0;

  // Define one at a time default instance
  template <template <typename> class Client, typename Lock> 
  Client<Lock>  one_at_a_time<Client, Lock>::_default;
    
  /** @struct singleton util.hh <rlmm/util.hh>
      @brief Utility class to implement singletons.  
      @ingroup UTIL 

      Sub classes will be singletons.  In sub classes, the constructor
      should preferable be protected (or even private) - which,
      however, requires that the sub-class befriends this class
      template:
      @code 
      template <typename Lock>
      class only : singleton<only,Lock>
      {
        template <template <typename> class C,typename L>
	friend class singleton;
	only() {}
	only(const only& o) {}
	only& operator=(const only& o) { return *this; }
        ...
      };
      @endcode 
      @param Client The sub class template 
      @param Lock The lock type. 
   */
  template <template <typename> class Client, typename Lock=single_thread> 
  struct singleton
  {
  protected: 
    /** Lock type */
    typedef Lock         lock_type;
    /** Client type */
    typedef Client<Lock> client_type;
    /** Guard type */
    typedef guard<Lock>  guard_type;
    /** The singleton */
    static  client_type* _instance;
    /** The global lock */
    static  lock_type    _lock;
    /** Protected constructor */
    singleton()
    {
      if (_instance) throw std::runtime_error("only one instance allowed");
      _instance = static_cast<client_type*>(this);
    }
    /** Protected copy constructor */
    singleton(const singleton& o) {}
    /** Protected assignment operator */
    singleton& operator=(const singleton& o) { return *this; }
  public:
    /** @return A reference to the singleton.  Client code should only
	access the class using this static member function */
    static client_type& instance() 
    {
      if (!_instance) {
	guard_type g(_lock);
	if (!_instance) /* _instance = */ new client_type;
      }
      return *_instance;
    }
    /** Destructor.  Removes the singleton. */
    virtual ~singleton() 
    {
      _instance = 0;
    }
  };

  // Define singleton lock
  template <template <typename> class Client, typename Lock>
  Lock singleton<Client, Lock>::_lock;

  // Define singleton instance 
  template <template <typename> class Client, typename Lock> 
  Client<Lock>*  singleton<Client, Lock>::_instance = 0;

  namespace 
  {
    //__________________________________________________________________
    /** Copy a C++ string to a newly (malloc) allocated C string. 
	@ingroup UTIL
	@param orig C++ string 
	@return newly malloc'ed C string, or NULL on failure */
    inline char* 
    rlmm_string_duplicate(const std::string& orig) 
    {
      size_t n      = orig.size();
      char*  retval = (char*)malloc((n + 1) * sizeof(char));
      if (!retval) return 0;
#if defined(__GNUC__) && __GNUC__ < 3
      memcpy(retval, orig.c_str(), n);
#else
      orig.copy(retval, n);
#endif
      retval[n]     = '\0';
      return retval;
    }

    //__________________________________________________________________
    /** Get the directory separator for the operating system
	@ingroup UTIL
	@return @c '\\' on Windows (but not Cygwin), @c '/' otherwise */
    inline char
    rlmm_directory_separator() 
    {
#if defined(__WINDOWS__) && !defined(__CYGWIN__)
      return '\\';
#else 
      return '/';
#endif
    }
    //__________________________________________________________________
    /** Get the basename of a file. 
	The returned string is a pointer into a static buffer and
	should be copied immediately if needed. The behaviour of this
	function is the same as XPG <tt>basename</tt>, except that it
	does not modify it's argument, and the returned pointer does
	not point into the argument.
	@param name The path to extract the basename from. 
	@return basename of file e.g., 
	@ingroup UTIL
	@code 
	rlmm_basename("/boot/vmlinuz") == "vmlinuz";
	rlmm_basename("/usr/bin/")     == "bin";
	rlmm_basename("/")             == "/";
	rlmm_basename("")              == ".";
	rlmm_basename(NULL)            == ".";
	@endcode */
    template <typename Lock>
    inline char* 
    rlmm_basename(const char* name) 
    {
      static Lock lock;
      static char buf[1024];
      guard<Lock> g(lock);
      buf[0] = '.'; buf[1] = '\0';
      // Check that we got a valid argument
      if (!name || name == (char*)NULL || name[0] == '\0') return buf;
  
      // Copy arg into buffer for manipulations
      strcpy(buf, name);
      // Get a pointer to end of the argument 
      char* e = buf + strlen(buf) - 1;
      // Remove all trailing slashes 
      while (*e == rlmm_directory_separator() && e != buf) { *e = '\0'; e--; }
      // Find last slash in the buffer 
      e = strrchr(buf, rlmm_directory_separator());
      // If we found it and it isn't the same as buffer, return 
      if (!e && e != buf) return buf;
      return e+1;
    }

    //__________________________________________________________________
    /** Get the dirname of a file. 
	The returned string is a pointer into a static buffer and
	should be copied immediately if needed. The behaviour of this
	function is the same as XPG <tt>dirname</tt>, except that it
	does not modify it's argument, and the returned pointer does
	not point into the argument.
	@param name The path to extract the dirname from. 
	@return dirname of file e.g., 
	@ingroup UTIL
	@code 
	rlmm_dirname("/boot/vmlinuz") == "/boot";
	rlmm_dirname("/usr/bin/")     == "/usr";
	rlmm_dirname("/")             == "/";
	rlmm_dirname("")              == ".";
	rlmm_dirname(NULL)            == ".";
	@endcode */
    template <typename Lock>
    inline char* 
    rlmm_dirname(const char* name) 
    {
      static Lock lock;
      static char buf[1024];
      guard<Lock> g(lock);
      buf[0] = '.'; buf[1] = '\0'; 
      // Check that we got a valid argument  
      if (!name || name == (char*)NULL || name[0] == '\0') 
	return buf;
  
      // Copy arg into buffer for manipulations
      strcpy(buf, name);
      // Get pointr to end
      char* e = buf + strlen(buf) - 1;
      // remove trailing slashes
      while (*e == rlmm_directory_separator() && e != buf) { *e = '\0'; e--; }
      // find last slash 
      e = strrchr(buf, rlmm_directory_separator());
      // If found,
      if (e) {
	// and it's at the start, we got a single slash only, so step
	if (e == buf) { 
	  e++;
	  // Remove trailing slashes 
	  while (*e == rlmm_directory_separator()) { *e = '\0' ; e++; }
	}
	// Set end-of-string mark
	*e = '\0';
      }
      else {
	// Otherwise, we're at the start, and we need to return a dot
	buf[0] = '.'; buf[1] = '\0';
      }
      return buf;
    }
  }
}

#endif
//
// EOF
//
      
