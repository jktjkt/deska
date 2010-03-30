//
// $Id: buffer.hh,v 1.11 2005/08/13 19:51:31 cholm Exp $ 
//  
//  rlmm::buffer
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
#ifndef rlmm_buffer_hh
#define rlmm_buffer_hh
#ifndef rlmm_util_hh
# include <rlmm/util.hh>
#endif

namespace rlmm
{
  /** @struct basic_buffer buffer.hh <rlmm/buffer.hh>
      @brief Buffer readline class. 
      interface to the readline buffer 
      @ingroup READLINE
  */
  template <typename Lock=single_thread>
  struct basic_buffer : public singleton<basic_buffer, Lock>
  {
  private: 
    /** base class is friend */
    template <template <typename> class C, typename L>
    friend class singleton;
    /** Guard type */
    typedef guard<Lock> guard_type;
    /** Constructor */
    basic_buffer() {}
    /** Constructor */
    basic_buffer(const basic_buffer& o) {}
    /** Assignment operator */
    basic_buffer& operator=(const basic_buffer& o) { return *this; }
  public:
    /** Return the internal buffer. 
	@return the internal buffer used. */
    char* start() 
    { 
      guard_type g(basic_buffer<Lock>::_lock); 
      return rl_line_buffer; 
    }
    /** Return the point in the buffer. 
	@return point in the buffer. */
    int point()   
    { 
      guard_type g(basic_buffer<Lock>::_lock); 
      return rl_point; 
    } 
    /** Return the end of the buffer. 
	@return pointer to end of the buffer. */
    int end()     
    { 
      guard_type g(basic_buffer<Lock>::_lock); 
      return rl_end; 
    } 
    /** Return the (possible) mark position in the buffer.
	@return the index of mark in buffer, 0 if no mark set. */
    int mark()    
    { 
      guard_type g(basic_buffer<Lock>::_lock); 
      return rl_mark; 
    } 
    /** Extend the buffer to hold at least more characters. 
	@param len the least lenght that the buffer should hold. */
    void extend(int len) 
    { 
      guard_type g(basic_buffer<Lock>::_lock); 
      rl_extend_line_buffer(len); 
    }
    /** Clear the line state.
	This member function can be called from signal handlers. */
    void clear() 
    { 
      guard_type g(basic_buffer<Lock>::_lock); 
      rl_free_line_state(); 
    }
    /** Inserts text at point.
	@param text to be inserted in buffer. 
	@return lenght of text inserted */
    int insert(const std::string& text) 
    { 
      guard_type g(basic_buffer<Lock>::_lock); 
      return rl_insert_text(text.c_str());
    }
    
    /** Stuff a character into the buffer.
	@param c character to be inserted at point in the buffer. 
	@return lenght of text inserted (0 on failure, 1 on success
	obviously) */
    int insert(int c) { 
      guard_type g(basic_buffer<Lock>::_lock); 
      return rl_stuff_char(c) != 1 ? 0 : 1;
    }
      
    /** Set the next character (command) to be read. 
	@param c the pending character. */
    void next(int c) 
    { 
      guard_type g(basic_buffer<Lock>::_lock); 
      rl_execute_next(c); 
    }
    /** Clear a previously set next. */
    void clear_next() { 
      guard_type g(basic_buffer<Lock>::_lock); 
      rl_clear_pending_input(); 
    }
    /** Delete text in buffer. 
	@param start Delete start here (inclusive).
	@param stop Delete ends here (exclusive). 
	@return number of characters deleted */
    int remove(int start, int stop) 
    { 
      guard_type g(basic_buffer<Lock>::_lock); 
      return rl_delete_text(start, end); 
    }
    
    /** Copy text from buffer. 
	@param start copy starts here (inclusive).
	@param stop copy ends here (exclusive). 
	@return the copied text. */
    std::string copy(int start, int stop)
    {
      guard_type g(basic_buffer<Lock>::_lock); 
      return std::string(rl_copy_text(start, end));
    }
    
    /** Delete text and put it in kill ring. 
	The text from start to stop is removed from the buffer and
	stored in kill-ring.  It can later be retrieved via a yank or
	similar. 
	@param start killing starts here (inclusive).
	@param stop killing end here (exclusive). */
    void kill(int start, int stop)
    {
      guard_type g(basic_buffer<Lock>::_lock); 
      rl_kill_text(start, end);      
    }    
  };
  //____________________________________________________________________
  /** @typedef buffer
      @brief Default instance of basic_buffer
      @ingroup READLINE 
  */
  typedef basic_buffer<single_thread> buffer;
}

#endif
//____________________________________________________________________
//
// EOF
//
