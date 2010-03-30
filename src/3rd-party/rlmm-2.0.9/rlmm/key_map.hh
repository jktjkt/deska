//
// $Id: key_map.hh,v 1.15 2005/08/17 16:33:51 cholm Exp $ 
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
#ifndef rlmm_key_map_hh
#define rlmm_key_map_hh
#ifndef __LIST__
# include <list>
#endif
#ifndef rlmm_command_hh
# include <rlmm/command.hh>
#endif

namespace rlmm 
{
  //__________________________________________________________________
  /** @struct basic_key_map key_map.hh <rlmm/key_map.hh>
      @brief key_map.
      class of key_maps 
      @ingroup READLINE
  */
  template <typename Lock>
  class basic_key_map : public one_at_a_time<basic_key_map, Lock>
  {
  protected:
    /** We befriend out base class */
    template <template <typename> class C, typename L>
    friend class one_at_a_time;
    /** List of key maps */
    typedef std::list<rlmm::basic_key_map<Lock>*> keymap_list;
    /** Set the current key_map. */
    void do_activate();
    /** Set the current key_map. */
    void do_deactivate() {}
    static void init();
    static bool _initialized;
  public:
    /** Iterator */
    typedef typename keymap_list::iterator      iterator;
    /** Constant iterator */
    typedef typename keymap_list::const_iterator const_iterator;
    
    /// Type of action.
    enum type {
      /// If it's a function.
      function, 
      /// If it's a macro.
      macro, 
      /// If it's a key_map.
      map
    };
  protected:
    /// List of key_maps. 
    static keymap_list* _keymaps;
    /** Find a key_map.
	@param m The internal key_map object that identifies the sought
	key_map. 
	@return a pointer to rlmm::key_map that contains the
	argument, or new key_map that contains the argument. */
    static iterator find_keymap(void* m);
    /// Pointer to internal readline struct
    ::Keymap _keymap;
  public:
    /// @name Basic operations
    /*@{*/ 
    basic_key_map();
    /// Create a new (possibly empty) key_map.
    basic_key_map(const std::string& name);
    /// Create a new (possibly empty) key_map.
    basic_key_map(bool empty);
    /// Copy a key_map. 
    basic_key_map(const basic_key_map<Lock>& km);
    /// Set a key_map from pointer. 
    basic_key_map(void* ptr);
    /// Destroy a key_map. 
    virtual ~basic_key_map();
    /// Get the name of the key_map.
    virtual std::string name() const;
    /*@}*/

    /// @name Manipluate the keymap list
    /*@{*/
    /// Find a key_map by name. 
    static iterator find_keymap(const std::string& name);
    /*@}*/

    /// @name Utility functions
    /*@{*/
    /// Return last key_map that had a binding. 
    static iterator last(); 
    /// Return name of currently executing macro, if any. 
    static std::string executing_macro();
    /// Return the key_map where current command is executing.
    static iterator executing_keymap();

    /// Make a C-@<key@> encoding. 
    static int ctrl(int key);
    /// Make a M-@<key@> encoding. 
    static int meta(int key);
    /// Make a C-M-@<key@> encoding. 
    static int ctrl_meta(int key) { return meta(ctrl(key)); }
    /*@}*/

    void set_to_system();
    void unset_from_system();

