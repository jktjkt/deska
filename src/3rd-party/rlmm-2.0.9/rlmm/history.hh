//
// $Id: history.hh,v 1.14 2005/08/12 14:11:26 cholm Exp $ 
//  
//  rlmm::history
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
#ifndef rlmm_history_hh
#define rlmm_history_hh
#ifndef __LIST__
# include <list>
#endif
#ifndef rlmm_util
# include <rlmm/util.hh>
#endif
#ifndef __READLINE_HISTORY_H__
#include <readline/history.h>
#endif

namespace rlmm
{
  namespace 
  {
    //________________________________________________________________
    template <typename Lock>
    int 
    rlmm_inhibit_expansion(char* line, int where);
  }
  
  /** @struct basic_history history.hh <rlmm/history.hh>
      @brief history for readline.
      This class provides a history interface for readline. 
      Several history objects can exists at a given time.  One can
      switch between history objects by calling the static member
      function <tt>history::install</tt>.  The current active history
      object can be accessed via the static member function
      <tt>history::instance</tt>.  The first history object created
      will install itself as the current history. 

      @see examples::rlmmtest for how to use multiple history objects.
      @ingroup READLINE
  */
  template <typename Lock>
  class basic_history : public one_at_a_time<basic_history, Lock>
  {
  protected:
    /** Base classs is a friend */
    template <template <typename> class C, typename L>
    friend class one_at_a_time;
    // 
    friend int rlmm_inhibit_expansion<Lock>(char*, int);
    /** C API state type */
    typedef HISTORY_STATE* state_type;
    /** C API entry type */
    typedef HIST_ENTRY* entry_type;
  public:
    /** An entry in the history list
     */
    class entry 
    {
    private:
      entry_type  _entry;
      /** The text line */
      std::string _line;
      /** Time stamp */
      std::string _time;
      /** User data */
      void*       _data;
      template <typename L>
      friend class basic_history;
    public:
      /** Create an empty entry */
      entry() {}
      /** Create an entry from a C API entry */
      entry(entry_type e) : _entry(e) {
	if (!e) return;
	_line = e->line;
#if defined(RL_READLINE_VERSION) && (RL_READLINE_VERSION >= 0x0500)
	_time = e->timestamp;
#endif
	_data = e->data;
      }
      /** Create an entry 
	  @param line The text
	  @param time Time stamp
	  @param data User data
      */
      entry(const std::string& line, 
	    const std::string& time=std::string(), 
	    void* data=0)
	: _line(line), _time(time), _data(data)
      {
	_entry = (entry_type)malloc(sizeof(HIST_ENTRY));
	_entry->line = rlmm_string_duplicate(_line);
#if defined(RL_READLINE_VERSION) && (RL_READLINE_VERSION >= 0x0500)
	_entry->timestamp = rlmm_string_duplicate(_time);
#endif
	_entry->data = data;
      }
      
	
      /** Copy constructor */
      entry(const entry& e) 
	: _entry(e._entry), _line(e._line), _time(e._time), _data(e._data)
      {}
      /** Destructor */
      virtual ~entry() { }
      /** Assignment operator
	  @param e Entry to copy 
	  @return reference to this object.  */
      entry& operator=(const entry& e) 
      {
	if (_entry) free(_entry);
	_entry = e._entry;
	_line  = e._line;
	_time  = e._time;
	_data  = e._data;
	return *this;
      }
      /** Equalty operator 
	  @param e Entry to compare to
	  @return true if all fields are equal */
      bool operator==(const entry& e) 
      {
	return (e._line == _line && e._time == e._time && 
		(_entry && e._entry ? e._entry->_data 
		 == _entry->data : true));
      }
      const std::string& line() const { return _line; }
      const std::string& time() const { return _time; }
      const void*        data() const { return (_entry ? _entry->data : 0); }
    };
    /** The empty entry */
    static const entry no_entry;
    typedef std::list<entry> entry_list;
  protected:
    /// The history 
    entry_list _list;
    /// Internal state information. 
    mutable state_type _state; 
    /// keep 
    mutable state_type _keep;
    /// Flag telling us if history library has been initialised. 
    static bool _initialized;
    /// comment character 
    char _comment_character; 
    /// expension character 
    char _expansion_character;
    /// substitution character 
    char _substitution_character;
    /// Word delimiters 
    std::string _word_delimiters;
    /// no expand characters 
    std::string _no_expand_characters;
    /// Search delimiters 
    std::string _search_delimiters;
    /// wether quotes inhibits expansion 
    bool _quote_inhibit_expansion;
    /** Save the current state if this object is not the current
	history state */
    void save() const;
    /** Restore the previously saved state */
    void restore() const;
    struct saver 
    {
      const basic_history<Lock>& _history;
      saver(const basic_history<Lock>& h) : _history(h) { _history.save(); }
      ~saver() { _history.restore(); }
    };
    /// @name global manupulations
    /*@{*/
    /** Install a new history state. 
	At this point, the old history is synced with the current
	internal state.  */
    void do_activate();
    /** Remove a history state 
     */
    void do_deactivate();
    /*@}*/
    /** Inhibit expansion hook.  
	Users can override this method in derived classes. 
	@param line the line currently being expanded. 
	@param where index in line where expansion might take place. 
	@return true if expansion is to be done at index, false
	otherwise. */
    virtual bool inhibit_expansion(const std::string& line, int where);
  public:    
    /// Directions. 
    enum direction {
      /// Search backward in history list. 
      backward = -1,
      /// Search forward in history list. 
      forward = 1 
    };

