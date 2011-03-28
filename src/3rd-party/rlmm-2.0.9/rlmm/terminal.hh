//
// $Id: terminal.hh,v 1.17 2005/08/13 19:51:31 cholm Exp $ 
//  
//  rlmm::Terminal
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
#ifndef rlmm_terminal_hh
#define rlmm_terminal_hh
#ifndef rlmm_util_hh
# include <rlmm/util.hh>
#endif

#include <stdio.h>

namespace rlmm
{
  namespace 
  {
    //__________________________________________________________________
    template <typename Lock> int  rlmm_read_character(FILE* in);
    template <typename Lock> void rlmm_redisplay();
    template <typename Lock> void rlmm_prepare(int eightbit);
    template <typename Lock> void rlmm_deprepare();          
  }
  /** @struct basic_terminal terminal.hh <rlmm/terminal.hh>
      @brief Terminal handling. 
      @ingroup READLINE

      @dontinclude simple.cc 
      @skip namespace examples 
      @until EOF
  */
  template <typename Lock>
  class basic_terminal : public one_at_a_time<basic_terminal, Lock>
  {
  protected:
    /** Base class is a friend */
    template <template <typename> class C, typename L>
    friend class one_at_a_time;
    friend int rlmm_read_character<Lock>(FILE*);
    friend void rlmm_redisplay<Lock>();		
    friend void rlmm_prepare<Lock>(int eightbit);
    friend void rlmm_deprepare<Lock>();          

    /** Constructor.  */
    basic_terminal();
    /** Constructor.  */
    basic_terminal(const basic_terminal& o);
    /** Constructor.  */
    basic_terminal& operator=(const basic_terminal& o);
    /** Install a new terminal handler. */
    void do_activate();
    /** Remove a terminal handler */
    void do_deactivate();

    /// 
    FILE* _input;
    ///
    FILE* _output;
    /// Terminal name 
    std::string _name;
    /** input time out in @f$ \mu s@f$ */
    int _input_timeout;
    /** Screen width in characters */
    int _width;
    /** Screen height in characters */
    int _height;

    /** Abstract method to be overloaded, read one character from
	passed file pointer. */
    virtual int  read_character();
    /// Prepare terminal for readline reads (overload). 
    virtual void prepare(bool eightbit=false);
    /// Undo last PrepareTerminal (overload).
    virtual void deprepare(); 
    /// Abstract method to be overloaded, redisplay the screen. 
    virtual void redisplay();
  public:
    /// DTOR.
    virtual ~basic_terminal();
    /// Reset the terminal to term.
    virtual void reset();
    /// Get the terminal name. 
    virtual const std::string& name() const;
    /// Set the terminal name.
    virtual void name(const std::string& name);

    /// Set the terminal size to default. 
    virtual void default_size();
    /// Set the size (in characters) of the terminal.
    virtual void resize(int width, int height);
    /// Get the size of the terminal. 
    virtual void size(int& width, int& height) const;
    /// Get the size of the terminal. 
    static void true_size(int& width, int& height);

    /** Set the input stream. 
	@param in A pointer to a C file stream. */
    virtual void instream(FILE* in);
    /// Get the input stream. 
    virtual FILE* instream() { return _input; }
    /// Set the output stream. 
    virtual void outstream(FILE* out);
    /// Get the output stream. 
    virtual FILE* outstream() { return _output; }

    /// Set the time out (in microseconds) before Input hook is exec'd. 
    virtual void input_timeout(int us);
    /// 
    virtual int input_timeout() const { return _input_timeout; }
    
    /// Read a character from input buffer.
    virtual int  read_key();
    /// Write a character to output, returns # of chars written.
    virtual int  write_character(int c);
    /// Write a message to output in `echo area'. 
    virtual void write_message(const std::string& message);
    /// Clear message area. 
    virtual void clear_message(); 
    /// Write on output stream 
    virtual void print(const std::string& txt);

    /// Force a redisplay. 
    virtual void  update();
    /** Signal a new line. 
	@param withPrompt If true, then it's assumed that the prompt
	has already been shown.  If false, then the prompt is
	reshown. This member function is a good candidate for begin
	the last thing to do in
	rlmm::completion::display_hook. */ 
    virtual void new_line(bool withPrompt=false);
    /// Jump to next line. 
    virtual void next_line();
    /// Redisplay the current line.
    virtual void reset_line();
    
    /// Ring the terminal bell, returns false if no echo. 
    virtual bool ding();
  };

