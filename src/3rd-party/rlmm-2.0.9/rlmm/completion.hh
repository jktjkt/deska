//
// $Id: completion.hh,v 1.16 2005/08/15 15:42:59 cholm Exp $ 
//  
//  rlmm::basic_completion
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
#ifndef rlmm_completion_hh
#define rlmm_completion_hh
#ifndef __LIST__
# include <list>
#endif
#ifndef __STDEXCEPT__
# include <stdexcept>
#endif
#ifndef rlmm_util_hh
# include <rlmm/util.hh>
#endif
#ifndef rlmm_terminal_hh
# include <rlmm/terminal.hh>
#endif

namespace rlmm
{
  // Forward declarations 
  namespace 
  {
    template <typename Lock>
    char* 
    rlmm_completer(const char* what, int state);

    template <typename Lock>
    char** 
    rlmm_alternate_completer(const char* what, int start, int end);

    template <typename Lock>
    char* 
    rlmm_filename_quote(char* what, int match, char* quote);
    
    template <typename Lock>
    char* 
    rlmm_filename_dequote(char* what, int quote);

    template <typename Lock>
    int
    rlmm_quoted(char* what, int index);

    template <typename Lock>
    int
    rlmm_remove(char** matches);
    
    template <typename Lock>
    int
    rlmm_directory_hook(char** fullpath);

    template <typename Lock>
    void
    rlmm_display_hook(char** matches, int n, int w);
  }
  
