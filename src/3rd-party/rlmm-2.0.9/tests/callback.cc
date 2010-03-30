//
// $Id: callback.cc,v 1.4 2005/08/15 15:42:59 cholm Exp $ 
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
#ifndef __IOSTREAM__
# include <iostream>
#endif
#ifndef rlmm_readline_hh
# include <rlmm/readline.hh>
#endif
#ifndef rlmm_callback_hh
# include <rlmm/callback.hh>
#endif
#ifndef __CSTDIO__
# include <cstdio>
#endif
#ifdef HAVE_UNISTD_H
# ifndef UNISTD_H
#  include <unistd.h>
# endif
# ifndef SYS_TIME_H
#  include <sys/time.h>
# endif
# ifndef SYS_TYPES_H
#  include <sys/types.h>
# endif
#endif

/** @file   callback.cc
    @author Christian Holm
    @date   Mon Oct  7 13:57:07 2002
    @brief  Call back example. */

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
  class callback : public rlmm::callback
  {
  private:
    /// The prompt. 
    string _prompt;
  public:
    /** CTOR
	@param prompt The prompt to use. */
    callback(const string& prompt) : _prompt(prompt) { _prompt += "> "; }
    void read() 
    {
      // std::cout << "Read callback" << std::endl;
      rlmm::callback::read();
    }
    
    void handle(char* line) 
    {
      std::string l(line);
      if (l == "quit") { 
	std::cerr << "bye" << std::endl;
	exit(0);
      }
      std::cerr << "Line: " << line << std::endl;
    }
    /// Run this example. 
    int run() {
#ifndef  _POSIX_VERSION
      std::cerr << "Not a POSIX system" << std::endl;
      return 0;
#endif
      install(_prompt);
      fd_set set;
      int filedes = STDIN_FILENO;
      /* Initialize the file descriptor set. */
      FD_ZERO (&set);
      FD_SET (filedes, &set);

      do {
	// `select' returns 0 if timeout, 1 if input available, -1 if error. 
	int ret = select (FD_SETSIZE, &set, NULL, NULL, NULL);
	if      (ret < 0) return 1;
	else if (ret > 0) read();
	else    break;
      } while (true);
      return 0;
    }
  };
}

//____________________________________________________________________
int main(int argc, char** argv) 
{
  examples::callback c(argv[0]);
  return c.run();
}

//____________________________________________________________________
//
// EOF
//