  namespace 
  {
    //__________________________________________________________________
    /** Read one character from @a in
	@ingroup INTERFACE 
	@param in File descriptor to read from 
	@return basic_terminal<Lock>::read_character() */
    template <typename Lock>
    inline int 
    rlmm_read_character(FILE* in) 
    {
      return basic_terminal<Lock>::active().read_character();
    }

    //__________________________________________________________________
    /** Redisplay screen 
	@ingroup INTERFACE */
    template <typename Lock>
    inline  
    void 
    rlmm_redisplay() 
    {
      basic_terminal<Lock>::active().redisplay();
    }

    //__________________________________________________________________
    /** Prepare terminal 
	@ingroup INTERFACE
	@param eightbit Whether to use the meta bit */
    template <typename Lock>
    inline  
    void 
    rlmm_prepare(int eightbit) 
    {
      basic_terminal<Lock>::active().prepare(eightbit ? true : false);
    }
    
    //__________________________________________________________________
    /** Deprepare screen 
	@ingroup INTERFACE */
    template <typename Lock>
    inline  
    void 
    rlmm_deprepare() 
    {
      basic_terminal<Lock>::active().deprepare();
    }
  }
  
  //__________________________________________________________________
  template <typename Lock>
  inline
  basic_terminal<Lock>::basic_terminal()
  {
    _input_timeout          = 100;
    _name                   = (rl_terminal_name ? rl_terminal_name : "");
    true_size(_width, _height);
    rl_getc_function        = rlmm_read_character<Lock>;
    rl_redisplay_function   = rlmm_redisplay<Lock>;
    rl_prep_term_function   = rlmm_prepare<Lock>;
    rl_deprep_term_function = rlmm_deprepare<Lock>;
    _input                  = stdin;
    _output                 = stdout;
    rl_instream             = _input;
    rl_outstream            = _output;
  }

  //__________________________________________________________________
  template <typename Lock>
  inline
  basic_terminal<Lock>::basic_terminal(const basic_terminal& o)
    : _input(o._input), 
      _output(o._output),
      _name(o._name),
      _input_timeout(o._input_timeout),
      _width(o._width),
      _height(o._height)
  {}

  //__________________________________________________________________
  template <typename Lock>
  inline basic_terminal<Lock>&
  basic_terminal<Lock>::operator=(const basic_terminal& o)
  {
    _input	   = o._input; 
    _output	   = o._output;
    _name	   = o._name;
    _input_timeout = o._input_timeout;
    _width	   = o._width;
    _height	   = o._height;
    return *this;
  }
  
  //__________________________________________________________________
  template <typename Lock>
  inline void
  basic_terminal<Lock>::do_activate()
  {
    // prepare();
    rl_terminal_name        = (_name.empty() ? 0 : _name.c_str());
    rl_instream             = _input;
    rl_outstream            = _output;
    rl_set_keyboard_input_timeout(_input_timeout);
  }
  //__________________________________________________________________
  template <typename Lock>
  inline void
  basic_terminal<Lock>::do_deactivate()
  {
    // deprepare();
    rl_terminal_name        = 0;
    rl_instream             = stdin;
    rl_outstream            = stdout;
    rl_set_keyboard_input_timeout(100);
  }
  
  //__________________________________________________________________
  template <typename Lock>
  inline
  basic_terminal<Lock>::~basic_terminal()
  {
    if (this->is_active()) this->deactivate();
    deprepare();
  }

  //__________________________________________________________________
  template <typename Lock>
  inline
  void
  basic_terminal<Lock>::prepare(bool eightbit) 
  {
    if (this->is_active()) rl_prep_terminal(eightbit ? 1 : 0);
  }

  //__________________________________________________________________
  template <typename Lock>
  inline
  void
  basic_terminal<Lock>::deprepare() 
  {
    if (this->is_active()) rl_deprep_terminal();
  }

  //__________________________________________________________________
  template <typename Lock>
  inline
  void
  basic_terminal<Lock>::reset() 
  {
    if (this->is_active()) 
      rl_reset_terminal(_name.empty() ? 0 : _name.c_str());
  }

  //__________________________________________________________________
  template <typename Lock>
  inline
  void
  basic_terminal<Lock>::instream(FILE* in) 
  {
    _input = in;
    if (this->is_active()) rl_instream = _input;
  }

