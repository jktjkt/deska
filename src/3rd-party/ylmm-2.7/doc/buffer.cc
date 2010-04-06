//
// $Id: buffer.cc,v 1.3 2003/06/21 20:22:28 cholm Exp $
//
//   buffer.cc
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
/** @file   doc/buffer.cc
    @author Christian Holm
    @date   Fri Jan 03 05:00:58 2003
    @brief  Buffer documentation */
/** @page buffer_issues More on ylmm::basic_buffer 

    The class ylmm::basic_buffer is an @e "Abstract base class" (ABC)
    for various kinds of input buffers.  It's interface is quite
    simple.  

    The thing to notice about ylmm::basic_buffer is the member function
    ylmm::basic_buffer::input and the pure virtual member functions
    ylmm::basic_buffer::input_buffered and
    ylmm::basic_buffer::input_interactive - at a minimum, these later
    two @e must overloaded in a derived class.

    ylmm::basic_buffer::input is the member function responsible for
    reading characters into the @b Flex generated C function.
    Depending on wether the scanner was defined as interactive or
    buffered, the member function will deligate the call to one of the
    virtual member functions ylmm::basic_buffer::input_buffered or
    ylmm::basic_buffer::input_interactive.  

    Hence, the derived class should overload
    ylmm::basic_buffer::input_buffered to read as much as possible into
    an internal buffer, while ylmm::basic_buffer::input_interactive
    should read up to a newline and no more, immediately returning the
    result.  

    The basic class reads input from an @c std::istream.  Using the @c
    std::streambuf layer, one can setup the scanner to read from
    almost anything.  The sterio-typical example is ofcourse from @c
    std::cin, which is also the default.  
    @code 
    my_scanner scanner(new ylmm::istream_buffer(std::cin)); 
    @endcode

    Another common applicatin is to read from a 
    file via @c std::ifstream.  
    @code 
    std::ifstream input_file("input_file.txt");
    my_scanner scanner(new ylmm::istream_buffer(input_file));
    @endcode 
        
    But one can easily make more exotic variations.  Suppose you
    have a class @c rlmm::streambuf which reads input lines
    via a @b readline interface (see 
    <a href="http://cholm.home.cern.ch/cholm/misc/#rlmm">here</a>
    for such an interface), and that class derives from 
    @c std::streambuf, then one can use the stream buffer layer to
    read lines from the user via @b readline, like so: 
    @code 
    std::istream input(new read_line::streambuf("prompt> ");
    ylmm::istream_buffer buffer(input);
    buffer.interactive(true);
    my_scanner scanner(&buffer);
    @endcode 

    At first, as well as reading the ISO/IEC C++ standard, you may
    be left with the impression that the @c std::streambuf layer
    is extreemly complex and difficult to deal with.  In fact, it
    is not.  It's one of the more beautiful designed elements of
    the ISO/IEC C++ standard.  Using the standard @c
    std::streambuf layer, only your imagination sets the limit. 
*/
#error This file is not for compilation
//
// EOF
//