  /** @struct basic_completion completion.hh <rlmm/completion.hh>
      @brief Class to do completions for readline. 
      readline completion. The application should sub-class this class
      for custom completion.  Several completer objects can exists.
      They should be installed as needed by sending the static message
      rlmm::completion::install. 
      @see examples::completer for an elaborate example. 
      @ingroup READLINE
  */
  template <typename Lock>
  class basic_completion : public one_at_a_time<basic_completion, Lock>
  {
  private:
    /** Base class is a friend */
    template <template <typename> class C, typename L>
    friend class one_at_a_time;
    // Glue functions are friends, so that the call backscan be
    // protected. 
    friend char*  rlmm_completer<Lock>(const char*, int);
    friend char** rlmm_alternate_completer<Lock>(const char*,int,int);
    friend char*  rlmm_filename_quote<Lock>(char*, int, char*);
    friend char*  rlmm_filename_dequote<Lock>(char*, int);
    friend int    rlmm_quoted<Lock>(char*, int);
    friend int    rlmm_remove<Lock>(char**);
    friend int    rlmm_directory_hook<Lock>(char**);
    friend void   rlmm_display_hook<Lock>(char**, int, int);
    /** Install a new completion handler. */
    void do_activate();
    /** Remove a completion handler. */
    void do_deactivate();    
  public:
    /** List of strings */
    typedef std::list<std::string> string_list;
    /// What kind of match.
    enum match {
      /// No match. 
      none, 
      /// Single match. 
      single, 
      /// Multiple matches. 
      multiple 
    };
    /// What kind of completion.
    enum type {
      /// Unknown. 
      unknown, 
      /// Normal (attempting) completion.
      normal, 
      /// List possible completions. 
      list, 
      /// List possible partial completions, if more than 0
      partial, 
      /// Insert all possible completions. 
      insert
    };
  protected:
    /// The state argument that completer was called with. 
    int _state;
    /// Maximum number of items to display before prompting.
    int _max_before_query;
    /// Characters that break words.
    std::string _word_breakers;
    /// Characters that quote. 
    std::string _quoters;
    /// Characters that forces quotation of a filename.
    std::string _filename_quoters;
    /// Word break characters to be left in match text.
    std::string _special_prefix;
    /// Append this character to the completions. 
    char _append_character;
    /// Wether to ignore duplicates or not
    bool _ignore_duplicates;
    /// Wether to inheibit completion or not 
    bool _inhibit;
    /** transform type enum to internal representation. 
	@param t the type to translate. 
	@return internal representation. */
    static int type2internal(type t);
    /** transform internal representation to type enum.
	@param t the internal type to translate. 
	@return class representation. */
    static type internal2type(int t);
    /** Call the internal completion function. 
	The actual action taken depends on the type argument.  This
	member function only really makes sense within the Completer, 
	Complete, and Possibilities member functions. 
	@param t The type of action to take. 
	@return true on success, falls otherwise. */
    virtual bool internal(type t); 
  protected:
    /** Abstract mmember function to be overloaded by user.  
	This should setup a list of possible matches to what.  A STL
	stack, deque, or similar is a good candidate.  
	@param what  What to setup a completion list for   */
    virtual void possibilities(const std::string& what) 
    {
      throw std::runtime_error("must overload possibilities");
    }    
    /** Member function that delegates to complete and possibilities. 
	@param what   What to complete
	@param state  First call (0) or later (>0) 
	@return next possible completion, or empty string if no more */
    virtual std::string completer(const std::string& what, int state);
    /** Abstract alternate completion to try first.  
	@param what What to make a completion list for
	@param start Pointer into buffer where completion starts 
	@param end Pointer into buffer where completion starts 
	@return list of possible completions (can be empty) */ 
    virtual string_list alternate_completer(const std::string& what,
						       int start, int end);
    /** Quote a file name. 
	@param filename File name to quote 
	@param m How many matches
	@param start Index into line buffer  
	@return the quoted file name */
    virtual std::string quote_filename(const std::string& filename, 
				       match m,
				       std::string::size_type& start);
    /** De-quote a file name. 
	@param filename File name to quote.
	@param start Index into line buffer. 
	@return the dequoated filename.  */
    virtual std::string dequote_filename(const std::string& filename, // 
					 std::string::size_type& start);
    /** Decide wether a character is quoted. 
	This method should be overloaded by the user.  If the
	character at the index postion in the passed string is quoted
	according to the quoting scheme, this should return true. 
	@param what The string. 
	@param where The character position. 
	@return true if character is quoted, false otherwise. */ 
    virtual bool quoted(const std::string& what, 
			std::string::size_type where) const;
    /** Remove matches from list of matches. 
	This member function is called with the a list of possible
        matches after completion.  The user can overload this member
        function to alter the list if some completions are
        undesirable.  
	@param substring Maximal substring common to all matches. 
	@param matches list of possible matches. 
	@return The true if the list was modified. */ 
    virtual bool remove_completions(const std::string& substring, 
				    string_list& matches);

    /** Hook for manipulating directory part of a file. 
	The method is called when completing a file name. The hook may
        change the directory part of it's argument. If it modifies it,
        it must return true, otherwise false.  It may not modify the
        filename part 
	@param dirname The directory part of the file.
	@param basename The file name of the file.
	@return true if the directory part is modified. */
    virtual bool directory_hook(std::string& dirname, 
				const std::string& basename);
    /** Method to display the completions. 
	This member function is called when readline is about to
        display a list of possible matches.  The sub-classes can
        redefine this method to do tricky stuff in the output. 
	@param matches A list of matches. 
	@param width Length of longest match. 
	@return false if internal displayer is to be used. */ 
    virtual bool display_hook(string_list& matches, 
			      int width);
    /** Abstract member function to be overloaded by user.  
	This should return a possible match to what, or the empty
	string if there's no (more) possible matches.  This member
	function is called repeatedly during completion of a single
	what.  If the member function Possilibities uses a STL
	container like stack (LIFO) or deque (FIFO), then this member
	function could simply pop the head of that container.
	@param what What to return a completion for 
	@return The next possible completion, or empty string if none */ 
    virtual std::string complete(const std::string& what)
    {
      throw std::runtime_error("must overload complete");
    }

