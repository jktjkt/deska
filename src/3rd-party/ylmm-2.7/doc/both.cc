//
// $Id: both.cc,v 1.3 2003/06/21 20:22:28 cholm Exp $
//
//   both.cc
//   Copyright (C) 2002  Christian Holm Christensen <cholm@nbi.dk>
//
//   This library is free software; you can redistribute it and/or
//   modify it under the terms of the GNU Lesser General Public License 
//   as published by the Free Software Foundation; either version 2 of 
//   the License, or (at your option) any later version.  
//
//   This library is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//   Lesser General Public License for more details. 
//
//   You should have received a copy of the GNU Lesser General Public
//   License along with this library; if not, write to the Free
//   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
//   MA 02111-1307  USA  
//
//
/** @file   doc/both.cc
    @author Christian Holm
    @date   Fri Jan 03 05:00:58 2003
    @brief  Both documentation */
/** @page both Putting the Parser and Scanner Together 

    Now, it's a really simple task to put the parser and scanner
    together and let them do their work. Here, we simply create an
    instance of out scanner and parser classes. 
    @dontinclude simple.cc 
    @skip main 
    @until simple_parser

    And finally, we put the parser to work by invoking the 
    @c parse member function 
    @until } 
*/
#error This file is not for compilation
//
// EOF
//