    /// @name Creating and deleting
    /*@{*/
    /// CTOR.
    basic_history(); 
    /// DTOR.
    virtual ~basic_history() {}
    /*@}*/

    /// @name Setting/getting parameters
    /*@{*/
    /** Set the comment character (@c '#') */ 
    void comment_character(char c);
    /** Get the comment character */ 
    char comment_character() const { return _comment_character; }

    /** Set the expansion character. 
	@note that this changes the expansion character globally. 
	@param c The expansion character. (@c '!') */ 
    void expansion_character(char c);
    /** Return the expansion character */
    char expansion_character() const { return _expansion_character; }

    /** Set the characters that shouldn't be expanded. @c " \t\n\r=" */
    void no_expand_characters(const std::string& noexp);
    /** Get the characters that are not expanded. */
    const std::string& no_expand_characters() const 
    { 
      return _no_expand_characters; 
    }  

    /** Set wether quotes inhibit expansion. */
    void quote_inhibit_expansion(bool inhibit);
    /** Set wether quotes inhibit expansion. */
    bool quote_inhibit_expansion() const { return _quote_inhibit_expansion; }

    /** set the search delimiters. */
    void search_delimiters(const std::string& del);
    /** Get the search delimiters. */
    const std::string& search_delimiters() const { return _search_delimiters; }


    /** Set the substitution character. (@c '^')*/ 
    void substitution_character(char c);
    /** Get the substitution character. */ 
    char substitution_character() const { return _substitution_character; }

    /** Set the word delimiters. @c " \t\n()<>;&|" */
    void word_delimiters(const std::string& del);
    /** Get the word delimiters. */
    const std::string& word_delimiters() const { return _word_delimiters; }
    /*@}@

    /// @name Entry operations 
    /*@{*/
    /** Add a line to the history. 
	@param line Line to add. 
	@param data user data for line. */
    void add(const std::string& line, void* data=0);
    void add(const entry& e);
    /** Get entry at position. 
	@param index Index in history. 
	@return entry or 0 if out of bounds */
    entry at(int index) const;
    /** Return the number of bytes used by entries. 
	@return sum of bytes used by entries. */
    int bytes() const;
    /** Clear the history. */
    void clear();
    /** Get the current history entry */
    entry current() const;
    /** Get the length of the current history. */
    int length() const;
    /** Get the next entry. 
	@return The next entry. */ 
    entry next() const;
    /** Get the previous entry. 
	@return The next entry. */ 
    entry previous() const;
    /** Remove an entry from the history. 
	Remove one entry from the history.  The entry is returned, and
	it's the users responsibility to free the allocated memory in
	the returned entry.  Note, that the string is allocated using
	std::malloc and should therefor be deallocated with
	std::free. 
	@param index Index of line in history to remove.  
	@return The removed entry. */
    entry remove(int index);
    /** Replace a line in the history. 
	Replace a line in the history with a new element. The entry is
	returned, and it's the users responsibility to free the
	allocated memory in the returned entry.  Note, that the string
	is allocated using std::malloc and should therefor be
	deallocated with std::free. 
	@param index Line number to replace. 
	@param e the entry
	@return the old entry, or 0 if not valid. */
    entry replace(int index, const entry& e);
    /** Set the position in the history. 
	@param pos Position to go to. 
	@return true on success, false if pos is outout bounds. */
    bool position(int pos) ;
    /** Get the current offset in the history. 
	@return current offset. */
    int where() const;
    /** Get the history list.  Note, this cannot be modified. */
    const entry_list list() const { return _list; }
    