    /// @name Adding and removing bindings in the keymap
    /*@{*/
    /** Bind key to a command in this map. 
	@param key the key to bind. 
	@param cmd the command to bind. 
	@return true on success, false otherwise */
    virtual bool add(int key, command_t cmd);
    /** Bind a command to a key sequence in this map.    
	@param keyseq the key sequence to bind. 
	@param cmd the command to bind. 
	@return true on success, false otherwise */ 
    virtual bool add(const std::string& keyseq, command_t cmd);
    /** Bind a command to a key in this map.    
	The template argument should be a sub-class of
	rlmm::command. 
	@param key the key to bind. 
	@return true on success, false otherwise */
    template<class T> bool add(int key);
    /** Bind a command to a key sequence in this map.    
	The template argument should be a sub-class of
	rlmm::command. 
	@param keyseq the key sequence to bind. 
	@return true on success, false otherwise */
    template<class T> bool add(const std::string& keyseq);
    /** Bind a macro. 
	To invalidate a macro pass an empty string for the replacment
	text. 
	@param macro is the macro name. 
	@param replace is the replacment text. 
	@return true on success, false otherwise */
    virtual bool add_macro(const std::string& macro, 
			   const std::string& replace);
    /** Unbind a named function in this map.
	@param name the macro that should be unbound. 
	@return true on success, false otherwise */
    virtual bool remove(const std::string& name);
    /** Unbind a key in this map.    
	@param key The key to unbind. 
	@return true on success, false otherwise */
    virtual bool remove(int key);
    /** Unbind all keys bound to a command in this map.    
	@param cmd command which should have no keys bound.
	@return true on success, false otherwise */
    virtual bool remove(command_t cmd);
    /** Unbind all keys to a command in this map. 
	Template argument should be a sub-class of
	rlmm::commmand. 
	@return true on success, false otherwise */
    template<class T> bool remove();
    /*@}*/

    /// @name Information on keymap
    /*@{*/
    /** Get command invoked by key sequence. 
	@param keys The key sequence to find the command for. 
	@param type The type of the returend command. 
	@return a pointer to wrapper function, or 0 on failure. */
    virtual command_t get(const std::string& keys, type& type) const;
    /** Get a list of keysequences that invokes a command. 
	@param cmd The command to find the keysequences for. 
	@return list of keysequences (possibly empty). */ 
    virtual std::list<std::string> keys(command_t cmd) const; 
    /** Get a list of keysequences that invokes a command. 
	Template argument should be a sub-class of rlmm::command. 
	@return list of keysequences (possibly empty). */
    template<class T> std::list<std::string> keys() const;
    /** List all key-bindings in this map. 
	@param readable If true, then output in a format suitable for
	an INPUTRC file. */ 
    void list_bindings(bool readable=false);
    /** List all macrosin this map. 
	@param readable If true, then output in a format suitable for
	an INPUTRC file. */ 
    void list_macros(bool readable=false);
    /*@}*/
  };


  //====================================================================
  template <typename Lock> 
  typename basic_key_map<Lock>::keymap_list* basic_key_map<Lock>::_keymaps = 0;
  //====================================================================
  template <typename Lock> 
  bool basic_key_map<Lock>::_initialized = false;

  //____________________________________________________________________
  template <typename Lock>
  inline void
  basic_key_map<Lock>::init() 
  {
    if (!_initialized) {
      guard<Lock> g(basic_key_map<Lock>::_lock);
      if (!_initialized) {
	if (!_keymaps) _keymaps = new keymap_list;
      }
    }
  }
  //____________________________________________________________________
  template <typename Lock>
  inline typename basic_key_map<Lock>::iterator
  basic_key_map<Lock>::find_keymap(void* m) 
  {
    if (!m) return 0;
    for (iterator i = _keymaps->begin(); i != _keymaps->end(); i++) 
      if ((*i)->_keymap == m) return i;
    // Make a new key_map if none was found. 
    basic_key_map<Lock>* km = new basic_key_map<Lock>((void*)m);
    _keymaps->push_back(km);
    return km;
  }

  //==================================================================
  template <typename Lock>
  inline 
  basic_key_map<Lock>::basic_key_map(const std::string& name) 
  {
    init();
    ::Keymap ptr = rl_get_keymap_by_name(name.c_str());
    if (ptr) {
      _keymap = ptr;
      _keymaps->push_back(this);
    }
  }
  
  //__________________________________________________________________
  template <typename Lock>
  inline 
  basic_key_map<Lock>::basic_key_map() 
  {
    init();
    _keymap = rl_get_keymap();
    _keymaps->push_back(this);
  }
  
