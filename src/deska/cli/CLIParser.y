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
#include "CLIExpression.h"
#include "CLIParser.h"
#define YLMM_PARSER_CLASS CLI::Parser
#define YLMM_LEX_STATIC
#include <ylmm/yaccmm.hh>
%}
/* %pure_parser */

%token  HARDWARE
%token  MANUFACTURER
%token  MODEL
%token  CHASSIS
%token  WEIGHT
%token  END
%token  TEMPLATE
%token  HOST
%token  ROLE
%token  IP
%token  MAC
%token  SERIAL
%token  INT
%token  VLAN
%token  SWITCH
%token  PORT
%token  HW
%token  COMMIT
%token  WORD
%token  WORDS
%token  INTEGER
%token  FLOAT
%token  IPV4_ADDR
%token  IPV4_ADDR_WITH_MASK
%token  IPV6_ADDR
%token  MAC_ADDR
%token  ETH_INT_NAME

     
%%
/* Grammar follows */
commands : /* empty string */
         | commands command
         ;

command : HARDWARE WORDS commands_hardware         { $$ = _parser->result($2); }
        | TEMPLATE HOST WORDS commands_template    { $$ = _parser->result($2); }
        | HOST WORDS commands_host                 { $$ = _parser->result($2); }
        | COMMIT                                   { $$ = _parser->result(); }
        | END                                      { $$ = _parser->result(); }
        | MANUFACTURER WORDS                       { $$ = _parser->result($2); }
/*      | error NEWLINE                            { yyerrok; } */
        ;
         
commands_hardware : /* empty string */
                  | commands_hardware command_hardware
                  ;
         
command_hardware : MANUFACTURER WORDS    { $$ = _parser->result($2); }
                 | MODEL WORDS           { $$ = _parser->result($2); }
                 | CHASSIS WORDS         { $$ = _parser->result($2); }
                 | WEIGHT WORDS          { $$ = _parser->result($2); }
                 | END                   { $$ = _parser->result(); }
                 ;
                 
commands_template : /* empty string */
                  | commands_template command_template
                  ; 
                  
command_template : HARDWARE WORDS    { $$ = _parser->result($2); }
                 | ROLE WORDS        { $$ = _parser->result($2); }
                 ;
                 
commands_host : /* empty string */
              | commands_host command_host
              ;
                   
command_host : TEMPLATE WORDS                   { $$ = _parser->result($2); }
             | SERIAL WORD                      { $$ = _parser->result($2); }
             | ROLE WORDS                       { $$ = _parser->result($2); }
             | MODEL WORDS                      { $$ = _parser->result($2); }
             | INT ETH_INT_NAME commands_int    { $$ = _parser->result($2); }
             | END                              { $$ = _parser->result(); }
             ;
             
commands_int : /* empty string */  
             | commands_int command_int
             ; 
             
command_int : IP IPV4_ADDR_WITH_MASK    { $$ = _parser->result($2); }
            | IP IPV6_ADDR              { $$ = _parser->result($2); }
            | MAC MAC_ADDR              { $$ = _parser->result($2); }
            | VLAN INTEGER              { $$ = _parser->result($2); }
            | SWITCH WORDS              { $$ = _parser->result($2); }
            | PORT WORDS                { $$ = _parser->result($2); }
            | END                       { $$ = _parser->result(); }
            ;                        

%%