    /*@}*/

    /// @name Expansion of history 
    /*@{*/
    /** Return an event. 
	Returns the text of the history event at index in passed
	string. The index will be modified to point to after the event
	specifier.
	@param text Text where event specifier is in. 
	@param index Index of event specifier in text. 
	@param term Additional termination character. 
	@return the expanded event. */
    std::string event(const std::string& text, 
		      int& index, char term='\0');
    /** Do a history expansion on a line. 
	@param line Partial line to expand. 
	@param expansion The expansion. 
	@return status of the expansion:
	<ul>
	<li>-1: Error in expansion</li>
	<li>0:  No expansion.</li>
	<li>1:  Did expansion.</li>
	<li>2:  To be displayed</li>
	</ul>. */
    int expand(const std::string& line, std::string& expansion);
    /*@}*/


    /// @name searching in the history 
    /*@{*/
    /** Search for a line in the history. 
	Set the current history entry to the line if found. 
	@param line The line to search for.
	@param dir The direction of the search. 
	@param anchored String must match at start of a line.
	@return true if found. */
    bool search(const std::string& line, direction dir=forward, 
		bool anchored=false);
    /** Search for a line in the history. 
	Contrary to the non-position search, this does not change the
	current position in the history list. 
	@param line The line to search for.
	@param pos Line to start at. On return it contains the found
	location or -1 if no match was found. 
	@param dir The direction of the search. 
	@return true if found. */
    bool search(const std::string& line, int& pos, direction dir=forward);
    /*@}*/

    /// @name (Un)freezing of history size
    /*@{*/
    /** Test if the history is stiffled. 
	@return max number entries in history if it's stiffled,
	otherwise a negative value. */
    int stifle() const;
    /** Stiffle the history. 
	Set the maximum number of lines to remember in the history, or
	release the history.
	@param max The maximum number of lines.  If @p max is
	negative, the history is unstiffled.  
	@return max entries, or negative if it isn't stiffled. */
    int stifle(int max);
    /*@}*/

    /// @name History file manipulation
    /*@{*/
    /** Append lines to history file. 
	@param lines Number of lines to append. 
	@param filename File to append to. 
	@return 0 on success, errno otherwise. */
    int append(const std::string& filename, int lines);
    /** Read history from a file.
	@param filename file to read. 
	@param from Start at this entry if larger than zero. 
	@param to End at this entry if larger than zero. 
	@return 0 on success, errno otherwise. */
    int read(const std::string& filename, int from=-1, int to=-1);
    /** Truncate the history file.
	This will throw an Errno exception on failure. 
	@param filename File to truncate. 
	@param lines number lines in file after trnuncateion. */
    int truncate(const std::string& filename, int lines);
    /** Write history to file. 
	@param filename File to write to.
	@return 0 on success, errno otherwise. */
    int write(const std::string& filename);
    /*@}*/

    /// @name Miscellaneous operations
    /*@{*/
    /** Return arguments. 
	Return a string of positional arguments in the passed text. 
	@param first first argument to extract. 
	@param last last argument to extract. 
	@param text The line to extract from. 
	@return string with arguments seperated by . */
    std::string arguments(int first, int last, 
			  const std::string& text) const;
    /** Syncronise this object with current history */ 
    void sync();
    /** Tokenize a string according to history. 
	@param text Text to tokenize. */
    std::list<std::string> tokenize(const std::string& text) const;
    /*@}*/
  };

  namespace 
  {
    //________________________________________________________________
    template <typename Lock>
    inline int 
    rlmm_inhibit_expansion(char* line, int where) 
    {
      std::string l(line);
      return basic_history<Lock>::active().inhibit_expansion(l, where); 
    }
  }
  
