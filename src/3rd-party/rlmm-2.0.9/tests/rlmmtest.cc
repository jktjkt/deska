//
// $Id: rlmmtest.cc,v 1.3 2005/08/10 10:30:25 cholm Exp $ 
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
#ifndef rlmm_history_hh
# include <rlmm/history.hh>
#endif
#ifndef rlmm_key_map_hh
# include <rlmm/key_map.hh>
#endif
#ifndef rlmm_function_map_hh
# include <rlmm/function_map.hh>
#endif
#ifndef __IOSTREAM__
# include <iostream>
#endif
#ifndef __IOMANIP__
# include <iomanip>
#endif
#ifndef __CSTDIO__
# include <cstdio>
#endif

/** @file   rlmmtest.cc
    @author Christian Holm
    @date   Mon Oct  7 13:57:07 2002
    @brief  Test of rlmm::readline. */

using namespace std;
using namespace rlmm;

//____________________________________________________________________
namespace examples 
{
  /** Test of rlmm::readline. 
      This example shows how to change histories in mid-stream. */
  class rlmmtest : public rlmm::readline 
  {
  private:
    typedef rlmm::readline base_type;
    /// First history. 
    rlmm::history _keeper1;
    /// Second history. 
    rlmm::history _keeper2;
    /// key map 
    rlmm::key_map _key_map;
    
    /// Line number when using the first history 
    int _line1;
    /// Line number when usign the second history 
    int _line2;
    /// Flag to set when done. 
    bool     _done;
    /// The prompt 
    string _prompt;
  public:
    /// CTOR.
    rlmmtest();
    /** Utility member function to make a prompt. 
	A occurence of a integer format specifier is substituted with
	the current history number.  E.g., if the @p skel parameter is
	@c "prompt [%d] " and the current history number is 10, then
	the @c %d will be replaced by the 10, and the prompt is 
	@c "prompt [10] ".  All normal integer format specifiers can
	be used e.g., if @p skel is @c "prompt [%03d]" and the current
	history line is 10, then the output will be 
	@c "prompt [010] ". 
	@param skel Skeleton for prompt. 
	@param num history number. 
	@return new prompt with substitutions. */
    string make_prompt(const string& skel, int num);
    /// Run this example.
    int run();
    /// Handle an input of @c "quit".
    void handle_quit() { _done = true; }
    /// Handle an input of @c "which".
    void handle_which();
    /// Handle an input of @c "switch".
    void handle_switch();
    /// Handle an input of @c "dump".
    void handle_dump() { key_map::active().list_bindings(); }
    /// Handle an input of @c "list".
    void handle_list();
    /// Handle an input of @c "help".
    void handle_help();
  };
}

//____________________________________________________________________
examples::rlmmtest::rlmmtest() 
{
  _prompt  = "readline [%d] ";
  _keeper2.read(".rlmmtest_history");
  _key_map.activate();
  _done    = false;
  _line1   = 0;
  _line2   = 0;
}

//____________________________________________________________________
string examples::rlmmtest::make_prompt(const string& skel, int num)
{
  static char str[16];
  size_t colon = skel.find('%');
  if (colon == string::npos) 
    return string(skel);
  
  size_t d = skel.find('d', colon);
  string format = skel.substr(colon, d+1);
      
  sprintf(str, format.c_str(), num);
      
  string p(skel);
  p.replace(colon, d+1, str);
  return p;
}

//____________________________________________________________________
int examples::rlmmtest::run() {
  string line;
  while (!_done) {
    int& lineno = _keeper1.is_active() ? _line1 : _line2;
    string p = make_prompt(_prompt, lineno);
    if (!read(p, line)) 
      return 0;
    
    size_t j = line.size();
    size_t i = line.size()-1;
    while (i >= 0) {
      if (line[i] != ' ' && line[i] != '\t') break;
      i--;
    }
    if (j != i+1) line.erase(i+1, j - i);
      
    if (line.empty()) 
      continue;
    
    lineno++;
    cerr << history::active().length() << ": `" << line << "'" <<  endl;
    history::active().add(line);

    if (line == "quit")   handle_quit();
    if (line == "which")  handle_which();
    if (line == "switch") handle_switch();
    if (line == "dump")   handle_dump();
    if (line == "list")   handle_list();
    if (line == "help")   handle_help();
  }
  return 0;
}

//____________________________________________________________________
void examples::rlmmtest::handle_help() 
{
  cerr << "Commands: \n\n" 
       << "\tquit\tQuit the application\n"
       << "\twhich\tShow which history is in use\n"
       << "\tswitch\tSwitch which history is in use\n"
       << "\tdump\tDump key bindings\n" 
       << "\tlist\tDump current history\n"
       << "\thelp\tThis help\n" << endl;
}

//____________________________________________________________________
void examples::rlmmtest::handle_list() 
{
  history::entry e;
  for (int i = 1; i <= history::active().length(); i++) {
    e = history::active().at(i);
    cerr << setw(4) << i << ": " << e.line() << endl;
  }
}

//____________________________________________________________________
void examples::rlmmtest::handle_switch() 
{
  if (_keeper1.is_active()) 
    _keeper2.activate();
  else 
    _keeper1.activate();
}

//____________________________________________________________________
void examples::rlmmtest::handle_which() 
{
  cout << "keeper is " 
       << (_keeper1.is_active() ? "1" : "2")  << endl;
}

//____________________________________________________________________
int main(int argc, char** argv) 
{
  examples::rlmmtest test;
  return test.run();
}

//____________________________________________________________________
//
// EOF
//


