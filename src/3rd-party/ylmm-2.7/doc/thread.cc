//
// $Id: thread.cc,v 1.2 2003/06/21 20:22:28 cholm Exp $
//
//   thread.cc
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
/** @file   doc/thread.cc
    @author Christian Holm
    @date   Fri Jan 03 05:00:58 2003
    @brief  Buffer documentation */
/** @page thread_issues Thread-safety of the classes

    The thread-safety level of the classes depends greatly on the
    thread-safty of the underlying parser and scanner generators 
    (@b Yacc and @b Lex).  If they produce thread-safe code, then the
    classes are thread-safe.  

    For example, @b Bison allows you to specify the parser to be
    reentrant (does not depend on global variables or static data) via
    the definition @c %pure_parser.  If this is desired for the client
    application, then the grammar can define that, and the
    ylmm::basic_parser class will respect that. 

    That said, it is actually easy to make the client code thread-safe
    using a little disciplin.  If the client code derives classes from
    ylmm::basic_parser and ylmm::basic_scanner and implment locking in
    these classes, the client code is for all pratical purposes thread
    safe.  The only caveat, is that all access to the parser and
    scanner should go through that derived layer - client code @e must
    @e not  manipulate the global variable @c yylval and similar
    directly.  That disciplin is in practise easy to achive, as a C++
    programmer will in general be more comfortable manipulating an
    object of a class rather than some global low-level variable. 

    That said, there is one point where the multi-threaded programmer
    @c must take care.  The class ylmm::basic_messenger has a static
    member ylmm::basic_messenger::_default_handler which is the
    default message handler.  That member is initialised as a
    singleton, meaning it may potentially fall pray to race
    conditions.  

    To protect against that, ylmm::basic_messenger accepts a template
    argument @c Lock, which should be the type of a thread
    syncronisation locking class (a @e mutex).  The class must: 
    <ul>
    <li> Be default constructable</li>
    <li> Have a member function @c lock with the semantics of aquiring
      a mutually exclusive lock.  For example on a POSIX platform,
      that member function could call @c pthread_mutex_lock, while on
      a Win32 platform it could call @c EnterCriticalSection. </li>
    <li> Have a member function @c unlock with the semantics of
      releasing the previously aquired mutually exclusive lock. 
      On a POSIX platform that member function could call 
      @c pthread_mutex_unlock, while on a Win32 it could call 
      @c LeaveCriticalSection. </li>
    </ul>

    All classes that manipulates, directly or indirectly, a
    ylmm::basic_messenger has a similar argument, which is passed on
    to the ylmm::basic_messenger class.  So, for a multi-threaded
    application on a platform that provides a POSIX interface, this
    could spell out to be 
    @code 
    #ifndef _PTHREAD_H
    #include <pthread.h>
    #endif

    class thread_lock { 
    private: 
      pthread_mutex_t _mutex;
    public:
      thread_lock() { pthread_mutex_init(&_mutex); }
      ~thread_lock() { pthread_mutex_destroy(&_mutex); }
      void lock() pthread_mutex_lock(&_mutex); }
      void unlock() pthread_mutex_lock(&_mutex); }
    }; 

    class thread_guard {
    private: 
      thread_lock& _lock;
    public:
      thread_guard(thread_lock& l) : _lock(l) { _lock.lock(); }
      ~thread_guard() { _lock.unlock(); }
    };
    @endcode
    
    The client can then use that class as an argument to the parser
    and scanner classes: 
    
    @code 
    class node;
    
    class scanner : public ylmm::basic_scanner<node,ylmm::location,
                                               0, thread_lock>;
    class parser : public ylmm::basic_parser<node,ylmm::location,
                                             0, thread_lock>;
    @endcode
    
    Both ylmm::basic_parser and ylmm::basic_scanner defines the nested
    type @c lock_type, which the application can then use as
    applicable. For example to protect the token object in the parser,
    the application could do:
    @code 
    class parser : public ylmm::basic_parser<node,ylmm::location,
                                             0, thread_lock> { 
    private:
      lock_type _lock;
      ylmm::basic_parser<node,ylmm::location,0, thread_lock> _scanner;
    public: 
      int scan(void* arg) { 
        thread_guard g(_lock);
	return _scanner->next(&token());
      }
      ...
    }
    @endcode
    (Another C++ idom was used here - the guard that exploits the
    scoping of variables - sometimes it pays off reading the
    periodicals). 
*/
#error This file is not for compilation
//
// EOF
//

	
    


