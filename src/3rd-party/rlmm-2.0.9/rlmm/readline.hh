//
// $Id: readline.hh,v 1.14 2005/08/13 19:51:31 cholm Exp $ 
//  
//  CxxInt::readline
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
#ifndef rlmm_readline_hh
#define rlmm_readline_hh
#ifndef rlmm_command_hh
# include <rlmm/command.hh>
#endif


/** @defgroup READLINE The Readline classes. */
/** @defgroup INTERFACE Interface functions 
    @ingroup READLINE 
*/
/** Namespace for C++ interface to C readline library. 
    This namespace holds some classes for using the C readline library
    with C++.  The idea is to use STL containers and standard strings
    rather thant C strings and arrays, so that the user may have the
    full power of C++ available.  However, this does create a bit of
    overhead, as we sometimes have to allocate and deallocate memory
    as we go from C arrays to STL containers and back. This has of
    course been kept to a minimum to the authors ability.  The author
    believes that the overhead is a small price to pay for the
    increased ease of use due to the STL containers and like.  As far
    as possible, the readline have been kept as coheriant as
    possible, but a few odd things does pop up from time to time. 
*/
namespace rlmm 
{
  //__________________________________________________________________
  /** @struct basic_readline readline.hh <rlmm/readline.hh>
      @brief Readline.
      Interface the C readline library 
      @ingroup READLINE
      A very basic example is
      @include "basic.cc" */
  template <typename Lock>
  class basic_readline : public singleton<basic_readline,Lock>
  {
  protected:
    /** Base class is a friend */
    template <template <typename> class C, typename L>
    friend class singleton;
    /** Create an readline. */
    basic_readline();
    /** Constructor.  */
    basic_readline(const basic_readline& o);
    /** Constructor.  */
    basic_readline& operator=(const basic_readline& o);
    /** Name of the readline application */
    mutable std::string _name;
    /** parenthesis blink timeout in @f$ \mu{}s@f$ */
    int _parenthesis_blink_timeout;
  public:
    /** EditingMode. 
	Enum of possible editing modes */ 
    enum edit_mode {
      vi,
      emacs
    };

    /** State. 
	The state of the readline library. */
    enum state_bit {
      /// nothing yet
      none,           
      /// What you expect
      initializing,
      /// What you expect
      initialized, 
      /// The terminal is prepared 
      terminal_preped, 
      /// Reading a command
      command,        
      /// Read a meta-prefix
      meta,           
      /// Dispatching to a cimmand 
      dispatching, 
      /// Reading more input after an editing command 
      more, 
      /// Performing incremental search
      incremental, 
      /// Performing a non-incremental search
      non_incremental, 
      /// Performing a search 
      search,
      /// Reading a numeric argument 
      numeric_arg, 
      /// Input from a macro expansion 
      macro_input, 
      /// Reading charaters for a macro replacement text
      macro_define, 
      /// In overwrite mode
      overwrite, 
      /// In completion mode 
      completing, 
      /// In the signal handler 
      signal, 
      /// Undoing previous stuff
      undo, 
      /// More to read 
      pending, 
      /// About to return line to caller
      finish
    };
    
    
    /** DTOR*/ 
    virtual ~basic_readline() {  }
    /** Get the unique application name. 
	@return the unique application name */
    const std::string& name() const;
    /** Set the unique appliation name. 
	@param name the application name. */
    void name(const std::string& name); 

    /// @name Initialise
    /*@{*/
    /** Read settings from a file 
	@param file file to read from. If empty, then the default file
	is read, which is (in order or priority): Last file read with
	this member function; value of INPUTRC; or ~/.inputrc. 
	@return 0 on success, otherwise errno */ 
    int read_init_file(const std::string& file=std::string()); 
    /** Parse a line and do bindings accordingly.
	@param line Line to parse. 
	@return true on success, false otherwise. */
    bool parse(const std::string& line);
    /*@}*/

    /// @name Reading lines
    /*@{*/
    /** Read one line of input 
	@param prompt Prompt to be displayed. 
	@param line Line read. 
	@return true on success, false otherwise. */ 
    bool read(const std::string& prompt, std::string& line);
    /** Read one line of input 
	User must free() the returned line read.
	@param prompt Prompt to be displayed. 
	@param line Line read (NULL terminated). 
	@return true on success, false otherwise. */ 
    bool read(const std::string& prompt, char*& line);
    /*@}*/
    

