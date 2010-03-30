//
// $Id: signal_handler.hh,v 1.12 2005/08/13 19:51:31 cholm Exp $ 
//  
//  rlmm::signal_handler
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
#ifndef rlmm_signal_handler_hh
#define rlmm_signal_handler_hh
#ifndef rlmm_util_hh
# include <rlmm/util.hh>
#endif

namespace rlmm
{
  /** @struct basic_signal_handler signal_handler.hh <rlmm/signal_handler.hh>
      @brief signal handler utility class
      @ingroup READLINE
      Signal handling for readline. If signal handling is enabled, the
      signals, INT, QUIT, TERM, ALRM, TSTP, TTOU and possibly WINCH
      are caught and dealt with in readline.  Note, that the signal
      handler isn't installed until the member function install is
      called. 
  */
  template <typename Lock>
  class basic_signal_handler : public singleton<basic_signal_handler,Lock>
  {
  private:
    /** Base class is a friend */
    template <template <typename> class C, typename L>
    friend class singleton;
    /** Constructor.  */
    basic_signal_handler();
    /** Constructor.  */
    basic_signal_handler(const basic_signal_handler& o) {}
    /** Constructor.  */
    basic_signal_handler& operator=(const basic_signal_handler& o) 
    {
      return *this;
    }
  public:
    /** Also catch SIG_SIGWINCH */
    void catch_siqwinch(bool ok) { rl_catch_sigwinch = ok ? 1 : 0; }
    /** Turn on singal handling */
    void on() { rl_catch_signals = 1; }
    /** Turn off singal handling */
    void off() { rl_catch_signals = 0; }
    /// DTOR
    virtual ~basic_signal_handler(); 
    /** Are we catching signals? 
	@return true if signal handling is on */
    bool is_on() const { return rl_catch_signals == 0 ? false : true; }
    /** Are we also catching SIGWINCH? 
	@return true if SIGWINCH is caught. */
    bool catch_sigwinch() const {return rl_catch_sigwinch == 0 ? false : true;}
    /// Cleanup after a signal. 
    void cleanup() { rl_cleanup_after_signal();  }
    /// Reset signal handler after signal.
    void reset() { rl_reset_after_signal(); }
    /// Install signal handler. 
    static void install() { rl_set_signals(); }
    /// Remove the signal handler.
    static void remove() { rl_clear_signals(); }
       
  };

  //__________________________________________________________________
  template <typename Lock>
  inline 
  basic_signal_handler<Lock>::basic_signal_handler()
  {
    basic_signal_handler<Lock>::_instance = this;
  }
  
  //__________________________________________________________________
  template <typename Lock>
  inline 
  basic_signal_handler<Lock>::~basic_signal_handler()
  {
    off();
    catch_sigwinch(false);
    remove();
  }

  //_________________________________________________________________
  /** @typedef signal_handler 
      @brief Default instance of basic_signal_handler
      @ingroup READLINE
  */
  typedef basic_signal_handler<single_thread> signal_handler;
}

#endif
//____________________________________________________________________
//
// EOF
//
