//
// $Id: toycalc.cc,v 1.9 2003/06/29 11:29:35 cholm Exp $
//
//    toycalc.cc 
//    Copyright (C) 2002  Christian Holm <cholm@linux.HAL3000> 
// 
//    This library is free software; you can redistribute it and/or 
//    modify it under the terms of the GNU Lesser General Public 
//    License as published by the Free Software Foundation; either 
//    version 2.1 of the License, or (at your option) any later version. 
// 
//    This library is distributed in the hope that it will be useful, 
//    but WITHOUT ANY WARRANTY; without even the implied warranty of 
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
//    Lesser General Public License for more details. 
// 
//    You should have received a copy of the GNU Lesser General Public 
//    License along with this library; if not, write to the Free Software 
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
//    02111-1307  USA
/** @file   toycalc.cc
    @author Christian Holm
    @date   Sat Dec 07 20:14:22 2002
    @brief  Main program */
#ifndef toycalc_parser_hh
# include "toycalc_parser.hh"
#endif
#ifndef toycalc_scanner_hh
# include "toycalc_scanner.hh"
#endif
#ifndef __FSTREAM__
# include <fstream>
#endif
#ifndef YLMM_util
# include "util.hh"
#endif

/** A toy calculator
    @param argc Number of command line argumetns from runtime system 
    @param argv Command line arguments from runtime system 
    @return 0 in case of success, non-zero on failure 
    @ingroup toycalc 
*/
int main(int argc, char** argv) 
{
  // Get the command line arguments 
  std::string file;
  getargs(argc, argv, file);

  int retval = 0;
  try {
    // create a new scan buffer if a file name was passed to the program 
    ylmm::basic_buffer* input = 0;
    if (!file.empty()) input = new ylmm::basic_buffer(file,false);

    // Create the scanner and parser
    toycalc::scanner s(input);
    toycalc::parser  p(s);

    // Create the messenger and set it in the parser and scanner 
    ylmm::basic_messenger<ylmm::basic_lock> out;
    s.messenger(out);
    p.messenger(out);
    
    // p.debug(1);
    // Parse the input
    retval = p.parse(0);
  }
  catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 0;
  }
  return retval;
}

//
// EOF
//