  public: 
    /// CTOR
    basic_completion();
    /// DTOR
    virtual ~basic_completion();
    /** Return a list of possible matches. 
	First element is the replacement text (if any) for what,
	possibly what itself.  
	@param what What to get completions for 
	@return A list of possible completions.  First element is the 
	expansion of what. */  
    virtual string_list matches(const std::string& what);

    /** Complete a file name. 
	@param what  File name to complete 
	@return The next possible completion to partial file name */ 
    virtual std::string filename_complete(const std::string& what);
    /** Complete a user name. 
	@param what The partial user name to complete 
	@return The next possible completion to partial username */
    virtual std::string username_complete(const std::string& what);


    
    /** Set the maximum number of items before query. 
	This method sets the maximum number of items that the
	completion will show without querying the user if she wants to
	see the completions. 
	@param max The maximum. */
    virtual void max_before_query(int max=100);
    /**  the maximum number of items before query. 
	@return The maximum number of items. */
    virtual int max_before_query() { return _max_before_query; }

    /** Set Characters that break words.
        @param breakers Charaters that break words */
    virtual void word_breakers(const std::string& breakers);
    /** Get characters that break words.
        @return Characters that break words. */
    virtual const std::string& word_breakers() const; 

    /**  characters that quote. 
        @param quoters Characters tha can be used for quoting. */
    virtual void quoters(const std::string& quoters);
    /**   Characters that quote. 
        @return  Characters that quote.  */
    virtual const std::string& quoters() const { return _quoters; } 

    /**   Characters that forces quotation of a filename.
        @param quote Characters that forces quotation. */
    virtual void filename_quoters(const std::string& quote);
    /**   Characters that forces quotation of a filename.
        @return  Characters that forces quotation of a filename. */
    virtual const std::string& filename_quoters() const;

    /**  the word break characters to be left in match text.
	@param prefix The prefix characters. */
    virtual void special_prefix(const std::string& prefix);
    /**  the word break characters to be left in match text.
	@return The prefix characters. */
    virtual const std::string& special_prefix() const;

    /**  the character to the completions. 
	@param append the character to append. */ 
    virtual void append_character(char append=' ');
    /**  the character to the completions. 
	@return the character to append. */ 
    virtual char append_character() const { return _append_character; }

    /**  wether to ignore duplicates or not
        @param ignore true if to ignore  duplicates, false otherwise. */
    virtual void ignore_duplicates(bool ignore);
    /**  wether to ignore duplicates or not
        @return  true if to ignore  duplicates, false otherwise. */
    virtual bool ignore_duplicates() const { return _ignore_duplicates; } 
    /**  wether to inheibit completion or not 
        @param inhibit true if completion is inhibited, false otherwise. */
    virtual void inhibit(bool inhibit=false);
    /**  wether to ineibit completion or not 
        @return true if completion is inhibited, false otherwise. */
    virtual bool inhibit() const { return _inhibit; } 
    /** Flag the completions as filenames. 
	Sending this message tells readline, that the completion
        matches are to be treated as filenames.  This member function
        does not have any effect outside of the Complete member
        function.  
	@param isfiles true if completion treated as filenames. */
    virtual void filename_match(bool isfiles=true);
    /** Wether matches are filenames. 
	Note that this member function only makes sense in the
        Complete function.
	@return true if matches are flag as filenames. */ 
    virtual bool filename_match() const;
    /** Flag filename to be quoted. 
	Sending this message from the Complete member function will
        prompt readline to quote the filename using the QuoteFilename
        member function.  This message has no effect outside of the
        Complete function. 
	@param isquote true if filename is to be quoted. */ 
    virtual void quote_match(bool isquote=true);
    /** Wether mathces are to be quoted. 
	Note that this member function only makes sense in the
        Complete member function. 
	@return true if match is to be quoted. */ 
    virtual bool quote_match() const;
    /** Flag the completion as done. 
	Sending this message causes readline to stop doing
        completions.  This should be called from AlternativeComplete
        only - it has no effect elsewhere. 
	@param isover true if completion is over. */
    virtual void completion_done(bool isover=true);
    /** Wether the completion is flaged as over. 
	Note that this member function only makes sense in the
        AlternativeComplete member function. 
	@return wether completion is flaged as over. */ 
    virtual bool completion_done()  const;

