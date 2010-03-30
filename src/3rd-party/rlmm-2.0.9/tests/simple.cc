//
// $Id: simple.cc,v 1.3 2005/08/10 10:30:25 cholm Exp $ 
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
#ifndef rlmm_terminal_hh
# include <rlmm/terminal.hh>
#endif
#ifndef rlmm_readline_hh
# include <rlmm/readline.hh>
#endif
#ifndef rlmm_buffer_hh
# include <rlmm/buffer.hh>
#endif
#ifndef __IOSTREAM__
# include <iostream>
#endif
#ifndef __FSTREAM__
# include <fstream>
#endif
#ifndef __SSTREAM__
# include <sstream>
#endif
#ifndef __CSTDIO__
# include <cstdio>
#endif

/** @file   simple.cc
    @author Christian Holm
    @date   Mon Oct  7 13:57:07 2002
    @brief  Simple example using rlmm::readline. */

using namespace std;
using namespace rlmm;

//____________________________________________________________________
namespace examples 
{
  /** Simple example. 
      This example shows how to use hooks and other file desripters
      than the standard ones. */ 
  class simple : public rlmm::readline
  {
  private:
    /// The default input 
    string    _def;
    /// The file descriptor used for input
    string    _fd;
    /// MEximum number of characters to read. 
    int       _n;
    /// The prompt 
    string _prompt;
  public:
    /** CTOR
	@param argc from main. 
	@param argv from main. */
    simple(int argc, char** argv);
    /// DTOR.
    virtual ~simple() {}
    /// Hook run when starting up 
    virtual int startup_hook();
    /// show usage 
    void usage();
    /// main function
    int run();
  };
}

//____________________________________________________________________
examples::simple::simple(int argc, char** argv)
{
  _def  = "";
  _n    = 0;
  _fd   = "";
  name(argv[0]);
  _prompt = argv[0];
  
  for (int i = 1; i < argc; i++) {
    if (argv[i][0] != '-' || argv[i][1] == '\0' || !argv[i+1]) 
      continue;
    stringstream sarg(argv[i+1]);
    switch (argv[i][1]) {
    case 'p': sarg >> _prompt; break;
    case 'u': sarg >> _fd;     break;
    case 'd': sarg >> _def;    break;
    case 'n': sarg >> _n;      break;
    default: usage(); exit(2); break;
    }
    i++;
  }
}

//____________________________________________________________________
int examples::simple::run()
{

  if (!_fd.empty()) {
    FILE* fp = fopen(_fd.c_str(), "r");
    if (!fp) {
      stringstream ss;
      ss << name() << ": Bad file descriptor: " << _fd;
      terminal::active().print(ss.str());
      return 2;
    }
    terminal::active().instream(fp);
  }

  if (_n > 0) max_read(_n);

  _prompt.append("$ ");
  string line;
  if (!read(_prompt, line)) return 1;
  terminal::active().print(line);
  terminal::active().next_line();
  return 0;
}



//____________________________________________________________________
int examples::simple::startup_hook() 
{
  static bool first = false;
  if (first) return 0;
  first = true;
  buffer::instance().insert(_def);
  return 0;
}

//____________________________________________________________________
void examples::simple::usage() 
{
  stringstream ss;
  ss << "Usage: " << name() 
     << " [-p prompt] [-u unit] [-d default] [-n nchars]\n";
  terminal::active().print(ss.str()); 
  fflush(terminal::active().outstream());
}

//____________________________________________________________________
int main(int argc, char** argv) 
{
  examples::simple s(argc, argv);
  return s.run();
}

//____________________________________________________________________
//
// EOF
//

	 
	 
	 
	
	
	
	
    
