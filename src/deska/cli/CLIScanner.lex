%{
#include "CLIScanner.h"
#ifdef HAVE_CONFIG_H
# include "Config.h"
#endif
#define YLMM_SCANNER_CLASS CLI::Scanner
#include <ylmm/lexmm.hh>
%}

/* Short hands for patterns */
A                      ([Aa])
B                      ([Bb])
C                      ([Cc])
D                      ([Dd])
E                      ([Ee])
F                      ([Ff])
G                      ([Gg])
H                      ([Hh])
I                      ([Ii])
J                      ([Jj])
K                      ([Kk])
L                      ([Ll])
M                      ([Mm])
N                      ([Nn])
O                      ([Oo])
P                      ([Pp])
Q                      ([Qq])
R                      ([Rr])
S                      ([Ss])
T                      ([Tt])
U                      ([Uu])
V                      ([Vv])
W                      ([Ww])
X                      ([Xx])
Y                      ([Yy])
Z                      ([Zz])
WHITESPACE             ([ \r\t\f])

WORD                   ([a-zA-Z0-9\-_]+)
WORDS                  (({WORD}{WHITESPACE)*{WORD})
INTEGER                ([0-9]+)
HEX_DIGIT              ([a-fA-F0-9])
HEX_NUMBER             ({HEX_DIGIT}+)
FRAC_CONST             (([0-9]*\.[0-9]+)|([0-9]\.))
EXPO_PART              ([eE][-+]?[0-9]+)
FLOAT                  (({FRAC_CONST}{EXPO_PART}?)|([0-9]+{EXPO_PART}?))

IPV4_ADDR              ({INTEGER}\.{INTEGER}\.{INTEGER}\.{INTEGER})
IPV4_ADDR_WITH_MASK    ({IPV4_ADDR}/{INTEGER})
IPV6_ADDR              (({HEX_NUMBER}\:){0-7}{HEX_NUMBER})
MAC                    (({HEX_DIGIT}{2}\:){5}{HEX_DIGIT})
ETH_INTERFACE_NAME     ({E}{T}{H}{INTEGER})


%%

/* skip whitespaces */
WHITESPACE+

{H}{A}{R}{D}{W}{A}{R}{E}                  { return 0; }
{M}{A}{N}{U}{F}{A}{C}{T}{U}{R}{E}{R}      { return 0; }
{M}{O}{D}{E}{L}                           { return 0; }
{C}{H}{A}{S}{S}{I}{S}                     { return 0; }
{W}{E}{I}{G}{H}{T}                        { return 0; }
{E}{N}{D}                                 { return 0; }
{T}{E}{M}{P}{L}{A}{T}{E}                  { return 0; }
{H}{O}{S}{T}                              { return 0; }
{R}{O}{L}{E}                              { return 0; }
{I}{P}                                    { return 0; }
{M}{A}{C}                                 { return 0; }
{S}{E}{R}{I}{A}{L}                        { return 0; }
{I}{N}{T}                                 { return 0; }
{V}{L}{A}{N}                              { return 0; }
{S}{W}{I}{T}{C}{H}                        { return 0; }
{P}{O}{R}{T}                              { return 0; }
{H}{W}                                    { return 0; }
{C}{O}{M}{M}{I}{T}                        { return 0; }



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