  //__________________________________________________________________
  template <typename Lock>
  inline
  void
  basic_terminal<Lock>::outstream(FILE* out) 
  {
    _output = out;
    if (this->is_active()) rl_outstream = _output;
  }

    //__________________________________________________________________
  template <typename Lock>
  inline
  void
  basic_terminal<Lock>::name(const std::string& name)
  {
    _name = name;
    if (this->is_active()) rl_terminal_name = name.c_str();
  }

  //__________________________________________________________________
  template <typename Lock>
  inline
  const std::string&
  basic_terminal<Lock>::name() const
  {
    return _name;
  }

  //__________________________________________________________________
  template <typename Lock>
  inline
  void
  basic_terminal<Lock>::input_timeout(int us) 
  {
    _input_timeout = us;
    if (this->is_active()) rl_set_keyboard_input_timeout(us);
  }

  //__________________________________________________________________
  template <typename Lock>
  inline
  void
  basic_terminal<Lock>::default_size() 
  {
    if (this->is_active()) rl_resize_terminal();
  }

  //__________________________________________________________________
  template <typename Lock>
  inline
  void
  basic_terminal<Lock>::resize(int width, int height) 
  {
    _width  = width;
    _height = height;
    if (this->is_active()) rl_set_screen_size(width, height);
  }

  //__________________________________________________________________
  template <typename Lock>
  inline
  void
  basic_terminal<Lock>::size(int& width, int& height) const
  {
    width  = _width;
    height = _height;
  }
  
  //__________________________________________________________________
  template <typename Lock>
  inline
  void
  basic_terminal<Lock>::true_size(int& width, int& height)
  {
    rl_get_screen_size(&width, &height);
  }

  //__________________________________________________________________
  template <typename Lock>
  inline
  int
  basic_terminal<Lock>::read_character() 
  {
    return rl_getc(_input);
  }

  //__________________________________________________________________
  template <typename Lock>
  inline
  int
  basic_terminal<Lock>::read_key() 
  {
    if (!this->is_active()) return -1;
    return rl_read_key();
  }

  //__________________________________________________________________
  template <typename Lock>
  inline
  int
  basic_terminal<Lock>::write_character(int c) 
  {
    if (!this->is_active()) return -1;
    return rl_show_char(c);
  }

  //__________________________________________________________________
  template <typename Lock>
  inline
  void
  basic_terminal<Lock>::print(const std::string& message) 
  {
    if (!this->is_active()) return;
    for (size_t i = 0; i < message.size(); i++) 
      write_character(int(message[i]));
  }
  
  //__________________________________________________________________
  template <typename Lock>
  inline
  void
  basic_terminal<Lock>::write_message(const std::string& message) 
  {
    if (!this->is_active()) return;
    // Make special case for FILE an ostream based input
    // #if defined(USE_VARARGS) && defined(PREFER_STDARG)
    rl_message(message.c_str());
    // #else 
    // rl_message();
    // #endif
  }

  //__________________________________________________________________
  template <typename Lock>
  inline
  void
  basic_terminal<Lock>::clear_message() 
  {
    if (!this->is_active()) return;
    rl_clear_message();
  }

  //__________________________________________________________________
  template <typename Lock>
  inline
  void
  basic_terminal<Lock>::redisplay() 
  {
    rl_redisplay();
  }

  //__________________________________________________________________
  template <typename Lock>
  inline
  void
  basic_terminal<Lock>::update() 
  {
    if (!this->is_active()) return;
    rl_forced_update_display();
  }

  //__________________________________________________________________
  template <typename Lock>
  inline
  void
  basic_terminal<Lock>::new_line(bool withPrompt) 
  {
    if (!this->is_active()) return;
    if (withPrompt) rl_on_new_line_with_prompt();
    else            rl_on_new_line();
  }

  //__________________________________________________________________
  template <typename Lock>
  inline
  void
  basic_terminal<Lock>::next_line() 
  {
    if (!this->is_active()) return;
    rl_crlf();
  }

  //__________________________________________________________________
  template <typename Lock>
  inline
  void
  basic_terminal<Lock>::reset_line() 
  {
    if (!this->is_active()) return;
    rl_reset_line_state();
  }

  //__________________________________________________________________
  template <typename Lock>
  inline
  bool
  basic_terminal<Lock>::ding()
  {
    if (!this->is_active()) return true;
    return rl_ding() == 0 ? true : false;
  }
  /** The default terminal class */
  typedef basic_terminal<single_thread> terminal;
}

#endif
//____________________________________________________________________
//
// EOF
//
