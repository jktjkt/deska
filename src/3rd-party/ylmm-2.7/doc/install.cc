//
// $Id: install.cc,v 1.3 2003/06/29 11:29:20 cholm Exp $
//
//    install.cc
//    Copyright (C) 2002  Christian Holm <cholm@linux.HAL3000> 
//   
//    This library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Lesser General Public
//    License as published by the Free Software Foundation; either
//    version 2.1 of the License, or (at your option) any later
//    version.
//   
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Lesser General Public License for more details.
//   
//    You should have received a copy of the GNU Lesser General Public
//    License along with this library; if not, write to the Free
//    Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
//    MA 02111-1307 USA
//
/** @file   install.cc
    @author Christian Holm
    @date   Sun Jun 29 13:27:25 2003
    @brief  Installation information
*/
/** @page install Installation Issues

    @section install1 Installation 

    To install this package, go through the well known 3-step build
    process:
    @verbatim 
    ./configure 
    make 
    make install
    @endverbatim 

    For the various options that you can pass to <tt>./configure</tt>
    please do 
    @verbatim
    ./configure --help
    @endverbatim 

    @section platform Tested platforms 

    <center>
      <table border=0 style="border-top:thin gray solid;border-left:thin
        gray solid;border-bottom:thin gray solid;border-right:thin
        gray solid;"> 
      <tr style="border-bottom:thin gray solid;">
        <th>CPU</th><th>Operating System</th><th>Compiler</th>
	<th>Yacc</th><th>Lex</th>
      </tr>
      <tr style="border-bottom:thin gray solid;">
        <td>i386</td><td>Debian GNU/Linux 3.0</td>
        <td>
          GCC 2.95<br>
          GCC 3.2<br>
          Intel C++ 7.0
        </td>
	<td>
	  <a href="http://www.gnu.org/software/bison">bison</a><br>
	  byacc<br>
	  <a
	  href="http://minnie.tuhs.org/UnixTree/V7/usr/src/cmd/yacc/">Original
	  Yacc</a> 
	</td>
	<td>
	  <a href="http://www.gnu.org/software/flex">flex</a><br>
	  <a
	  href="http://minnie.tuhs.org/UnixTree/V7/usr/src/cmd/lex/">Original 
	  Lex</a> 
	</td>
      </tr>
      <tr class=doc>
        <td>i386</td><td>Windos XP (cygwin)</td>
        <td>
          GCC 2.95<br>
          GCC 3.2
        </td>
	<td>
	  <a href="http://www.gnu.org/software/bison">bison</a><br>
	</td>
	<td>
	  <a href="http://www.gnu.org/software/flex">flex</a><br>
	</td>
      </tr>
    </table>
    </center>

    @section msvc Microsoft Visual C++ (not yet tested)
    
    To install using Microsoft Visual C++<sup>TM</sup>, you can do it
    two ways: under <a href="http://www.cygwin.com">CygWin</a> or from
    Microsoft Visual Studio<sup>TM</sup> (currently not supported).  I
    heartly recommend the Cygwin approach, as it will install
    everything for you. 

    @b Cygwin: In a @b bash shell do 
    @verbatim 
    ./configure CC=`pwd`/ide/cl					\
	        CXX=`pwd`/ide/cl				\
	        CXXFLAGS="-GX"					
    @endverbatim 
    <ul>
      <li> @c ide/cl is a wrapper script around the Microsoft Visual
        C++ compiler @c cl.exe.  The @e reason @e d'etre of this
	wrapper is that I use @c .cc to denote source files, so I need
	to pass the option @c -Tp for each source file. </li>
      <li> @c CXXFLAGS="-GX" is to turn on exception handling in 
        Microsoft Visual C++</li>
    </ul>

    <b>Microsoft Visual Studio<sup>TM</sup>:</b> You can either open
    the workspace file <tt>ide@\ylmm.dsw</tt> from the GUI, or in a
    command prompt run 
    @verbatim
    MSDEV ide\ylmm.dsw /MAKE 
    @endverbatim 

*/
#error This file is not for compilation
// 
// EOF
//