    /**  the completion action type. 
	 the internal flag for what kind of completion is to be
        done.  
	@param t the type of action. */
    virtual void completion_type(type t);
    /**  the current completion type. 
	See the completionType(char) member function for a list of
	possible return values. 
	@return the current completion type. */
    virtual type completion_type(void) const;
  };

  namespace 
  {
    //__________________________________________________________________
    /** Interface function for completion
	@ingroup INTERFACE
	@param what What to complete 
	@param state The current state 
	@return return value of basic_completion<Lock>::completer(@a
	what, @a state) */ 
    template <typename Lock>
    inline char* 
    rlmm_completer(const char* what, int state) 
    {
      typedef basic_completion<Lock> comp_type;
      // We extract the string from the singleton object, and 
      if (what == (char*)NULL) what = 0;
      std::string comp = comp_type::active().completer(what, state);  
      // if the completion is empty, return NULL to readline, otherwise
      if (comp.empty()) return NULL;
      
      // we allocate a C string via malloc, as readline will want to
      // free it it self.  Return the malloc'ed C string 
      return rlmm_string_duplicate(comp);
    }

    //__________________________________________________________________
    /** Inteface function
	@ingroup INTERFACE
	@param what What to complete 
	@param start Start of range
	@param end end of range 
	@return return of
	basic_completion<Lock>::alternate_completer(@a what,@a
	start, @a end) */
    template <typename Lock>
    inline char** 
    rlmm_alternate_completer(const char* what, int start, int end) 
    {
      typedef basic_completion<Lock> comp_type;
      typedef typename comp_type::string_list string_list;
      string_list comps = 
	comp_type::active().alternate_completer(what,start,end);
      if (comps.empty()) return 0;

      // We need to allocate one more entry than there are completions so
      // that we can terminate the list by a zero. 
      size_t n       = comps.size();
      char** matches = (char**)malloc(sizeof(char*) * (n + 1));
      if (!matches) return 0;

      size_t i = 0;
      for (typename string_list::const_iterator iter = comps.begin(); 
	   iter != comps.end() && i < n; iter++, i++) {
	matches[i] = rlmm_string_duplicate(*iter);
      }
      matches[n] = 0;

      return matches;
    }

    //__________________________________________________________________
    /** Quite a file name 
	@ingroup INTERFACE
	@param what What to quite 
	@param match Kind of match 
	@param quote pointer to quote in @a what
	@return return value of
	basic_completion<Lock>::quote_filename(@a fn, @a match, @a quote)  */
    template <typename Lock>
    inline char* 
    rlmm_filename_quote(char* what, int match, char* quote) 
    {
      typedef  basic_completion<Lock> comp_type;
      std::string fn(what);
      std::string::size_type index = 0;
      while (index < fn.size()) {
	if (what[int(index)] == *quote) break;
      }
      std::string ret = 
	comp_type::active().quote_filename(fn, (match == SINGLE_MATCH ? 
						comp_type::single :
						comp_type::multiple), 
					   index); 
      return const_cast<char*>(ret.c_str());
    }

    //__________________________________________________________________
    /** Unquote a file name 
	@ingroup INTERFACE
	@param what What to dequote. 
	@param quote Pointer to quote in @a what
	@return return value of
    basic_completion<Lock>::dequote_filename(@a what, @a quote) */
    template <typename Lock>
    inline char* 
    rlmm_filename_dequote(char* what, int quote) 
    {
      typedef basic_completion<Lock> comp_type;
      size_t s = size_t(quote);
      std::string ret = 
	comp_type::active().dequote_filename(std::string(what), s);
      return const_cast<char*>(ret.c_str());
    }

