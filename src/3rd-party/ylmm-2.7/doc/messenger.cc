//
// $Id: messenger.cc,v 1.2 2003/06/21 20:22:28 cholm Exp $
//
//   messenger.cc
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
/** @file   doc/messenger.cc
    @author Christian Holm
    @date   Fri Jan 03 05:00:58 2003
    @brief  Buffer documentation */
/** @page messenger_doc Handling Output Messages

    To make the handling of output messages of the generated parser
    and scanner, the package defines the class ylmm::basic_messenger.
    It provides a number of servies to output text to a normal stream,
    and to an error stream. The default behaviour is to print to
    std::cout and std::cerr respectively.  

    However, one can instantise a ylmm::basic_messenger object with
    any two I/O streams, and then pass it to the scanner and parser
    classes via the member functions ylmm::parser_base::messenger and
    ylmm::scanner_base.  The classes will then use that messenger to
    handle output.  

    In this way, the client code can ensure that all messages and
    errors are logged to the same stream from both the scanner and the
    parser, and perhaps also from the client code itself. 

    The client code can derive a sub-class from ylmm::basic_messenger
    if needed, but a better approach is to use the std::streambuf
    layer of the standard library (see @ref buffer_issues). 
*/
#error This file is not for compilation
//
// EOF
//

