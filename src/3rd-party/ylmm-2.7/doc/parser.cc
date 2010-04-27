//
// $Id: parser.cc,v 1.6 2003/11/18 11:27:31 cholm Exp $
//
//   parser.cc
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
/** @file   doc/parser.cc
    @author Christian Holm
    @date   Fri Jan 03 05:00:58 2003
    @brief  Parser documentation */
/** @page parser_doc The Parser Class

    @section parser_setup Setting up the Parser Class

    To make a C++ class for the parser, you should have a grammar file
    that looks like
    @code 
    %{ 
    ... // Normal declaration stuff goes here 
    #define YLMM_PARSER_CLASS parser
    #define YLMM_LEX_STATIC 
    #include <ylmm/yaccmm.hh> 
    %}
   ...// The rest is as usual 
    @endcode 

    The file @link yaccmm.hh ylmm/yaccmm.hh @endlink redefines the
    usual @b Bison macros to forward calls to the user provided class
    (or it's base class ylmm::basic_parser).  The macro 
    @c YYPRINT is defined to forward calls to 
    ylmm::basic_parser::print, @c YYFPRINTF to
    ylmm::basic_parser::message, @c YYFPRINTF to
    ylmm::basic_parser::trace, yyerror to
    ylmm::basic_parser::error.  @c YYLTYPE is defined to be
    ylmm::location, and @c YYLLOC_DEFAULT to call
    ylmm::location::last.  To customize the behaviour, overload  the
    corresponding member functions in you derived classess. 

    You are free to define macros @c YYPARSE_PARAM, @c YYLEX_PARAM, 
    @c YYERROR_VERBOSE, @c YYLSP_NEEDED, and so on.  

    The file @link yaccmm.hh ylmm/yaccmm.hh @endlink
    defines the interface to the generated C function via a C++ class,
    where @c YLMM_PARSER_CLASS is the name of the user parser class. 
    The developer can either specify this as a specific instantation
    of ylmm::basic_parser, or it can be a sub-class of specific
    instantation of ylmm::basic_parser. 

    The macro @link yaccmm.hh YLMM_LEX_STATIC @endlink @e must be
    defined if the @b Yacc input file isn't a pure parser.  If it's
    defined, the static function @c int @c yylex() will be defined.  
    If your grammar uses location information, then you need to define   
    @link yaccmm.hh YLMM_LEX_STATIC_LOCATION @endlink instead of 
    @c YLMM_LEX_STATIC.  Location information comes about in a 
    @b Bison grammar via the use of @c @@$ or @c @@N (where @c N is a 
    number) in the grammar rules or an explicit definition of the
    preprocessor constant @c YYLSP_NEEDED in the declaration part.  

    If the sematic token type isn't defined before inclussion of
    ylmm/yacmm.hh, then it is defined to be
    ylmm::basic_parser::token_type of the specific instantation. 

    If the grammar defines a @e Pure (that is @e reentrant) parser  
    (via the @b Bison directive @c %pure_parser), then
    @c YLMM_LEX_STATIC must not be defined.

    A derived class must define the member function @c scan.  Hence, a
    minimal derived class looks like.  
    @code 
    #ifndef YLMM_basic_parser
    #include <ylmm/basic_parser.hh>
    #endif

    class parser : public ylmm::basic_parser<YYSTYPE> { 
    public:
      int  scan(); 
    }
    @endcode 
    The @c scan member function should be defined in the regular way
    of @c yylex - that is, it should read a token from the input and
    return it's sematic token number (see also the @b Bison 
    documentation).

    If the application uses the ylmm::basic_parser template directly,
    the ylmm::basic_parser::scan member function must be defined for
    the particular instantitation, as the default behaviour is to end
    parsing immediately. 

    See also the example 
    @link simple_parser.yy simple_parser.yy @endlink 
    for and example of simple usage, and 
    @link toycalc_parser.yy toycalc_parser.yy @endlink for a more
    complex usage. 

    @section parser_file Example Grammar 

    The grammar is defined in the usual way.  Note, that a pointer to
    the parser is passed as the parameter @c _parser to the @b Bison
    generated C function.  Hence, you can use that pointer in actions
    of the grammar. 

    Below follows a simple example of a parser of integers.  This is
    reproduced from the example 
    @link simple_parser.yy simple_parser.yy @endlink. See also the
    example @link toycalc_parser.yy toycalc_parser.yy @endlink for a
    more complex example.  
    
    First, we setup the usual stuff (see @ref parser_setup). 
    @dontinclude simple_parser.yy 
    @skip %{ 
    @until %}
    
    Next we define all the tokens and precedence rules. 
    @until %%

    And then finally we make the production rules, where we use the
    pointer to the parser object (via the @c _parser static variable),
    to make the actual productions 
    @until %%

    @subsection parser_class The Parser Class 

    The user defined parser class can then be setup to make a parse
    tree, using its member functions. 

    Below follows a simple example of a parser of integers.  This is
    reproduced from the example 
    @link simple_parser.hh simple_parser.hh @endlink. See also
    @link toycalc_parser.hh toycalc_parser.hh @endlink for a more
    complex example. 
    
    Here, we use an object of a ylmm::basic_scanner derived class to do
    the lexical scanning. That class will be explored in 
    @ref scanner_file below. 
    @dontinclude simple_parser.hh 
    @skip class 
    @until scan(

    We define the needed member functions (see @ref parser_setup), as
    well as a couple a utility function.  The @c error member
    functions uses the data member ylmm::basic_parser::_err_stream for
    output.  The point is, that the user can set these so that all 
    error messages are treated the same when using other libraries,
    etc. 
    @until if 

    And finally, we define the member functions to deal with the
    semantic tokens to generated by the production rules in the parser
    file.  Notice that we again use the data member 
    ylmm::basic_parser::_msg_stream for output so that we can insure
    coherient output from the client application. 
    @until }; 

*/
#error This file is not for compilation
//
// EOF
//
