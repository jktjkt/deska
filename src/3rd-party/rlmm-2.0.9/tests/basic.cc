//
// $Id: basic.cc,v 1.2 2005/08/10 10:30:25 cholm Exp $ 
//  
//  simple.cc
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
#ifdef HAVE_CONFIG_H
# include <config.hh>
#endif
#ifndef rlmm_readline_hh
# include <rlmm/readline.hh>
#endif
#ifndef __IOSTREAM__
# include <iostream>
#endif

/** @file   rlmmtest.cc
    @author Christian Holm
    @date   Mon Oct  7 13:57:07 2002
    @brief  Very basic example. */

using namespace std;
using namespace rlmm;

//____________________________________________________________________
/** @namespace examples. 
    @brief Namespace for all the examples in rlmm. 

    All examples belong to this namespace (just to make it easier to
    see what is code and what is examples. */ 
namespace examples
{
  /// The (almost) most basic example of using rlmm. 
  class basic : public rlmm::readline 
  {
  private:
    /// The prompt. 
    string _prompt;
  public:
    /** CTOR
	@param prompt The prompt to use. */
    basic(const string& prompt) : _prompt(prompt) { _prompt += "> "; }
    /// Run this example. 
    int run() {
      string line;
      while(read(_prompt, line)) cout << line << endl;
      return 0;
    }
  };
}

//____________________________________________________________________
int main(int argc, char** argv) 
{
  examples::basic b(argv[0]);
  return b.run();
}

//____________________________________________________________________
//
// EOF
//


