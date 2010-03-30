//
// $Id: function_map.hh,v 1.12 2005/08/13 19:51:31 cholm Exp $ 
//  
//  rlmm::function_map
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
#ifndef rlmm_function_map_hh
#define rlmm_function_map_hh
#ifndef __LIST__
# include <list>
#endif
#ifndef rlmm_command_hh
# include <rlmm/command.hh>
#endif

/** @file   function_map.hh
    @author Christian Holm
    @date   Sat Oct  5 01:53:36 2002
    @brief  Map of currently defined functions. */

namespace rlmm
{
  /** @struct basic_function_map function_map.hh <rlmm/function_map.hh>
      @brief Map of currently defined functions. 
      @ingroup READLINE
  */
  template <typename Lock>
  class basic_function_map : public singleton<basic_function_map,Lock>
  {
  protected:
    /** Base class is a friend */
    template <template <typename> class C, typename L>
    friend class singleton;
    /** Constructor.  */
    basic_function_map();
    /** Constructor.  */
    basic_function_map(const basic_function_map& o) {}
    /** Constructor.  */
    basic_function_map& operator=(const basic_function_map& o) {return *this;}
  public:
    /** Destructor. */
    virtual ~basic_function_map() {  }

    /** get the named command.
	@param name Name to look up. 
	@return pointer to command wrapper function. */
    command_t get(const std::string& name);
    /** add the named command. 
	@param name the name of the command. 
	@param cmd The command function. 
	@return true on success, false on errors. */
    bool add(const std::string& name, command_t cmd);
    /** add the named command. 
	@param name the name of the command. 
	@return true on success, false on errors. */
    template<class T>
    bool add(const std::string& name);
    /** Get a list of defined function names. 
	@return list of defined function names. */
    std::list<std::string> name_list() const;
    /// Show a list of currently defined functions 
    void list();    
  };
  //__________________________________________________________________
  template<typename Lock> 
  template<class T> 
  inline bool 
  basic_function_map<Lock>::add(const std::string& name) 
  {
    return add(name, rlmm_command_wrapper<T>);
  }

  //__________________________________________________________________
  template <typename Lock>
  inline 
  basic_function_map<Lock>::basic_function_map()
  {}

  //__________________________________________________________________
  template <typename Lock>
  inline command_t
  basic_function_map<Lock>::get(const std::string& name) 
  {
    return (command_t)rl_named_function(name.c_str());
  }

  //__________________________________________________________________
  template <typename Lock>
  inline bool 
  basic_function_map<Lock>::add(const std::string& name, 
				command_t cmd) 
  {
    int ret = rl_add_funmap_entry(name.c_str(), (rl_command_func_t*)cmd);
    return (ret >= 0 ? true : false);
  }

  //__________________________________________________________________
  template <typename Lock>
  inline void 
  basic_function_map<Lock>::list() 
  {
    rl_list_funmap_names();
  }

  //__________________________________________________________________
  template <typename Lock>
  inline std::list<std::string> 
  basic_function_map<Lock>::name_list() const
  {
    const char** nl = rl_funmap_names();
    std::list<std::string> l; 

    const char* next = nl[0];
    while (next && next[0] != '\0')
      l.push_back(std::string(next++));

    free(nl);
    return l;
  }
  //__________________________________________________________________
  /** @typedef function_map 
      @brief Default function_map type 
      @ingroup READLINE
  */
  typedef basic_function_map<single_thread> function_map;
  
}

#endif
//____________________________________________________________________
//
// EOF
//