    //__________________________________________________________________
    /** Check if @a what is quoted
	@ingroup INTERFACE 
	@param what String to check 
	@param index Index where quote might be
	@return basic_completion<Lock>::quoted(@a what, @a index) */
    template <typename Lock>
    inline int
    rlmm_quoted(char* what, int index) 
    {
      typedef basic_completion<Lock> comp_type;
      size_t s = size_t(index);  
      return comp_type::active().quoted(std::string(what), s) ? 1 : 0; 
    }

    //__________________________________________________________________
    /** Filter matches. 
	@ingroup INTERFACE 
	@param matches List of possible matches.  On output, the
	filtered list. 
	@return Number of items in @a matches on output. */
    template <typename Lock>
    inline int
    rlmm_remove(char** matches) 
    {
      typedef basic_completion<Lock> comp_type;
      typedef typename comp_type::string_list string_list;
      if (!matches || !matches[0]) return 0;
      string_list ml;
      char**                 mptr = matches;
      mptr++;

      while (*mptr && *mptr != (char*)NULL) {
	std::string s = *mptr; // cpy_c_string(s, *mptr);
	mptr++;
	ml.push_back(s);
      }
      size_t orig_size = ml.size();

      if (!comp_type::active().remove_completions(matches[0], ml))
	return orig_size;

      mptr = &matches[1];
      for (typename string_list::iterator next = ml.begin(); 
	   next != ml.end() && *mptr && *mptr != (char*)NULL; 
	   next++, mptr++) {
	if ((*next) == *mptr) continue;
	free(*mptr);
	*mptr = rlmm_string_duplicate(*next);
      }
      if (orig_size != ml.size()) {
	while(*mptr && *mptr != (char*)NULL) {
	  free(*mptr);
	  *mptr = (char*)NULL;
	  mptr++;
	}
      }
      return ml.size();
    }
  
    //__________________________________________________________________
    /** 
	@ingroup INTERFACE 
	@param fullpath filename.  On return, the full path. 
	@return
	basic_completion<Lock>::directory_hook(rlmm_basename(@a
	*fullpath), rlmm_dirname(@a *fullpath)) */
    template <typename Lock>
    inline int
    rlmm_directory_hook(char** fullpath) 
    {
      typedef basic_completion<Lock> comp_type;
      // Get the base and dir names. 
      const std::string bname = rlmm_basename<Lock>(*fullpath);
      std::string       dname = rlmm_dirname<Lock>(*fullpath);

      // Call the user routine 
      if (!comp_type::active().directory_hook(dname, bname)) return 0;

      // Free memeory  required by readline 
      free(*fullpath);
  
      // Make the new string 
      std::string fname(dname);
      // Append directory seperator if none
      if (dname[dname.size() - 1] != rlmm_directory_separator()) 
	fname += rlmm_directory_separator();
      // put in files basename
      fname.append(bname);
  
      // Allocate new space, and copy string
      *fullpath = rlmm_string_duplicate(fname);
  
      // Return true to indicate we modifed the string
      return 1;
    }
    //__________________________________________________________________
    /** Hook to run when displaying possible completions 
	@ingroup INTERFACE
	@param matches List of possible completions. 
	@param n Number of items in @a matches. 
	@param w The number of characters in the widest entry in @a
	matches. 
    */
    template <typename Lock>
    inline void
    rlmm_display_hook(char** matches, int n, int w) 
    {
      typedef basic_completion<Lock> comp_type;
      typedef basic_terminal<Lock>   term_type;
      typedef typename comp_type::string_list string_list;
      string_list ml;
      for (int i = 0; i < n; i++) ml.push_back(std::string(matches[i]));

      // We check the return value, so we don't have to allocate too much
      // memory for the list. 
      if (!comp_type::active().display_hook(ml, w)) {
	rl_display_match_list(matches, n, w);
	term_type::active().new_line(false);
      }
    }
  }
  
  
  //__________________________________________________________________
  template <typename Lock>
  inline 
  basic_completion<Lock>::basic_completion()
  {
    _max_before_query                   = 100;
    _word_breakers                      = rl_basic_word_break_characters;
    _quoters                            = rl_basic_quote_characters;
    _filename_quoters                   = "";
    _special_prefix                     = "";
    _append_character                   = ' ';
    _inhibit                            = false;
    _ignore_duplicates                  = true;
    rl_completion_entry_function        = rlmm_completer<Lock>;  
    rl_attempted_completion_function    = rlmm_alternate_completer<Lock>;
    rl_filename_quoting_function        = rlmm_filename_quote<Lock>;
    rl_filename_dequoting_function      = rlmm_filename_dequote<Lock>;
    rl_char_is_quoted_p                 = rlmm_quoted<Lock>;
    rl_ignore_some_completions_function = rlmm_remove<Lock>;
    rl_directory_completion_hook        = rlmm_directory_hook<Lock>;
    rl_completion_display_matches_hook  = rlmm_display_hook<Lock>;
  }
  //__________________________________________________________________
  template <typename Lock>
  inline void
  basic_completion<Lock>::do_activate()
  {
    rl_completion_query_items           = _max_before_query;
    rl_inhibit_completion               = (_inhibit ? 1 : 0);
    rl_completion_append_character      = _append_character;
    rl_ignore_completion_duplicates     = (_ignore_duplicates ? 1 : 0);
    rl_filename_quote_characters        = 
      const_cast<char*>(_filename_quoters.c_str());
    rl_completer_quote_characters      = const_cast<char*>(_quoters.c_str());
    rl_completer_word_break_characters = 
      const_cast<char*>(_word_breakers.c_str());
    rl_special_prefixes                = 
      const_cast<char*>(_special_prefix.c_str());
  }

