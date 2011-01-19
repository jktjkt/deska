#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include <boost/config/warning_disable.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>

namespace Deska
{
namespace CLI
{

    namespace spirit = boost::spirit;
    namespace phoenix = boost::phoenix;
    namespace ascii = boost::spirit::ascii;
    namespace qi = boost::spirit::qi;



    template < typename Iterator >
    class ErrorHandler
    {
    public:
        template <typename, typename, typename, typename>
            struct result { typedef void type; };

        void operator()( Iterator start, Iterator end, Iterator errorPos, const spirit::info& what ) const;
    };



    /* Predefined rules for parsing single parameters
    */
    template < typename Iterator >
    class PredefinedRules
    {
    public:
        PredefinedRules();
        qi::rule< Iterator, ascii::space_type > getRule( const std::string typeName );

    private:
        std::map< std::string, qi::rule< Iterator, ascii::space_type > > rulesMap;
    };



    template < typename Iterator >
    class IfaceGrammar: public qi::grammar< Iterator, ascii::space_type, qi::locals< qi::rule< Iterator, ascii::space_type > > >
    {
    public:
        IfaceGrammar() : IfaceGrammar::base_type( start )
        {
            using qi::int_;
            using qi::lit;
            using qi::double_;
            using qi::lexeme;
            using qi::_1;
            using qi::_a;
            using qi::_val;
            using ascii::char_;

            // Keyword table for matching keywords to parameter types (parser)
            keyword.add( "name", predefined.getRule( "string" ) );
            keyword.add( "id", predefined.getRule( "integer" ) );
            keyword.add( "ip", predefined.getRule( "string" ) );

            // Head of top-level grammar
            cat_start %= lit( "interface" ) >> predefined.getRule( "identifier" );

            // Trick for building the parser during parse time
            start = cat_start >> +( keyword[ _a = _1 ] >> lazy( _a ) ) >> lit( "end" );
        }

        qi::symbols< char, qi::rule< Iterator, ascii::space_type > > keyword;
        qi::symbols< char, qi::grammar< Iterator, ascii::space_type, qi::locals< qi::rule< Iterator, ascii::space_type > > > > nested;

        qi::rule< Iterator, std::string(), std::string(), ascii::space_type > cat_start;
        qi::rule< Iterator, ascii::space_type, qi::locals< qi::rule< Iterator, ascii::space_type > > > start;

        PredefinedRules< Iterator > predefined;
    };

    template <typename Iterator>
    class MainGrammar: public qi::grammar< Iterator, ascii::space_type, qi::locals< qi::rule< Iterator, ascii::space_type > > >
    {
    public:
        MainGrammar() : MainGrammar::base_type( start )
        {
            using qi::int_;
            using qi::lit;
            using qi::double_;
            using qi::lexeme;
            using qi::_1;
            using qi::_2;
            using qi::_3;
            using qi::_4;
            using qi::_a;
            using qi::_val;
            using qi::on_error;
            using qi::fail;
            using ascii::char_;
            using ascii::string;

            // Keyword table for matching keywords to parameter types (parser)
            keyword.add( "name", predefined.getRule( "string" ) );
            keyword.add( "id", predefined.getRule( "integer" ) );
            keyword.add( "price", predefined.getRule( "double" ) );

            // TODO: Problem, that grammars are non-copyable objects -> wrapping to phoenix::ref() or something
            //qi::rule< Iterator, ascii::space_type, qi::locals< qi::rule< Iterator, ascii::space_type > > > iface = IfaceGrammar< Iterator >();
            //IfaceGrammar< Iterator > iface = IfaceGrammar< Iterator >();
            //nested.add( "interface", iface );

            // Head of top-level grammar
            cat_start %= lit( "hardware" ) > predefined.getRule( "identifier" );
            cat_start.name("cathegory start");

            // Trick for building the parser during parse time
            // TODO: Problem, that grammars are non-copyable objects -> wrapping to phoenix::ref() or something
            start = ( cat_start > +( ( keyword[ _a = _1 ] > lazy( _a )[ std::cout << "Parsed: " << _1 << "\n" ] ) /*|| ( nested[ _a = _1 ] >> lazy( _a ) )*/ ) > lit( "end" ) );

            phoenix::function< ErrorHandler< Iterator> > wrappedError = ErrorHandler< Iterator >();
            on_error< fail >( start, wrappedError( _1, _2, _3, _4 ) );
        }

        qi::symbols< char, qi::rule< Iterator, ascii::space_type > > keyword;
        qi::symbols< char, qi::rule< Iterator, ascii::space_type, qi::locals< qi::rule< Iterator, ascii::space_type > > > > nested;
        
        qi::rule< Iterator, std::string(), std::string(), ascii::space_type > cat_start;
        qi::rule< Iterator, ascii::space_type, qi::locals< qi::rule< Iterator, ascii::space_type > > > start;

        PredefinedRules< Iterator > predefined;
    };
}
}



#endif
