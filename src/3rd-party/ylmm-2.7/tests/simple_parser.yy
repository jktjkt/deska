/* -*- mode: c++; c-auto-newline: nil -*- 
  
   $Id: simple_parser.yy,v 1.3 2003/01/04 15:45:50 cholm Exp $
  
   Simple C++ scanner
     Copyright (C) 2002  Christian Holm Christensen <cholm@nbi.dk>
  
     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Lesser General Public License 
     as published by the Free Software Foundation; either version 2 of 
     the License, or (at your option) any later version.  
  
     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Lesser General Public License for more details. 
  
     You should have received a copy of the GNU Lesser General Public
     License along with this library; if not, write to the Free
     Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
     MA 02111-1307  USA  
*/
/** @file   simple_parser.yy
    @author Christian Holm Christensen
    @date   Thu Aug 29 00:12:43 2002
    @brief  parser definition. 

    This contains the grammar parser for an integer grammar */
%{
  /* Declarations */
#include "simple_parser.hh"
#define YLMM_PARSER_CLASS simple_parser
#define YLMM_LEX_STATIC
#include <ylmm/yaccmm.hh>
%}
%token NUM
%token NEWLINE
%%
input   : /* empty string */
        | input line
        ;

line    : NEWLINE       { $$ = _parser->result();   }
        | NUM NEWLINE   { $$ = _parser->result($1); }
        | error NEWLINE { yyerrok;                 }
        ;
%%

/*
  EOF
*/ 
