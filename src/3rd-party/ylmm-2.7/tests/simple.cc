#ifndef simple_parser_hh
#include "simple_parser.hh"
#endif
#ifndef simple_scanner_hh
#include "simple_scanner.hh"
#endif
#ifndef YLMM_util
# include "util.hh"
#endif

/** @file   simple.cc
    @author Christian Holm
    @date   Tue Jan 28 01:46:50 2003
    @brief   */

/** Simple program to iluustrate @b Yacc/Lex--
    @param argc Number of command line argumetns from runtime system 
    @param argv Command line arguments from runtime system 
    @return 0 in case of success, non-zero on failure 
    @ingroup simple 
*/
int main(int argc, char** argv) 
{
  std::string file;
  getargs(argc, argv, file);
  
  if (argc > 1) return 0;
  simple_scanner s;
  simple_parser  p(s);
  return p.parse(0);
}

// 
// EOF
//