    /// @name Status information
    /*@{*/
    /** Get the status of readline.
	This is an bitwise or of the bits in the state enum
	@return the status of readline.*/
    int state() const; 
    /** Set the status of readline.
	@param status A bit-wise or of state bits. */ 
    void state(int status); 
    /** Set a bit in the status of readline.
	@param bit A state bit. 
	@param on if true, bit is turned on, otherwise off. */
    int state(state_bit bit, bool on=true); 
    /** Test a bit in the status. 
	@param bit State bit to test. 
	@return true if bit is on. */
    bool test_state(state_bit bit) const;
    /*@}*/

    
    /// @name Prompting
    /*@{*/
    /** Get the current prompt
	@return the current prompt */
    const std::string& prompt() const;
    /** Set the prompt. 
	@param prompt Prompt to use. */
    void prompt(const std::string& prompt);
    /** Save away the prompt */
    void save_prompt();
    /** Restore the prompt previously saved */
    void restore_prompt();
    /** Expand the prompt to new prompt.
	@param prompt The new prompt. */
    void expand_prompt(const std::string& prompt);
    /** Set if application does the prompting. 
	If the application is in charge of doing the prompting itself,
	it should tell readline so by calling this member function. 
	@param already Should be true if applcation sets the prompt. */
    void already_prompted(bool already=true);
    /** Does application control the prompt?
	@return true if application deals with the prompt itself. */
    bool already_prompted() const;
    /*@}*/
    
    /// @name Editing mode
    /*@{*/
    /** Get the editing mode. 
	@return emacs for Emacs-like, and vi for vi-like */
    edit_mode editing_mode() const; 
    /** Set the editing mode. 
	@param mode Mode to use. */
    void editing_mode(edit_mode mode=emacs) ; 
    /*@}*/

    /// @name Miscellaneous parameters 
    /*@{*/
    /** See if empty lines are dicarded. 
	@return true if clearing empty lines */
    bool clear_empty_lines() const;
    /** Set wether or not to clear empty lines. 
	@param clear true if empty lines should be ignored. */
    void clear_empty_lines(bool clear=true);
    /** Get the limit on reads before returning . 
	@return Maximum number of characters to read before
	returning. 0 or less means upto a newline. */
    int max_read() const;
    /** Set the limit on reads before returning 
	@param n Maximum number of characters to read before
	returning. 0 or less means upto a newline. */
    void max_read(int n);    
    /** Set the timeout (micro seconds) before showing matching
	parenthesis. 
	@param us timeout in microseconds. */
    void parenthesis_timeout(int us);
    /*@}*/

    /// @name Miscellaneous operations
    /*@{*/ 
    /// Force readline to return immediately 
    void done();
    int parenthesis_blink_timeout() const {return _parenthesis_blink_timeout;}
    /** Set the parenthis blink timeout in @f$ \mu{}s@f$
	@param us Timeout. */
    void parenthesis_blink_timeout(int us);
    /// Get the version of readline (C) library 
    std::string version();
    /// Test if this GNU readline (it is, trust me)
    bool gnu_readline(); 
    /*@}*/

    /// @name Variables and functions
    /*@{*/
        /** Set a variable.
	@param var variable name. 
	@param val The value. */ 
    int variable(const std::string& var, const std::string& val);
    /// Return pointer to last command executed. 
    static command_t last_command();
    /// Show a list of currently defined variables 
    void list_variables(bool inputrc=false);    
    /*@}*/

    /// @name Hooks
    /*@{*/
    /** Input hook.
	member function to be run after input timeout. Users can
	overload this function to do some period checking.
	@return has no special meaning and is ignored by C API. */
    virtual int input_hook();
    /** Event hook.
	member function to be run after event timeout. Users can
	overload this function to do some period checking. 
	@return has no special meaning and is ignored by C API. */
    virtual int event_hook();
    /** Startup hook.
	member function to be run after event timeout. Users can
	overload this function to do some period checking. 
	@return has no special meaning and is ignored by C API. */
    virtual int startup_hook();
    /*@}*/    
  };
  namespace 
  {
    //====================================================================
    /** Hook to run on input 
	@ingroup INTERFACE 
	@return basic_readline<Lock>::input_hook() */
    template <typename Lock>
    inline int
    rlmm_input_hook() 
    {
      return basic_readline<Lock>::instance().input_hook(); 
    }