  //__________________________________________________________________
  template <typename Lock>
  inline void 
  basic_history<Lock>::save() const
  {
    // Sae the current state of the history, but only if this object
    // is not the current history state - that's to avoid too many
    // calls to the C API
    _keep = 0;
    if (!this->is_active()) {
      _keep = _state;
      basic_history<Lock>::active().sync();
      history_set_history_state(_state);       
    }
  }
  //__________________________________________________________________
  template <typename Lock>
  inline void 
  basic_history<Lock>::restore() const
  {
    if (_keep) {
      history_set_history_state(_state);
      _keep = 0;
    }
  }

  //__________________________________________________________________
  template <typename Lock>
  const typename basic_history<Lock>::entry 
  basic_history<Lock>::no_entry("", "", 0);

  //__________________________________________________________________
  template <typename Lock>
  bool basic_history<Lock>::_initialized = false;

  //__________________________________________________________________
  template <typename Lock>
  basic_history<Lock>::basic_history()
    : _keep(0)
  {
    if (!_initialized) {
      using_history();
      history_inhibit_expansion_function = rlmm_inhibit_expansion<Lock>;
    }

    // _state = history_get_history_state();
    _state = state_type(calloc(1, sizeof(HISTORY_STATE)));
    _quote_inhibit_expansion  = (history_quotes_inhibit_expansion = 0 ? 
				 true : false);
    _comment_character        = history_comment_char;
    _expansion_character      = history_expansion_char;
    _substitution_character   = history_subst_char;
    _no_expand_characters     = (history_no_expand_chars == 0 || 
				 (history_no_expand_chars == (char*)NULL 
				  ? "" :  history_no_expand_chars));
    _search_delimiters        = (history_search_delimiter_chars == 0 ||  
				 history_search_delimiter_chars == (char*)NULL 
				 ? "" :  history_search_delimiter_chars);
    _word_delimiters          = ((history_word_delimiters == 0 ||  
				  history_word_delimiters == (char*)NULL 
				  ? "" :  history_word_delimiters));
  }

  //__________________________________________________________________
  template <typename Lock>
  inline void 
  basic_history<Lock>::do_deactivate() 
  {
    if (this->is_active()) sync();
  }
  
  //__________________________________________________________________
  template <typename Lock>
  inline void 
  basic_history<Lock>::do_activate() 
  {
    history_set_history_state(_state);
    // Set additional variables. 
    history_quotes_inhibit_expansion = _quote_inhibit_expansion ? 0 : 1;
    history_comment_char             = comment_character();
    history_expansion_char           = expansion_character();
    history_subst_char               = substitution_character();
    history_no_expand_chars          = 
      const_cast<char*>(no_expand_characters().c_str());
    history_search_delimiter_chars   = 
      const_cast<char*>(search_delimiters().c_str());
    history_word_delimiters          = 
      const_cast<char*>(word_delimiters().c_str());
  }

  //__________________________________________________________________
  template <typename Lock>
  inline void 
  basic_history<Lock>::quote_inhibit_expansion(bool inhibit) 
  {
    _quote_inhibit_expansion = inhibit;
    if (this->is_active()) 
      history_quotes_inhibit_expansion = !_quote_inhibit_expansion ? 0 : 1;
  }

  //__________________________________________________________________
  template <typename Lock>
  inline void 
  basic_history<Lock>::comment_character(char c) 
  {
    _comment_character = c;
    if (this->is_active()) 
      history_comment_char = c;
  }

  //__________________________________________________________________
  template <typename Lock>
  inline void
  basic_history<Lock>::expansion_character(char c) 
  {
    _expansion_character = c;
    if (this->is_active()) 
      history_expansion_char = c;
  }

  //__________________________________________________________________
  template <typename Lock>
  inline void
  basic_history<Lock>::no_expand_characters(const std::string& chars) 
  {
    _no_expand_characters = chars;
    if (this->is_active() && !chars.empty()) 
      strcpy(history_no_expand_chars, chars.c_str());
  }

  //__________________________________________________________________
  template <typename Lock>
  inline void
  basic_history<Lock>::search_delimiters(const std::string& del) 
  {
    _search_delimiters = del;
    if (this->is_active() && !del.empty()) 
      history_search_delimiter_chars = const_cast<char*>( del);
  }
  
