//
// $Id: util.hh,v 1.2 2003/06/29 11:29:35 cholm Exp $ 
//  
//  util
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
#ifndef YLMM_util
#define YLMM_util

/** @file   util.hh
    @author Christian Holm
    @date   Tue May 06 14:11:51 2003
    @brief  Utility functions used in the tests */

#ifndef __IOSTREAM__
# include <iostream>
#endif
#ifndef __STRING__
# include <string>
#endif
#ifdef HAVE_CONFIG_H
# include "config.hh"
#include <stdlib.h>
#else
# define PACKAGE_STRING "FILE-- ???"
#endif

/** Print usage message on stdandard out 
    @param name The name of the program 
*/
inline void usage(const char* name) 
{
  std::cout << "Usage: " << name << " [OPTIONS] [FILE]" << std::endl;
  exit(0);
}

/** Print version information for the program 
    @param name The name of the program 
*/
inline void version(const char* name) 
{
  std::cout << PACKAGE_STRING << " test program: " << name << std::endl;
  exit(0);
}

/** Get command line arguments. 
    @param argc Number of arguments from @c main
    @param argv The command line arguments from @c main
    @param file Variable to hold file argument 
*/
inline void getargs(int& argc, char** argv, std::string& file) 
{
  for (int i = 1; i < argc; ++i) {
    if (argv[i] && argv[i][0] != '-') { 
      file = argv[i];
      continue;
    }
    if (argv[i][1] && argv[i][1] != '-') {
      switch(argv[i][1]) {
      case 'h': usage(argv[0]);   break;
      case 'v': version(argv[0]); break;
      }
    }
    else {
      std::string opt(argv[i]);
      if (opt == "--help")    usage(argv[0]);
      if (opt == "--version") version(argv[0]);
    }
  }
}


#endif
//____________________________________________________________________
//
// EOF
//