    //____________________________________________________________________
    /** Hook to run on an event
	@ingroup INTERFACE 
	@return basic_readline<Lock>::event_hook() */
    template <typename Lock>
    inline int
    rlmm_event_hook() 
    {
      return basic_readline<Lock>::instance().event_hook(); 
    }

    //____________________________________________________________________
    /** Hook to run on startup
	@ingroup INTERFACE 
	@return basic_readline<Lock>::startup_hook() */
    template <typename Lock>
    inline int
    rlmm_startup_hook() 
    {
      return basic_readline<Lock>::instance().startup_hook(); 
    }
  }

  //__________________________________________________________________
  /// Startup hook
  template <typename Lock>
  inline int 
  basic_readline<Lock>::startup_hook() 
  {
    return 0; 
  } 
  //__________________________________________________________________
  /// Input hook
  template <typename Lock>
  inline int 
  basic_readline<Lock>::input_hook() 
  {
    return 0; 
  } 
  //__________________________________________________________________
  /// Event Hook
  template <typename Lock>
  inline int 
  basic_readline<Lock>::event_hook() 
  {
    return 0; 
  } 


  //____________________________________________________________________
  template <typename Lock>
  inline 
  basic_readline<Lock>::basic_readline() 
  {
    rl_initialize();
    rl_pre_input_hook   = &rlmm_input_hook<Lock>;
    rl_event_hook       = &rlmm_event_hook<Lock>;
    rl_startup_hook     = &rlmm_startup_hook<Lock>;
    _parenthesis_blink_timeout = 500000;
  }

  //____________________________________________________________________
  template <typename Lock>
  inline 
  basic_readline<Lock>::basic_readline(const basic_readline& o) 
    : _name(o._name)
  {}
  //____________________________________________________________________
  template <typename Lock>
  inline basic_readline<Lock>& 
  basic_readline<Lock>::operator=(const basic_readline& o) 
  {
    _name = o._name;
    return *this;
  }
  
  //__________________________________________________________________
  template <typename Lock>
  inline int
  basic_readline<Lock>::read_init_file(const std::string& file) 
  {
    return rl_read_init_file(file.c_str());
  }

  //__________________________________________________________________
  template <typename Lock>
  inline bool
  basic_readline<Lock>::parse(const std::string& line) 
  {
    return (rl_parse_and_bind(const_cast<char*>(line.c_str())) 
	    == 0 ? true: false);
  }

  //__________________________________________________________________
  template <typename Lock>
  inline bool
  basic_readline<Lock>::read(const std::string& prompt, char*& line) 
  {
    line = ::readline(prompt.c_str());
    if (!line) 
      // Return false, to indicate EOF
      return false;
    return true;
  }


  //__________________________________________________________________
  template <typename Lock>
  inline bool
  basic_readline<Lock>::read(const std::string& prompt, std::string& line) 
  {
    char* result;
    if (!this->read(prompt, result))
      return false;

    // Empty std::string and assign new value 
    line.erase(); 
    line = result;

    // Free the readline malloc'ed memory.  
    free(result);

    // Return an OK
    return true;
  }