  //__________________________________________________________________
  template <typename Lock>
  inline 
  basic_completion<Lock>::~basic_completion()
  {
    if (this->is_active()) this->deactivate();
  }

  //__________________________________________________________________
  template <typename Lock>
  inline void 
  basic_completion<Lock>::do_deactivate()
  {
    rl_completion_query_items           = 100;
    rl_inhibit_completion               = 1;
    rl_completion_append_character      = ' ';
    rl_completer_word_break_characters  = 
      const_cast<char*>(rl_basic_word_break_characters);
    rl_ignore_completion_duplicates     = 1;
    rl_completer_quote_characters       = rl_basic_quote_characters;
    rl_filename_quote_characters        = NULL;
    rl_special_prefixes                 = NULL;
  }

  //__________________________________________________________________
  template <typename Lock>
  inline std::string 
  basic_completion<Lock>::completer(const std::string& what, int state)
  {
    // If this is the first time we get here, call the user overloaded  
    // member function Possibilities to set up a table of possibilities 
    _state = state;
    if (_state == 0) possibilities(what);
    // We always return a possibility (or null) 
    return complete(what);
  }


  //__________________________________________________________________
  template <typename Lock>
  inline std::string 
  basic_completion<Lock>::filename_complete(const std::string& what) 
  {
    std::string s;
    char* c = rl_filename_completion_function(what.c_str(), _state);
    s = !c || c ==(char*)NULL ? "" : c;
    return s;
  }
  
  //__________________________________________________________________
  template <typename Lock>
  inline std::string 
  basic_completion<Lock>::username_complete(const std::string& what) 
  {
    std::string s;
    char* c = rl_username_completion_function(what.c_str(), _state);
    s =  c == 0 ||  c == (char*)NULL ? "" :  c;
    return s;
  }

