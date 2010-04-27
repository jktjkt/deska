/* -*- mode: c++; c-auto-newline: nil -*-                             */
/*                                                                    */
/* $Id: simple_scanner.ll,v 1.8 2003/06/29 10:27:19 cholm Exp $      */
/*                                                                    */
/*   Toy calculator C++ scanner                                       */
/*   Copyright (C) 2002  Christian Holm Christensen <cholm@nbi.dk>    */
/*                                                                    */
/*   This library is free software; you can redistribute it and/or    */
/*   modify it under the terms of the GNU Lesser General Public       */
/*   License as published by the Free Software Foundation; either     */
/*   version 2 of the License, or (at your option) any later version. */
/*                                                                    */
/*   This library is distributed in the hope that it will be useful,  */
/*   but WITHOUT ANY WARRANTY; without even the implied warranty of   */
/*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    */
/*   GNU Lesser General Public License for more details.              */
/*                                                                    */
/*   You should have received a copy of the GNU Lesser General Public */
/*   License along with this library; if not, write to the Free       */
/*   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,   */
/*   MA 02111-1307  USA                                               */
/*                                                                    */
/** @file   simple_scanner.ll                                         */
/*  @author Christian Holm Christensen                                */
/*  @date   Thu Aug 29 00:12:43 2002                                  */
/*  @brief  Toycalc_Scanner definition.                               */
/*                                                                    */
/*    This contains the lexical scanner rules for an integer parser. */
%{
#include "simple_scanner.hh"
#ifdef HAVE_CONFIG_H
# include "config.hh"
#endif
#define YLMM_SCANNER_CLASS simple_scanner
#define LEXDEBUG 1
#include <ylmm/lexmm.hh>
%}

%%
[0-9]+	     { return _scanner->integer(yytext, yyleng); }
"\n"         { return _scanner->newline(yytext, yyleng); }
"quit"       { return 0;                                 }
.            { if (yytext[0] == EOF) return 0;           }

%%

/* EOF */