  //__________________________________________________________________
  template <typename Lock>
  inline int
  basic_readline<Lock>::state() const
  {
    int retval = 0;
    if (RL_ISSTATE(RL_STATE_NONE))         retval |= (1 << none);
    if (RL_ISSTATE(RL_STATE_INITIALIZING)) retval |= (1 << initializing);
    if (RL_ISSTATE(RL_STATE_INITIALIZED))  retval |= (1 << initialized);
    if (RL_ISSTATE(RL_STATE_TERMPREPPED))  retval |= (1 << terminal_preped);
    if (RL_ISSTATE(RL_STATE_READCMD))      retval |= (1 << command);
    if (RL_ISSTATE(RL_STATE_METANEXT))     retval |= (1 << meta);
    if (RL_ISSTATE(RL_STATE_DISPATCHING))  retval |= (1 << dispatching);
    if (RL_ISSTATE(RL_STATE_MOREINPUT))    retval |= (1 << more);
    if (RL_ISSTATE(RL_STATE_ISEARCH))      retval |= (1 << incremental);
    if (RL_ISSTATE(RL_STATE_NSEARCH))      retval |= (1 << non_incremental);
    if (RL_ISSTATE(RL_STATE_SEARCH))       retval |= (1 << search);
    if (RL_ISSTATE(RL_STATE_NUMERICARG))   retval |= (1 << numeric_arg);
    if (RL_ISSTATE(RL_STATE_MACROINPUT))   retval |= (1 << macro_input);
    if (RL_ISSTATE(RL_STATE_MACRODEF))     retval |= (1 << macro_define);
    if (RL_ISSTATE(RL_STATE_OVERWRITE))    retval |= (1 << overwrite);
    if (RL_ISSTATE(RL_STATE_COMPLETING))   retval |= (1 << completing);
    if (RL_ISSTATE(RL_STATE_SIGHANDLER))   retval |= (1 << signal);
    if (RL_ISSTATE(RL_STATE_UNDOING))      retval |= (1 << undo);
    if (RL_ISSTATE(RL_STATE_INPUTPENDING)) retval |= (1 << pending);
    if (RL_ISSTATE(RL_STATE_DONE))         retval |= (1 << finish);
    return retval;
  }
  
  //__________________________________________________________________
  template <typename Lock>
  inline void
  basic_readline<Lock>::state(int state) 
  {
    rl_readline_state = 0;
    if (state & (1 << none))            RL_SETSTATE(RL_STATE_NONE);
    if (state & (1 << initializing))    RL_SETSTATE(RL_STATE_INITIALIZING);
    if (state & (1 << initialized))     RL_SETSTATE(RL_STATE_INITIALIZED);
    if (state & (1 << terminal_preped)) RL_SETSTATE(RL_STATE_TERMPREPPED);
    if (state & (1 << command))         RL_SETSTATE(RL_STATE_READCMD);
    if (state & (1 << meta))            RL_SETSTATE(RL_STATE_METANEXT);
    if (state & (1 << dispatching))     RL_SETSTATE(RL_STATE_DISPATCHING);
    if (state & (1 << more))            RL_SETSTATE(RL_STATE_MOREINPUT);
    if (state & (1 << incremental))     RL_SETSTATE(RL_STATE_ISEARCH);
    if (state & (1 << non_incremental)) RL_SETSTATE(RL_STATE_NSEARCH);
    if (state & (1 << search))          RL_SETSTATE(RL_STATE_SEARCH);
    if (state & (1 << numeric_arg))     RL_SETSTATE(RL_STATE_NUMERICARG);
    if (state & (1 << macro_input))     RL_SETSTATE(RL_STATE_MACROINPUT);
    if (state & (1 << macro_define))    RL_SETSTATE(RL_STATE_MACRODEF);
    if (state & (1 << overwrite))       RL_SETSTATE(RL_STATE_OVERWRITE);
    if (state & (1 << completing))      RL_SETSTATE(RL_STATE_COMPLETING);
    if (state & (1 << signal))          RL_SETSTATE(RL_STATE_SIGHANDLER);
    if (state & (1 << undo))            RL_SETSTATE(RL_STATE_UNDOING);
    if (state & (1 << pending))         RL_SETSTATE(RL_STATE_INPUTPENDING);
    if (state & (1 << finish))          RL_SETSTATE(RL_STATE_DONE);
  }

