//
// $Id: callback.hh,v 1.13 2005/08/15 15:42:59 cholm Exp $ 
//  
//  rlmm::callback
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
#ifndef rlmm_callback_hh
#define rlmm_callback_hh
#ifndef rlmm_util_hh
# include <rlmm/util.hh>
#endif

namespace rlmm
{
  /** @struct basic_callback rlmm/callback.hh <rlmm/callback.hh>
      @brief Readline in call-back mode. 

      In this mode, the user application controls the event loop, and
      readline is only executed when so requested from the
      application.  The member function @c read should be called when
      the application deems that terminal input is ok.  This mode is
      useful for for example GUIs, and probably also multithreaded
      programs.  There can only be one call back handler at a given
      time. The member function @c handle is called when a full line
      is read by successive call-backs.

      Note, that you still need to instantice a rlmm::readline
      object to control other aspects of the C library. 

      Simply instanising an object of a sub-class will not install the
      handler - the member function install must be called on the
      object.  When the object is deleted (out of scope or explicit
      delete), the call back handler is removed.  
      @ingroup READLINE
  */
  template <typename Lock>
  struct basic_callback : public singleton<basic_callback, Lock>
  {
  protected:
    /// The prompt
    std::string _prompt;
    /** Base class is a friend */
    template <template <typename> class C, typename L>
    friend class singleton;
    /** Create the call back */
    basic_callback() { }
    /** Create the call back */
    basic_callback(const basic_callback& o) : _prompt(o._prompt) {}
    basic_callback& operator=(const basic_callback& o) 
    {
      _prompt = o._prompt;
      return *this;
    }
  public:
    /// Remove and free the call back handler. 
    virtual ~basic_callback();
    /** Method to execute periodically.  
	Read one (or more) character from the input. */
    virtual void read();
    /** Get the prompt */
    const std::string& prompt() const { return _prompt; }
    /** Set the prompt, and install call back handler */
    void install(const std::string& p);
    /** Abstract member function to overload. 
	This member function is called when a full line is read from
	the input.  
	@param line The read line. */ 
    virtual void handle(char* line) {}
  };

  //____________________________________________________________________
  namespace 
  {
    /** Callback interface 
	@ingroup INTERFACE 
	@param line The line read 
    */
    template <typename Lock>
    inline 
    void 
    rlmm_callback(char* line) 
    {
      rlmm::basic_callback<Lock>::instance().handle(line);
    }
  }
  
  //____________________________________________________________________
  template <typename Lock>
  inline void
  basic_callback<Lock>::install(const std::string& prompt)
  {
    _prompt = prompt;
    rl_callback_handler_install(_prompt.c_str(), rlmm_callback<Lock>);
  }
  
  //____________________________________________________________________
  template <typename Lock>
  inline 
  basic_callback<Lock>::~basic_callback()
  {
    rl_callback_handler_remove();
  }
  
  //____________________________________________________________________
  template <typename Lock>
  inline 
  void 
  basic_callback<Lock>::read() 
  {
    rl_callback_read_char();
  }

  //____________________________________________________________________
  /** @typedef callback
      @brief Default instance of basic_callback
      @ingroup READLINE 
  */
  typedef basic_callback<single_thread> callback;
}

#endif
//____________________________________________________________________
//
// EOF
//