  //__________________________________________________________________
  template <typename Lock>
  inline void
  basic_history<Lock>::substitution_character(char c)
  {
    _substitution_character = c;
    if (this->is_active())
      history_subst_char = c;
  }
 
  //__________________________________________________________________
  template <typename Lock>
  inline void
  basic_history<Lock>::word_delimiters(const std::string& del)
  {
    _word_delimiters = del;
    if (this->is_active() && !del.empty())
      history_word_delimiters = const_cast<char*>( del);
  }

  //__________________________________________________________________
  template <typename Lock>
  inline void
  basic_history<Lock>::add(const std::string& line, void* data) 
  {
    saver s(*this);
    add_history(line.c_str());
    entry_type e  = history_get(history_length-1);
    if (data) if (e) e->data = data;
    _list.push_back(e);
    sync();
  }
  //__________________________________________________________________
  template <typename Lock>
  inline void
  basic_history<Lock>::add(const entry& en) 
  {
    add(en.line(), en.data());
  }

  //____________________________________________________________________
  template <typename Lock>
  inline typename basic_history<Lock>::entry
  basic_history<Lock>::at(int where) const
  {
    saver s(*this);
    entry_type e = history_get(where);
    if (!e) return no_entry;
    for (typename entry_list::const_iterator i = _list.begin(); 
	 i != _list.end(); ++i) 
      if ((*i)._entry == e) return (*i);
    return no_entry;
  }
  
  //__________________________________________________________________
  template <typename Lock>
  inline int 
  basic_history<Lock>::bytes() const
  {
    saver s(*this);
    int ret = history_total_bytes();
    return ret;
  }
 
  //__________________________________________________________________
  template <typename Lock>
  inline void 
  basic_history<Lock>::clear()
  {
    saver s(*this);
    clear_history();
    sync();
  }

  //__________________________________________________________________
  template <typename Lock>
  inline typename basic_history<Lock>::entry
  basic_history<Lock>::current() const
  {
    saver s(*this);
    entry_type e = current_history();
    if (!e) return no_entry;
    return entry(e);
  }
  
  //__________________________________________________________________
  template <typename Lock>
  inline int 
  basic_history<Lock>::length() const
  {
    saver s(*this);
    int ret = history_length;
    return ret;
  }
  

  //__________________________________________________________________
  template <typename Lock>
  inline typename basic_history<Lock>::entry
  basic_history<Lock>::next() const
  {
    saver s(*this);
    entry_type e = next_history();
    if (!e) return no_entry;
    return entry(e);
  }

  //__________________________________________________________________
  template <typename Lock>
  inline typename basic_history<Lock>::entry
  basic_history<Lock>::previous() const
  {
    saver s(*this);
    entry_type e = previous_history();
    if (!e) return no_entry;
    return entry(e);
  }

  //__________________________________________________________________
  template <typename Lock>
  inline typename basic_history<Lock>::entry
  basic_history<Lock>::remove(int idx)
  {
    saver s(*this);
    entry_type e = remove_history(idx);
    if (!e) return no_entry;
    sync();
    return entry(e);
  }

  //__________________________________________________________________
  template <typename Lock>
  inline typename basic_history<Lock>::entry
  basic_history<Lock>::replace(int idx, const entry& n)
  {
    saver s(*this);
    entry_type e = replace_history_entry(idx, n.line().c_str(), n.data());
    if (!e) return no_entry;
    sync();
    return entry(e);
  }
 
  //__________________________________________________________________
  template <typename Lock>
  inline bool 
  basic_history<Lock>::position(int idx) 
  {
    saver s(*this);
    int ret =  history_set_pos(idx);
    sync();
    return ret == 1 ? true : false;
  }

  //__________________________________________________________________
  template <typename Lock>
  inline int 
  basic_history<Lock>::where() const
  {
    saver s(*this);
    int ret = where_history();
    return ret;
  }

  //__________________________________________________________________
  template <typename Lock>
  inline std::string 
  basic_history<Lock>::event(const std::string& line, int& index, char qchar) 
  {
    saver s(*this);
    char* ret = get_history_event(line.c_str(), &index, qchar);
    std::string out(ret);
    return out;
  }