  //__________________________________________________________________
  template <typename Lock>
  inline 
  basic_key_map<Lock>::basic_key_map(bool empty) 
  {
    init();
    if (empty) _keymap = rl_make_bare_keymap();
    else       _keymap = rl_make_keymap();
    _keymaps->push_back(this);
  }

  //__________________________________________________________________
  template <typename Lock>
  inline 
  basic_key_map<Lock>::basic_key_map(const rlmm::basic_key_map<Lock>& km) 
  {
    init();
    _keymap = rl_copy_keymap(km._keymap);
    _keymaps->push_back(this);
  }

  //__________________________________________________________________
  template <typename Lock>
  inline 
  basic_key_map<Lock>::basic_key_map(void* ptr) 
  {
    init();
    _keymap = static_cast< ::Keymap>(ptr);
    _keymaps->push_back(this);
  }

  //__________________________________________________________________
  template <typename Lock>
  inline 
  basic_key_map<Lock>::~basic_key_map() 
  {
    for (iterator i = _keymaps->begin(); i != _keymaps->end(); i++) {
      if (*i == this) {
	_keymaps->erase(i);
	break;
      }
    }
    rl_discard_keymap(_keymap);
  }

  //__________________________________________________________________
  template <typename Lock>
  inline void 
  basic_key_map<Lock>::do_activate() 
  {
    rl_set_keymap(_keymap);
  }

    //__________________________________________________________________
  template <typename Lock>
  inline typename basic_key_map<Lock>::iterator
  basic_key_map<Lock>::find_keymap(const std::string& name)
  {
    ::Keymap m = rl_get_keymap_by_name(name.c_str());
    return find_keymap(m);
  }

    //__________________________________________________________________
  template <typename Lock>
  inline typename basic_key_map<Lock>::iterator
  basic_key_map<Lock>::last() 
  {
    return find_keymap(rl_binding_keymap);
  }

    //__________________________________________________________________
  template <typename Lock>
  inline std::string 
  basic_key_map<Lock>::executing_macro() 
  {
    return std::string(rl_executing_macro);
  }

    //__________________________________________________________________
  template <typename Lock>
  inline typename basic_key_map<Lock>::iterator
  basic_key_map<Lock>::executing_keymap() 
  {
    return find_keymap(rl_executing_keymap);
  }

  //__________________________________________________________________
  template <typename Lock>
  inline int 
  basic_key_map<Lock>::ctrl(int key) 
  {
    return CTRL(key);
  }

  //__________________________________________________________________
  template <typename Lock>
  inline int 
  basic_key_map<Lock>::meta(int key)
  {
    return META(key);
  }
  
  //__________________________________________________________________
  template <typename Lock>
  inline   std::string 
  basic_key_map<Lock>::name() const
  {
    return std::string(rl_get_keymap_name(_keymap));
  }

  //__________________________________________________________________
  template <typename Lock>
  inline bool 
  basic_key_map<Lock>::add_macro(const std::string& macro, 
				const std::string& replace) 
  {
    char* data = (replace.empty() ? 0 : (char*)replace.c_str());
    int ret = rl_generic_bind(ISMACR, macro.c_str(), data,
			      _keymap);
    return ret == 0 ? true : false; 
  }

  //__________________________________________________________________
  template <typename Lock>
  inline void
  basic_key_map<Lock>::set_to_system()
  {
    rl_tty_set_default_bindings(_keymap);
  }
  
  //__________________________________________________________________
  template <typename Lock>
  inline void
  basic_key_map<Lock>::unset_from_system()
  {
#if defined(RL_READLINE_VERSION) && (RL_READLINE_VERSION >= 0x0500)
    rl_tty_unset_default_bindings(_keymap);
#endif
  }

  //__________________________________________________________________
  template <typename Lock>
  inline bool 
  basic_key_map<Lock>::add(int key, command_t cmd)
  {
    int ret = rl_bind_key_in_map(key, cmd, _keymap);
    return ret == 0 ? true : false;
  }