  //__________________________________________________________________
  template <typename Lock>
  inline int 
  basic_readline<Lock>::state(state_bit st, bool on) 
  {
    int bit = -1;
    switch(st) {
    case none:            bit = RL_STATE_NONE;            break ; 
    case initializing:    bit = RL_STATE_INITIALIZING;    break ; 
    case initialized:     bit = RL_STATE_INITIALIZED;     break ; 
    case terminal_preped: bit = RL_STATE_TERMPREPPED;     break ; 
    case command:         bit = RL_STATE_READCMD;         break ; 
    case meta:            bit = RL_STATE_METANEXT;        break ; 
    case dispatching:     bit = RL_STATE_DISPATCHING;     break ; 
    case more:            bit = RL_STATE_MOREINPUT;       break ; 
    case incremental:     bit = RL_STATE_ISEARCH;         break ; 
    case non_incremental: bit = RL_STATE_NSEARCH;         break ; 
    case search:          bit = RL_STATE_SEARCH;          break ; 
    case numeric_arg:     bit = RL_STATE_NUMERICARG;      break ; 
    case macro_input:     bit = RL_STATE_MACROINPUT;      break ; 
    case macro_define:    bit = RL_STATE_MACRODEF;        break ; 
    case overwrite:       bit = RL_STATE_OVERWRITE;       break ; 
    case completing:      bit = RL_STATE_COMPLETING;      break ; 
    case signal:          bit = RL_STATE_SIGHANDLER;      break ; 
    case undo:            bit = RL_STATE_UNDOING;         break ; 
    case pending:         bit = RL_STATE_INPUTPENDING;    break ; 
    case finish:          bit = RL_STATE_DONE;            break ; 
    }
    if (bit >= 0) {
      if (on) RL_SETSTATE(bit);
      else    RL_UNSETSTATE(bit);
    }
  
    return rl_readline_state;
  }

  //__________________________________________________________________
  template <typename Lock>
  inline bool
  basic_readline<Lock>::test_state(state_bit state) const
  {
    int ret = 0;
    switch(state) {
    case none:            ret = RL_ISSTATE(RL_STATE_NONE);            break ; 
    case initializing:    ret = RL_ISSTATE(RL_STATE_INITIALIZING);    break ; 
    case initialized:     ret = RL_ISSTATE(RL_STATE_INITIALIZED);     break ; 
    case terminal_preped: ret = RL_ISSTATE(RL_STATE_TERMPREPPED);     break ; 
    case command:         ret = RL_ISSTATE(RL_STATE_READCMD);         break ; 
    case meta:            ret = RL_ISSTATE(RL_STATE_METANEXT);        break ; 
    case dispatching:     ret = RL_ISSTATE(RL_STATE_DISPATCHING);     break ; 
    case more:            ret = RL_ISSTATE(RL_STATE_MOREINPUT);       break ; 
    case incremental:     ret = RL_ISSTATE(RL_STATE_ISEARCH);         break ; 
    case non_incremental: ret = RL_ISSTATE(RL_STATE_NSEARCH);         break ; 
    case search:          ret = RL_ISSTATE(RL_STATE_SEARCH);          break ; 
    case numeric_arg:     ret = RL_ISSTATE(RL_STATE_NUMERICARG);      break ; 
    case macro_input:     ret = RL_ISSTATE(RL_STATE_MACROINPUT);      break ; 
    case macro_define:    ret = RL_ISSTATE(RL_STATE_MACRODEF);        break ; 
    case overwrite:       ret = RL_ISSTATE(RL_STATE_OVERWRITE);       break ; 
    case completing:      ret = RL_ISSTATE(RL_STATE_COMPLETING);      break ; 
    case signal:          ret = RL_ISSTATE(RL_STATE_SIGHANDLER);      break ; 
    case undo:            ret = RL_ISSTATE(RL_STATE_UNDOING);         break ; 
    case pending:         ret = RL_ISSTATE(RL_STATE_INPUTPENDING);    break ; 
    case finish:          ret = RL_ISSTATE(RL_STATE_DONE);            break ; 
    }
    return ret == 0 ? false : true;
  }


  //__________________________________________________________________
  template <typename Lock>
  inline const std::string& 
  basic_readline<Lock>::name() const
  {
    return _name = rl_readline_name;
  }

  //__________________________________________________________________
  template <typename Lock>
  inline void
  basic_readline<Lock>::name(const std::string& name)
  {
    _name = name;
    rl_readline_name = _name.c_str();
  }

  //__________________________________________________________________
  template <typename Lock>
  inline const std::string& 
  basic_readline<Lock>::prompt() const
  {
    return std::string(rl_prompt);
  }

