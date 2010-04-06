/* -*- mode: c++; c-auto-newline: nil -*- 
  
   $Id: toycalc_parser.yy,v 1.8 2004/02/19 13:59:46 cholm Exp $
  
     Toy calculator C++ scanner
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
/** @file   toycalc_parser.yy
    @author Christian Holm Christensen
    @date   Thu Aug 29 00:12:43 2002
    @brief  Parser definition. 

    This contains the grammar parser for the toy calculator. */
%{
  /* Declarations */
#define YYDEBUG 1
#define yyparse tcparse
#define yylex tclex
#define yyerror tcerror
#define yylval tclval
#define yychar tcchar
#define yydebug tcdebug
#define yynerrs tcnerrs
#ifndef YYLSP_NEEDED
#define YYLSP_NEEDED 1
#endif
#define YYERROR_VERBOSE
#include "toycalc_expression.hh"
#include "toycalc_parser.hh"
#define YLMM_PARSER_CLASS toycalc::parser
#define YLMM_LEX_STATIC
#include <ylmm/yaccmm.hh>
%}
/* %pure_parser */

%token  NUM
%token  LPAREN           /* '('                */
%token  RPAREN           /* ')'                */
%token  EQ               /* '='                */
%token  PLUS             /* '+'                */
%token  MINUS            /* '-'                */
%token  STAR             /* '*'                */
%token  SLASH            /* '/'                */
%token  NEWLINE          /* '\n'               */


%left   MINUS PLUS       /* addition and subtration     */
%left   STAR  SLASH      /* Multiplication and division */
%left   NEG              /* negation--unary minus       */
     
%%
/* Grammar follows */
input   : /* empty string */
        | input line
        ;

line    : NEWLINE                { $$ = _parser->result();              }
        | exp NEWLINE            { $$ = _parser->result($1);            }
	| error NEWLINE          { yyerrok; 				}
        ;

exp     : NUM                    { $$ = _parser->number($1);            }
        | exp PLUS exp           { $$ = _parser->addition($1, $3);      }
        | exp MINUS exp          { $$ = _parser->subtraction($1, $3);   }
        | exp STAR exp           { $$ = _parser->multiplication($1, $3);}
        | exp SLASH exp          { $$ = _parser->division($1, $3);      }
        | MINUS exp  %prec NEG   { $$ = _parser->negation($2);          }
        | LPAREN exp RPAREN      { $$ = _parser->precedence($2);        }
        ;
%%