  //__________________________________________________________________
  template <typename Lock>
  inline bool 
  basic_key_map<Lock>::add(const std::string& keyseq, command_t cmd)
  {
    int ret = rl_set_key(keyseq.c_str(), cmd, _keymap);
    return ret == 0 ? true : false;
  }

  //__________________________________________________________________
  template <typename Lock>
  inline bool 
  basic_key_map<Lock>::remove(const std::string& name)
  {
    int ret = rl_unbind_command_in_map(name.c_str(), _keymap);
    return ret == 0 ? true : false;
  }

  //__________________________________________________________________
  template <typename Lock>
  inline bool 
  basic_key_map<Lock>::remove(int key)
  {
    int ret = rl_unbind_key_in_map(key, _keymap);
    return ret == 0 ? true : false;
  }
  
  //__________________________________________________________________
  template <typename Lock>
  inline bool 
  basic_key_map<Lock>::remove(rlmm::command_t cmd)
  {
    int ret = rl_unbind_function_in_map(cmd, _keymap);
    return ret == 0 ? true : false;
  }

  //____________________________________________________________________
  template <typename Lock>
  inline command_t 
  basic_key_map<Lock>::get(const std::string& keys, 
			   type& ty)const
  {
    int t;
    command_t retval = 
      (command_t)rl_function_of_keyseq(keys.c_str(), 
				       _keymap, &t);
    switch(t) {
    case ISFUNC: ty = function; break;
    case ISMACR: ty = macro;    break;
    case ISKMAP: ty = map;      break;
    }
    return retval; 
  }

  //____________________________________________________________________
  template <typename Lock>
  inline std::list<std::string> 
  basic_key_map<Lock>::keys(command_t cmd) const
  {
    std::list<std::string> l;
    if (!cmd) 
      return l;
    char** seqs = rl_invoking_keyseqs_in_map((rl_command_func_t*)cmd,
					     _keymap); 
  
    char* seq = *seqs;
    char* old = 0;
    while (seq) {
      // add to the list
      l.push_back(std::string(seq));
      // Point to next 
      old = seq++;
      // Free the memory 
      free(old);
    }
    free(seqs);
    return l;
  }

  //__________________________________________________________________
  template <typename Lock>
  inline void 
  basic_key_map<Lock>::list_macros(bool inputrc)
  {
    if (this->is_active()) {
      rl_macro_dumper(inputrc ? 1 : 0);
      return;
    }
    basic_key_map<Lock>& old = this->activate();
    rl_macro_dumper(inputrc ? 1 : 0);
    old.activate();
  }
    
  //__________________________________________________________________
  template <typename Lock>
  inline void 
  basic_key_map<Lock>::list_bindings(bool inputrc)
  {
    if (this->is_active()) {
      rl_function_dumper(inputrc ? 1 : 0);
      return;
    }
    basic_key_map<Lock>& old = this->activate();
    rl_function_dumper(inputrc ? 1 : 0);
    old.activate();
  }

  //__________________________________________________________________
  template <typename Lock>
  template<class T> 
  inline bool 
  basic_key_map<Lock>::add(int key) 
  {
    return add(key, rlmm_command_wrapper<T>);
  }

  //__________________________________________________________________
  template <typename Lock>
  template<class T> 
  inline bool 
  basic_key_map<Lock>::add(const std::string& keyseq) 
  {
    return add(keyseq, rlmm_command_wrapper<T>);
  }

  //__________________________________________________________________
  template <typename Lock>
  template<class T> 
  inline bool 
  basic_key_map<Lock>::remove() 
  {
    return remove(rlmm_command_wrapper<T>);
  }

  //__________________________________________________________________
  template <typename Lock>
  template<class T>
  inline std::list<std::string> 
  basic_key_map<Lock>::keys() const
  {
    return get_keys(rlmm_command_wrapper<T>);
  }  

  //__________________________________________________________________
  /** @typedef key_map 
      @brief Default key_map type 
      @ingroup READLINE
  */
  typedef basic_key_map<single_thread> key_map;
}

#endif
//____________________________________________________________________
//
// EOF
//