  //__________________________________________________________________
  template <typename Lock>
  inline void
  basic_readline<Lock>::prompt(const std::string& prompt)
  {
    rl_set_prompt(prompt.c_str());
  }

  //__________________________________________________________________
  template <typename Lock>
  inline void 
  basic_readline<Lock>::save_prompt() 
  {
    rl_save_prompt();
  }

  //__________________________________________________________________
  template <typename Lock>
  inline void 
  basic_readline<Lock>::restore_prompt() 
  {
    rl_restore_prompt();
  }

  //__________________________________________________________________
  template <typename Lock>
  inline void 
  basic_readline<Lock>::expand_prompt(const std::string& prompt) 
  {
    rl_expand_prompt(const_cast<char*>(prompt.c_str()));
  }

  //__________________________________________________________________
  template <typename Lock>
  inline void
  basic_readline<Lock>::already_prompted(bool already)
  {
    rl_already_prompted = (already ? 1 : 0);
  }

  //__________________________________________________________________
  template <typename Lock>
  inline bool 
  basic_readline<Lock>::already_prompted() const
  {
    return (rl_already_prompted == 0 ? false : true);
  }

  //__________________________________________________________________
  template <typename Lock>
  inline typename basic_readline<Lock>::edit_mode 
  basic_readline<Lock>::editing_mode() const
  {
    if (!rl_editing_mode) return vi;
    return emacs;
  }

  //__________________________________________________________________
  template <typename Lock>
  inline   void
  basic_readline<Lock>::editing_mode(edit_mode mode)
  {
    rl_editing_mode = int(mode);
  }

  //__________________________________________________________________
  template <typename Lock>
  inline bool 
  basic_readline<Lock>::clear_empty_lines() const
  {
    return (rl_erase_empty_line == 0 ? false : true);
  }

  //__________________________________________________________________
  template <typename Lock>
  inline void
  basic_readline<Lock>::clear_empty_lines(bool clear) 
  {
    rl_erase_empty_line = (clear ? 1 : 0);
  }

  //__________________________________________________________________
  template <typename Lock>
  inline int 
  basic_readline<Lock>::max_read() const 
  {
    return rl_num_chars_to_read;
  }

  //__________________________________________________________________
  template <typename Lock>
  inline void
  basic_readline<Lock>::max_read(int n) 
  {
    rl_num_chars_to_read = n;
  }

  //__________________________________________________________________
  template <typename Lock>
  inline void
  basic_readline<Lock>::parenthesis_timeout(int us) 
  {
    rl_set_paren_blink_timeout(us);
  }

  //__________________________________________________________________
  template <typename Lock>
  inline void 
  basic_readline<Lock>::done() 
  {
    rl_done = 1;
  }
  //__________________________________________________________________
  template <typename Lock>
  inline void 
  basic_readline<Lock>::parenthesis_blink_timeout(int us) 
  {
    _parenthesis_blink_timeout = us;
    rl_set_paren_blink_timeout(_parenthesis_blink_timeout);
  }

  //__________________________________________________________________
  template <typename Lock>
  inline int 
  basic_readline<Lock>::variable(const std::string& var, 
				 const std::string& val)
  {
    return rl_variable_bind(var.c_str(), val.c_str());
  }

  //__________________________________________________________________
  template <typename Lock>
  inline rlmm::command_t 
  basic_readline<Lock>::last_command()
  {
    return (command_t)rl_last_func;
  }

  //__________________________________________________________________
  template <typename Lock>
  inline void 
  basic_readline<Lock>::list_variables(bool inputrc) 
  {
    rl_variable_dumper(inputrc ? 1 : 0);
  }

  //__________________________________________________________________
  template <typename Lock>
  inline std::string 
  basic_readline<Lock>::version()
  {
    return std::string(rl_library_version);
  }

  //__________________________________________________________________
  template <typename Lock>
  inline bool 
  basic_readline<Lock>::gnu_readline()
  {
    return (rl_gnu_readline_p == 1 ? true : false);
  }
  //__________________________________________________________________
  /** @typedef readline 
      @brief Default instance of basic_readline
      @ingroup READLINE 
  */
  typedef basic_readline<single_thread> readline;
}


#endif
//____________________________________________________________________
//
// EOF
//
