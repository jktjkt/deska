//
// $Id: streamer.cc,v 1.3 2005/08/10 10:30:25 cholm Exp $ 
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
#ifndef rlmm_streambuf_hh
# include <rlmm/streambuf.hh>
#endif
#ifndef __IOSTREAM__
# include <iostream>
#endif
#ifndef __IOMANIP__
# include <iomanip>
#endif

#if defined(__GNUC__) && __GNUC__ < 3
istream& noskipws(istream& ins) { ins.unsetf(ios::skipws); return ins; }
#endif
   
/** @file   streamer.cc
    @author Christian Holm
    @date   Mon Oct  7 13:57:07 2002
    @brief  Stream buffer example. */

using namespace std;
using namespace rlmm;

//____________________________________________________________________
namespace examples 
{
  /** Class tha shows the stream buffer layer. */
  class streamer 
  {
  private:
    /// Readline stream to read from.
    std::istream _in;
  public:
    /** CTOR. 
	Initialise our readline stream with a rlmm::streambuf
	object. */ 
    streamer() : _in(new rlmm::streambuf("streamer> ")) {}
    /// Do the character extraction example
    void do_char() { 
      char c; while((_in >> noskipws >> c)) cout << c; 
      cout << flush;
    }
    /// Do the string extraction example
    void do_string() { 
      string s; while((_in >> s)) cout << s << flush; }
    /// Do the single character get example
    void do_get() { 
      char g; 
      while((g = _in.get()) != EOF) cout << g;
      cout << flush; }
    /// Do the line reading example
    void do_line() { 
      string l; 
      while((getline(_in, l))) cout << l << endl; }
    /** Run this example. 
	@param argc from main. 
	@param argv from main. 
	@return always 0. */
    int run(int argc, char** argv) {
      char t = 'c';
      if (argc > 1 && argv[1][0] == '-') 
	t = argv[1][1];
	
      switch(t) {
      case 'c': do_char(); break;
      case 's': do_string(); break; 
      case 'g': do_get(); break;
      case 'l': do_line(); break; 
      default:
	cerr << "Unkown mode: '" << t << "'" << endl;
	return 1;
      }
      return 0;
    }
  };
}

//____________________________________________________________________
int main(int argc, char** argv) 
{
  examples::streamer s;
  return s.run(argc, argv);
}
//____________________________________________________________________
//
// EOF
//

