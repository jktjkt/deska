//
// $Id: undo.hh,v 1.8 2005/08/13 19:51:31 cholm Exp $ 
//  
//  rlmm::undo
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
#ifndef rlmm_undo_hh
#define rlmm_undo_hh
#ifndef rlmm_util_hh
# include <rlmm/util.hh>
#endif 

namespace rlmm
{
  /** 
      @struct undo undo.hh <rlmm/undo.hh>
      @brief Utility class.
      Make an undo block.  Simple createing the object starts the
      group, and when the object is deleted (out of scope, or explicit
      delete) ends the group.  The static member function Doundo() can
      be used to do all the undoes - it returns true as long as
      there's something to be undone, so it can be used in a while
      loop. 
      @ingroup READLINE
  */
  class undo
  {
  private:
  public:
    /// What kind of undo to save via the Add member function
    enum what {
      /// undo a delete 
      remove, 
      /// undo an insert 
      insert
    };
    /// CTOR - start a group 
    undo() { begin(); }
    /// DTOR - end a group
    virtual ~undo() { end(); }
    /// Start a group 
    virtual void begin();
    /// End a group 
    virtual void end();
    /// Remember how to undo an event 
    virtual void add(what what, int start, int stop, 
		     const std::string& text);
    /// Signal that we're modifying some text between start and stop 
    virtual void modifying(int start, int stop);

    /// Free an undo list 
    static void clear();
    /// Do the head of an undo list, returns true if something done 
    static bool do_undo();
  };

  //___________________________________________________________________
  inline void
  undo::begin()
  {
    rl_begin_undo_group();
  }

  //____________________________________________________________________
  inline void
  undo::end()
  {
    rl_end_undo_group();
  }

  //____________________________________________________________________
  inline void
  undo::add(rlmm::undo::what what, int start, 
	    int stop, const std::string& text) 
  {
    undo_code uc;
    switch(what) {
    case remove: uc = UNDO_DELETE ; break;
    case insert: uc = UNDO_INSERT ; break;
    default: return;
    }
    rl_add_undo(uc, start, stop, const_cast<char*>(text.c_str()));
  }

  //____________________________________________________________________
  inline void
  undo::modifying(int start, int stop) 
  {
    rl_modifying(start, stop);
  }

  //____________________________________________________________________
  inline void
  undo::clear() 
  {
    rl_free_undo_list();
  }

  //____________________________________________________________________
  inline bool
  undo::do_undo() 
  {
    return rl_do_undo() == 0 ? false : true;
  }
}

#endif
//____________________________________________________________________
//
// EOF
//
