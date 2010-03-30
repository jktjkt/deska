//
// $Id: mainpage.cc,v 1.8 2005/08/12 14:11:26 cholm Exp $ 
//  
//  rlmm::streambuf
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
#error Not for compilation - only documentation. 

/** @mainpage
    @image html logo.png 

    This package implements a full GNU Readline interface in C++. The
    API is quite simple and makes heavy use of templated container,
    standard classes and so on.  The idea is that a C++ developer
    should fell resonabily comfortable with the library API.  

    @section overview Overview

    The C library API function have been split into various class
    templates that deal with one specific topic.  Each template takes
    on parameter, the thread locking type. 
    <dl>
      <dt> rlmm::basic_readline </dt>
      <dd> This is the main class.  Users can manipulate the basic
        readline behaviour through this class. </dd> 
      <dt> rlmm::basic_history </dt>
      <dd> All history related operations go through this class. The
        class has been implmented such that it is possible (and easy)
	to have multiple histories (think of Emacs) in the same
	application. </dd> 
      <dt> rlmm::basic_key_map </dt>
      <dd> Binding keys and key-sequences go through this
        class. Mutliple key-maps can exist in a single
	application. </dd> 
      <dt> rlmm::basic_function_map </dt> 
      <dd> Manipluation of named functions. </dd> 
      <dt> rlmm::basic_completion </dt>
      <dd> An Abstract Base Class (ABC) for user defined completions.
        The class defines an interface the users can derive from to
	implement a completion scheme.  The user need at the minimum
	to implement a member function to get the list of possible
	completions (rlmm::basic_completion::possibilities) and a member
	function that will serve these completions
	(rlmm::basic_completion::complete).  If the application uses
	containers of some sort, these member functions can be very
	effectively implemented, by simply giving references into the
	containers. </dd> 
      <dt> rlmm::basic_callback </dt>
      <dd> An ABC for using Readline in a call-back fashion.  The user
        should derive her call-back class from this class, and
	implement the handler member function to handle lines of
	input. </dd> 
      <dt> rlmm::basic_signal_handler </dt> 
      <dd> Utility class </dd>
    </dl>

    In addition, there's a number of utility classes and class
    templates 
    <dl>
      <dt> rlmm::basic_buffer </dt> 
      <dd> Interface to readlines internal buffer </dd>
      <dt> rlmm::undo </dt>
      <dd> @e guard type class to implement undoing in a certain
      region. </dd>
      <dt> rlmm::command </dt>
      <dd> Interface functor class for commands.  Users can derive
      from this class to define new functions </dd>
    </dl>
    @subsection additional Stream interface. 
    @code 
    rlmm::basic_streambuf
    @endcode
    Use the library as a regular @c std::istream.  This makes
    the code very transparent, and easy to plug into other kinds
    of applications that accepts an @c std::istream as the input
    method. See also the @link streamer.cc <tt>streamer</tt>
    example @endlink

    @section locking Thread safety
    Thread safety is implemented in the class templates via a template
    parameter.   This parameter should be a class that defines the
    member functions @c lock() and @c unlock().  The semantics of
    these member functions, is that the @c lock() member function
    should lock a specific syncronisation objet, like a @e mutex or @c
    CriticalSection.  Similarly the @c unlock() member function should
    release the lock of the underlying sycronisation oject. 

    An example for POSIX threads could be 
    @code 
    #include <pthread.h>

    class posix_lock 
    {
    private:
      pthread_mutex_t _mutex;
    public:
      posix_lock() { pthread_mutex_init(&_mutex, NULL); }
      ~posix_lock() { pthread_mutex_destroy(&_mutex); }
      void lock() { pthread_mutex_lock(&_mutex); }
      void unlock() { pthread_mutex_unlock(&_mutex); }
    };
    @endcode 
      
    Note, that the class lilbrary provides the class
    rlmm::single_thread that does no locking what so ever. 

    @section concepts Singletons and `One at a time' classes
    A number of class templates in the library are singletons.  That
    is, only one such object can exists at a given time.   The object
    is accessed via @e @<class@> @c ::instance(), which returns a
    reference to the object.   Singletons in the library is
    implemented via the utility class rlmm::singleton. 

    Singletons in the library are 
    <ul>
      <li> rlmm::basic_buffer</li>
      <li> rlmm::basic_callback</li>
      <li> rlmm::basic_function_map</li>
      <li> rlmm::basic_readline</li>
      <li> rlmm::basic_signal_handler</li>
   </ul>

    For a number of the class templates in the library, only one
    object can be active at given time.   These are termed `one at a
    time' classes in the library.   A given object is made active by
    the member function @c activate(), which deactivates the current
    active object, and activates the object which the member function
    was called upon.   The member function @c deactivate() deactivates
    the object is it called upon, if and only if it is the current
    actie object.  It then restores the default object as current
    active object.  `one at a time' classes are implemented via
    rlmm::one_at_a_time. 

    `One at a time' class templates in the library are:
    <ul>
      <li> rlmm::basic_completion </li>
      <li> rlmm::basic_history </li>
      <li> rlmm::basic_key_map </li>
      <li> rlmm::basic_terminal </li>
    </ul>

    
*/
//____________________________________________________________________
//
// EOF
//
