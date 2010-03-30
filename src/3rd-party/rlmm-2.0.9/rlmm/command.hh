//
// $Id: command.hh,v 1.8 2005/08/13 19:51:31 cholm Exp $ 
//  
//  rlmm::command
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
#ifndef rlmm_command_hh
#define rlmm_command_hh
#ifndef rlmm_util_hh
# include <rlmm/util.hh>
#endif

namespace rlmm 
{
  //__________________________________________________________________
  /** Command_t.
      Type of commands (a C function */ 
  typedef int (*command_t)(int,int);

  //__________________________________________________________________
  /** @struct command command.hh <rlmm/command.hh>
      @brief Command.
      Abstract base class for commands 
      @ingroup READLINE
  */
  class command 
  {
  public:
    /// Destructor 
    virtual ~command() {}
    /// true if user gave explict argument.
    bool have_explicit() const;
    /// The numeric explicit argument. 
    int  argument() const;
    /// true if called via readlines key_mapping.
    bool dispatched() const;
    /// Abstract member function to be overloaded. 
    virtual int  operator()(int count, int key) = 0;
  };
  //__________________________________________________________________
  /** Wrapper function to call command::operator()(int,int). 
      Note, that an object of type @a T is created, and the member
      function operator()(int,int) called on that object, after which
      it is deleted.   That means, that @a T can not use the internal
      state in that member function, unless it's a static object or
      state. 
      @todo Perhaps this needn't use a specific base class (command),
      but only insist that the member function operator()(int,int)
      exists, and perhaps we can create the object in the stack too. 
      @ingroup INTERFACE 
      @param count 
      @param key 
      @return return value of T::operator(@a count, @a key) */
  template<class T> 
  inline int 
  rlmm_command_wrapper(int count, int key) 
  {
    command* cmd = new T;
    int retval = cmd->operator()(count, key);
    delete cmd;
    return retval;
  }

  //__________________________________________________________________
  inline bool
  command::have_explicit() const
  {
    return (rl_explicit_arg == 0 ? false : true);
  }
  //__________________________________________________________________
  inline int
  command::argument() const
  {
    return rl_numeric_arg;
  }
  //__________________________________________________________________
  inline bool
  command::dispatched() const
  {
    return (rl_dispatching == 0 ? false : true);
  }
}

#endif
//____________________________________________________________________
//
// EOF
//