  //__________________________________________________________________
  template <typename Lock>
  inline typename basic_completion<Lock>::string_list 
  basic_completion<Lock>::matches(const std::string& what) 
  {
    string_list matches; 
    char** rl_matches = 
      rl_completion_matches(what.c_str(), rlmm_completer<Lock>);

    // If there where no matches, return an empty list. 
    if (!rl_matches || !rl_matches[0]) 
      return matches;
  
    char** match = rl_matches;
    char*  old   = 0; 
    while (match && *match != (char*)NULL) {
      // Push a match onto the list
      std::string s;
      s =  *match == 0 ||  *match == (char*)NULL ? "" :  *match;
      matches.push_back(s);
      // Point to next 
      old = *(match++);
      // Free the memory 
      // free(old);
    }
  
    // free memory from readline 
    free(rl_matches);

    // Return the list 
    return matches;
  }

  //__________________________________________________________________
  template <typename Lock>
  inline typename basic_completion<Lock>::string_list 
  basic_completion<Lock>::alternate_completer(const std::string& what, 
					      int start, int stop) 
  {
    // Return an empty list 
    string_list l;
    return l;
  }

  //__________________________________________________________________
  template <typename Lock>
  inline void
  basic_completion<Lock>::max_before_query(int max) 
  {
    _max_before_query = max;
    if (this->is_active()) 
      rl_completion_query_items = _max_before_query;
  }

  //__________________________________________________________________
  template <typename Lock>
  inline void
  basic_completion<Lock>::word_breakers(const std::string& breakers)
  {
    _word_breakers = breakers;
    if (this->is_active()) 
      rl_completer_word_break_characters = 
	const_cast<char*>(_word_breakers.c_str());
  }
  //__________________________________________________________________
  template <typename Lock>
  inline void
  basic_completion<Lock>::quoters(const std::string& quoters)
  {
    _quoters = quoters;
    if (this->is_active()) 
      rl_completer_quote_characters = const_cast<char*>(_quoters.c_str());
  }

  //__________________________________________________________________
  template <typename Lock>
  inline void
  basic_completion<Lock>::filename_quoters(const std::string& quote)
  {
    _filename_quoters = quote;
    if (this->is_active()) 
      rl_filename_quote_characters = 
	const_cast<char*>(_filename_quoters.c_str());
  }

  //__________________________________________________________________
  template <typename Lock>
  inline void
  basic_completion<Lock>::special_prefix(const std::string& prefix) 
  {
    _special_prefix = prefix;
    if (this->is_active()) 
      rl_special_prefixes = const_cast<char*>(_special_prefix.c_str());
  }

  //__________________________________________________________________
  template <typename Lock>
  inline void
  basic_completion<Lock>::append_character(char append)
  {
    _append_character = append;
    if (this->is_active()) rl_completion_append_character = append;
  }

  //__________________________________________________________________
  template <typename Lock>
  inline void
  basic_completion<Lock>::ignore_duplicates(bool ignore)
  {
    _ignore_duplicates = ignore;
    if (this->is_active()) 
      rl_ignore_completion_duplicates = (_ignore_duplicates ? 1 : 0);
  }

  //__________________________________________________________________
  template <typename Lock>
  inline void
  basic_completion<Lock>::inhibit(bool inhibit)
  {
    _inhibit = inhibit;
    if (this->is_active()) rl_inhibit_completion = (_inhibit ? 1 : 0);
  }

  //__________________________________________________________________
  template <typename Lock>
  inline void
  basic_completion<Lock>::filename_match(bool isfiles)
  {
    if (this->is_active()) rl_filename_completion_desired = isfiles ? 1 : 0;
  }

  //__________________________________________________________________
  template <typename Lock>
  inline bool 
  basic_completion<Lock>::filename_match() const
  {
    return rl_filename_completion_desired == 0 ? false : true; 
  }

  //__________________________________________________________________
  template <typename Lock>
  inline void
  basic_completion<Lock>::quote_match(bool isquote)
  {
    if (this->is_active()) rl_filename_quoting_desired = isquote ? 1 : 0;
  }

