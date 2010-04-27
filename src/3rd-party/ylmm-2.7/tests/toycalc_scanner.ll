/* -*- mode: c++; c-auto-newline: nil -*-                             */
/*                                                                    */
/* $Id: toycalc_scanner.ll,v 1.8 2003/06/29 10:27:20 cholm Exp $      */
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
/** @file   toycalc_scanner.ll                                        */
/*  @author Christian Holm Christensen                                */
/*  @date   Thu Aug 29 00:12:43 2002                                  */
/*  @brief  Toycalc_Scanner definition.                               */
/*                                                                    */
/*  This contains the lexical scanner rules for the toy calculator.   */ 
%{
#include "toycalc_scanner.hh"
#ifdef HAVE_CONFIG_H
# include "config.hh"
#endif
#define YLMM_SCANNER_CLASS toycalc::scanner
#include <ylmm/lexmm.hh>
%}

/* Short hands for patterns */
INTEGER         ([1-9][0-9]*)
FRAC_CONST      (([0-9]*\.[0-9]+)|([0-9]\.))
EXPO_PART       ([eE][-+]?[0-9]+)
FLOAT           (({FRAC_CONST}{EXPO_PART}?)|([0-9]+{EXPO_PART}?))

%%
{INTEGER}            { return _scanner->integer(yytext, yyleng);  }
{FLOAT}              { return _scanner->floating(yytext, yyleng); }
"("                  { return _scanner->lparen(yytext, yyleng);   }
")"                  { return _scanner->rparen(yytext, yyleng);   }
"+"                  { return _scanner->plus(yytext, yyleng);     }
"-"                  { return _scanner->minus(yytext, yyleng);    }
"*"                  { return _scanner->star(yytext, yyleng);     }
"/"                  { return _scanner->slash(yytext, yyleng);    }
"\n"                 { return _scanner->newline(yytext, yyleng);  }
"quit"               { return 0;                                  }
.                    { if (yytext[0] == EOF) return 0;            }
%% 
