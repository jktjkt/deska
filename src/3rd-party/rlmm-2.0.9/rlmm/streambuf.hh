//
// $Id: streambuf.hh,v 1.10 2005/08/10 10:30:25 cholm Exp $ 
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
#ifndef rlmm_streambuf_hh
#define rlmm_streambuf_hh
#if defined(__GNUC__) && (__GNUC__ < 3)
# ifndef _STREAMBUF_H
#  include <streambuf.h>
# endif
#else
#ifndef __STREAMBUF__
# include <streambuf>
#endif
#endif
#ifndef __CASSERT__
# include <cassert>
#endif
#ifndef rlmm_readline_hh
# include <rlmm/readline.hh>
#endif
#ifndef rlmm_history_hh
# include <rlmm/history.hh>
#endif

/** @file   streambuf.hh
    @author Christian Holm
    @date   Mon Oct  7 23:18:23 2002
    @brief  Stream buffer reading from readline. */
namespace rlmm {
  /** @struct basic_streambuf streambuf.hh <rlmm/streambuf.hh>
      @brief Stream buffer reading from readline.
      Using this stream buffer layer, you can use a standard
      std::istream to read lines from the via readline.  
      
      Internally, the buffer uses the global instance of
      rlmm::readline, as it really makes the code simpler.  

      @note This stream buffer @e always returns the newline from
      readline.  If it didn't, you'd never know when one line start
      and one ends.  This requries a bit more bookkeeping but is worth
      the effort. 

      @par Example
      Below is shown an example of using this class template. 

      @dontinclude streamer.cc 
      @skip namespace examples
      @until EOF

  */
  template <typename Lock=single_thread>
  class basic_streambuf : public std::streambuf
  {
  public:
    typedef basic_readline<Lock> readline_type;
#if defined(__GNUC__) && __GNUC__ < 3
    // The type of the buffers 
    typedef char char_type;
    // Integer type
    typedef int  int_type;
#endif
  protected:
    /// Whether to use history
    bool _use_history;
    /// The prompt to use
    std::string _prompt;
    /// The line read from readline (the sequence).
    std::string _line;
    /** Current pending point in buffer
	Pointer into sequence, marking the start of avaliable 
	characters. */
    std::string::iterator _current;
    /** Read a line via readline and store it.  
	This member uses the C library @c readline function to read a
	line from the input. 
	@return true if a non-empty line was read, false otherwise. */
    virtual bool internal_read();
    /** Put back a character in the buffer.  
	If we're at the begining of line read, expand the buffer at
	start and insert @p c there; otherwise, simply back up one
	character in the read line. 
	@param c the character to put back. 
	@return EOF in case of errors. @p c otherwise. */
    virtual int_type pbackfail(int_type c=EOF);
    /** Return the number of characters avaliable in internal buffer. 
	@return # of characters avaliable in input sequence. */
    virtual int_type showmanyc() const { return _line.end() - _current; }
    /** Return next character from input sequence. 
	Returns next character from pending sequence and pushes it
	back to on read sequence. 
	@return pending character. */
    virtual int_type uflow();
    /** Return next pending character. 
	If the sequence is empty, read a new line from terminal.  If
	this again results in an empty line, repeat.
	@return pending character in sequence. */
    virtual int_type underflow();
    /** Get a number of characters from sequence. 
	Get a block of characters from the input sequence, and put
	them in the passed buffer @c buf which is of size @c n.  The
	maximum of @c n or # of pending characters are read into the
	passed buffer.  The iterators are updated accordingly. 
	@param buf the buffer to read into. 
	@param n the size of the buffer @c buf.
	@return # of copied characters.  */
    virtual std::streamsize xsgetn(char_type* buf, std::streamsize n);
  public:
    /** Constructor.
	Allocates a new stream buffer reading via readline. 
	@param prompt The prompt to display. 
	@param use_history Whether to use history
	@param buf externally allocated buffer for buffered reads. If
	this is zero, the reading will not be buffered. 
	@param n Size of the externally allocated buffer. */
    basic_streambuf(const std::string& prompt=std::string(), 
		    bool use_history=true,
		    char* buf=0, std::streamsize n=0);
    /** Destructor */
    virtual ~basic_streambuf() {}
    /** Set the prompt. 
	@param p The prompt to use. */
    virtual std::string& prompt(const std::string& p) { return _prompt = p; }
    /** Get the prompt. */
    virtual const std::string& prompt() const { return _prompt;}
  };


  //____________________________________________________________________
  template <typename Lock>
  inline 
  basic_streambuf<Lock>::basic_streambuf(const std::string& prompt, 
					 bool use_history,
					 char* buf, std::streamsize n)
    : _use_history(use_history), 
      _prompt(prompt), _line(""), _current(_line.begin())
  {
    setbuf(buf, n);
  }

  //____________________________________________________________________
  template <typename Lock>
  inline bool 
  basic_streambuf<Lock>::internal_read() 
  {
    // Reset the iterators. 
    _current = _line.begin();
    // Read a line, and return false if nothing was read.  
    if (!readline_type::instance().read(_prompt,_line))
      return false; 
    // If the read line isn't empty, append a new line character. 
    if (!_line.empty()) {
      if (_use_history) basic_history<Lock>::active().add(_line);
      _line.push_back('\n');
    }
    // Set high-water mark to point at end of read line
    _current  = _line.begin();
    // return an ok
    return true;
  }

  //____________________________________________________________________
  template <typename Lock>
  inline typename basic_streambuf<Lock>::int_type 
  basic_streambuf<Lock>::pbackfail(int_type c) 
  {
    if (_current != _line.begin()) 
      _current--;
    else if (c != EOF) {
      if (!_line.empty()) {
	std::string tmp(_line);
	_line[0] = char(c);
	_line.replace(1, tmp.size(), tmp);
      }
      else {
	assert(_line.empty());
	_line[0] = char(c);
      }
      _current = _line.begin();
    }
    return c;
  }
  
  //____________________________________________________________________
  template <typename Lock>
  inline typename basic_streambuf<Lock>::int_type 
  basic_streambuf<Lock>::uflow() 
  {
    int_type c = underflow();
    if (c != EOF) _current++;
    return c;
  }
  
  //____________________________________________________________________
  template <typename Lock>
  inline typename basic_streambuf<Lock>::int_type 
  basic_streambuf<Lock>::underflow() 
  {
    if (_current == _line.end() && !internal_read())
      return EOF;
    if (_current < _line.end()) return *_current; 
    else if (_line.empty()) {
      // Empty line read, read a new one. 
      return underflow(); 
    }
    return EOF; 
  }

  //____________________________________________________________________
  template <typename Lock>
  inline std::streamsize
  basic_streambuf<Lock>::xsgetn(char_type* buf, std::streamsize n) 
  {
    // Figure out the maximum ...
    int rd = n > showmanyc() ? showmanyc() : n;
    // ... and copy that number of characters to the passed buffer. 
#if defined(__GNUC__) && __GNUC__ < 3
    memcpy(buf, _current, rd);
#else
    _line.copy(buf, rd, _line.begin()-_current);
#endif
    _current += rd;
    // if we still have some room to read into, so we ask for another 
    // line. 
    if (rd < n && internal_read())
      rd += xsgetn(buf + rd, n - rd);
    return rd;
  }    
  //____________________________________________________________________
  /** @typedef streambuf
      @brief Default instance of basic_streambuf
      @ingroup READLINE 
  */
  typedef basic_streambuf<single_thread> streambuf;
  
}

#endif
//____________________________________________________________________
//
// EOF
//