  //__________________________________________________________________
  template <typename Lock>
  inline int 
  basic_history<Lock>::expand(const std::string& line, std::string& expansion)
  {
    saver s(*this);
    char* out = 0;
    int ret   = history_expand(const_cast<char*>(line.c_str()), &out);
    expansion = out;
    free(out);
    return ret;
  }


  //__________________________________________________________________
  template <typename Lock>
  inline bool
  basic_history<Lock>::search(const std::string& text, 
			      direction dir, 
			      bool anchored) 
  {
    saver s(*this);
    int ret = 0; 
    if (!anchored) ret = history_search(text.c_str(), dir);
    else           ret = history_search_prefix(text.c_str(), dir);
    return ret == -1 ? false : true;
  }

  //__________________________________________________________________
  template <typename Lock>
  inline bool 
  basic_history<Lock>::search(const std::string& text, int& pos,
			      direction dir)
  {
    saver s(*this);
    int ret = history_search_pos(text.c_str(), dir, pos);
    return ret == -1 ? false : true;
  }

  //====================================================================
  template <typename Lock>
  inline int
  basic_history<Lock>::stifle() const
  {
    saver s(*this);
    if (history_is_stifled() == 0) return -1;
    int ret = unstifle_history();
    stifle_history(ret);
    return ret;
  }

  //__________________________________________________________________
  template <typename Lock>
  inline int
  basic_history<Lock>::stifle(int max) 
  {
    saver s(*this);
    if (max < 0) max = unstifle_history();
    else         stifle_history(max);
    sync();
    return max;
  }

  //__________________________________________________________________
  template <typename Lock>
  inline int
  basic_history<Lock>::append(const std::string& file, int n) 
  {
    saver s(*this);
    int ret = append_history(n, file.empty() ? 0 : file.c_str());
    _list.erase();
    entry_type*   array = history_list();
    if (array)
      for (size_t i = 0; array[i]; i++) _list.push_back(entry(array[i]));
    return ret;

  }

  //__________________________________________________________________
  template <typename Lock>
  inline int 
  basic_history<Lock>::read(const std::string& filename, 
			    int from, int to) 
  {
    saver s(*this);
    int ret = 0;
    if (from < 0) ret = read_history(filename.c_str());
    else          ret = read_history_range(filename.c_str(), from, to);
    entry_type*   array = history_list();
    if (array)
      for (size_t i = 0; array[i]; i++) _list.push_back(entry(array[i]));
    sync();
    return ret;
  }

  //__________________________________________________________________
  template <typename Lock>
  inline int 
  basic_history<Lock>::truncate(const std::string& file, int n) 
  {
    saver s(*this);
    int ret = history_truncate_file(file.empty() ? 0 : file.c_str(), n);
    return ret == 0 ? true : false;
  }

  //__________________________________________________________________
  template <typename Lock>
  inline int 
  basic_history<Lock>::write(const std::string& file)
  {
    saver s(*this);
    int ret = write_history(file.empty() ? 0 : file.c_str());
    sync();
    return ret;
  }

  //__________________________________________________________________
  template <typename Lock>
  inline std::string
  basic_history<Lock>::arguments(int start, int stop, 
				 const std::string& text) const
  {
    saver s(*this);
    char* args = history_arg_extract(start, stop,text.c_str());
    std::string a = args == 0 || args == (char*)NULL ? "" : args;
    if (args) free(args);
    // sync();
    return a;
  }

  //__________________________________________________________________
  template <typename Lock>
  inline bool 
  basic_history<Lock>::inhibit_expansion(const std::string& line, int where) 
  {
    return false;
  }
  
  //__________________________________________________________________
  template <typename Lock>
  inline void 
  basic_history<Lock>::sync() 
  {
    if (_state) free(_state);
    _state = history_get_history_state();
  }


  //__________________________________________________________________
  template <typename Lock>
  inline std::list<std::string> 
  basic_history<Lock>::tokenize(const std::string& text) const
  {
    char** tokens = history_tokenize(text.c_str());
    char* token = tokens[0];
    std::list<std::string> l;
    while (token) {
      l.push_back(std::string(token));
      free(token);
      token++;
    }
    return l;
  }
  //__________________________________________________________________
  /** @typedef history 
      @brief Default history type 
      @ingroup READLINE
  */
  typedef basic_history<single_thread> history;
}

#endif
//____________________________________________________________________
//
// EOF
//