  //__________________________________________________________________
  template <typename Lock>
  inline bool 
  basic_completion<Lock>::quote_match() const
  {
    return rl_filename_quoting_desired == 0 ? false : true; 
  }

  //__________________________________________________________________
  template <typename Lock>
  inline void 
  basic_completion<Lock>::completion_done(bool isover)
  {
    if (this->is_active()) 
      rl_attempted_completion_over = isover ? 1 : 0;
  }
  
  //__________________________________________________________________
  template <typename Lock>
  inline bool 
  basic_completion<Lock>::completion_done() const
  {
    return rl_attempted_completion_over == 0 ? false : true; 
  }
  

  //__________________________________________________________________
  template <typename Lock>
  inline void
  basic_completion<Lock>::completion_type(type t) 
  {
    if (this->is_active()) rl_completion_type = type2internal(t);
  }

  //__________________________________________________________________
  template <typename Lock>
  inline typename basic_completion<Lock>::type
  basic_completion<Lock>::completion_type() const
  {
    return internal2type(rl_completion_type);
  }

  //__________________________________________________________________
  template <typename Lock>
  inline bool 
  basic_completion<Lock>::internal(type t) 
  {
    return rl_complete_internal(type2internal(t)) == 0 ? true : false;
  }

  //____________________________________________________________________
  template <typename Lock>
  inline typename basic_completion<Lock>::type
  basic_completion<Lock>::internal2type(int t) 
  {
    switch(rl_completion_type) {
    case '\t': return normal;
    case '?':  return list;
    case '*':  return insert;
    case '!':  return partial;
    }  
    return unknown;
  }

  //____________________________________________________________________
  template <typename Lock>
  inline int 
  basic_completion<Lock>::type2internal(type t) 
  {
    switch(t) {
    case normal:  return '\t';
    case list:    return '?'; 
    case insert:  return '*';
    case partial: return '!';
    case unknown: return 0; 
    }
    return 0;
  }

  //__________________________________________________________________
  template <typename Lock>
  inline std::string 
  basic_completion<Lock>::quote_filename(const std::string& filename, 
					 match match, 
					 std::string::size_type& index)
  {
    return filename;
  }
  //__________________________________________________________________
  template <typename Lock>
  inline std::string 
  basic_completion<Lock>::dequote_filename(const std::string& filename, 
					   std::string::size_type& index) 
  {
    return filename;
  }

  //__________________________________________________________________
  template <typename Lock>
  inline bool 
  basic_completion<Lock>::quoted(const std::string& text, 
				 std::string::size_type idx) const 
  {
    return false;
  }
  //__________________________________________________________________
  template <typename Lock>
  inline const std::string& 
  basic_completion<Lock>::word_breakers() const 
  { 
    return _word_breakers;  
  } 
  //__________________________________________________________________
  template <typename Lock>
  inline const std::string& 
  basic_completion<Lock>::filename_quoters() const 
  { 
    return _filename_quoters;
  } 
  //__________________________________________________________________
  template <typename Lock>
  inline const std::string& 
  basic_completion<Lock>::special_prefix() const 
  {
    return _special_prefix; 
  }
  //____________________________________________________________________
  template <typename Lock>
  inline bool 
  basic_completion<Lock>::remove_completions(const std::string& substring, 
					     string_list& matches) 
  {
    return false;
  }
  //____________________________________________________________________
  template <typename Lock>
  inline bool 
  basic_completion<Lock>::directory_hook(std::string& dirname, 
					 const std::string& basename) 
  {
    return false;
  }

  //____________________________________________________________________
  template <typename Lock>
  inline bool 
  basic_completion<Lock>::display_hook(string_list& matches, 
				       int width) 
  {
    return false;
  }
  //____________________________________________________________________
  /** @typedef completion
      @brief Default instance of basic_completion
      @ingroup READLINE 
  */
  typedef basic_completion<single_thread> completion;
  
}

#endif
//____________________________________________________________________
//
// EOF
//
