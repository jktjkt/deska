#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/fusion/include/io.hpp>

#include <iostream>
#include <string>
#include <vector>


/* Now this parses:
*
*  hardware <name> name <quoted string> id <integer> price <double> end
*
*  Its only an example how this can be handled.
*/


namespace DeskaCLI
{
    namespace qi = boost::spirit::qi;
    namespace spirit = boost::spirit;
    namespace phoenix = boost::phoenix;
    namespace ascii = boost::spirit::ascii;


    template < typename Iterator >
    class ErrorHandler
    {
    public:
        void operator()( Iterator start, Iterator end, Iterator errorPos, const spirit::info& what )
        {
            std::cout
                << "Error! Expecting " << what
                << " here: \"" << phoenix::construct< std::string >( errorPos, end ) << "\""
                << std::endl ;
        }
    };


    /* Predefined rules for parsing single parameters
    *  TODO: Rewrite to some singleton or something.
    */
    template < typename Iterator >
    class PredefinedRules
    {
    public:
        PredefinedRules()
        {
            t_int %= qi::int_;
            t_int.name( "integer" );
            
            t_string %= qi::lexeme[ '"' >> +( ascii::char_ - '"' ) >> '"' ];
            t_string.name( "quoted string" );
            
            t_double %= qi::double_;
            t_double.name( "double" );

            identifier %= qi::lexeme[ *( ascii::alnum | '_' ) ];
            identifier.name( "identifier (alphanumerical letters and _)" );
        }

        qi::rule< Iterator, int(), ascii::space_type > t_int;
        qi::rule< Iterator, std::string(), ascii::space_type > t_string;
        qi::rule< Iterator, double(), ascii::space_type > t_double;
        qi::rule< Iterator, std::string(), ascii::space_type > identifier;
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

            predefined = new PredefinedRules< Iterator >();

            // Keyword table for matching keywords to parameter types (parser)
            keyword.add( "name", predefined->t_string );
            keyword.add( "id", predefined->t_int );
            keyword.add( "ip", predefined->t_string );

            // Head of top-level grammar
            cat_start %= lit( "interface" ) >> predefined->identifier;

            // Trick for building the parser during parse time
            start = cat_start >> +( keyword[ _a = _1 ] >> lazy( _a ) ) >> lit( "end" );
        }

        ~IfaceGrammar()
        {
            delete predefined;
        };

        qi::symbols< char, qi::rule< Iterator, ascii::space_type > > keyword;
        qi::symbols< char, qi::grammar< Iterator, ascii::space_type, qi::locals< qi::rule< Iterator, ascii::space_type > > > > nested;

        qi::rule< Iterator, std::string(), std::string(), ascii::space_type > cat_start;
        qi::rule< Iterator, ascii::space_type, qi::locals< qi::rule< Iterator, ascii::space_type > > > start;

        PredefinedRules< Iterator >* predefined;
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

            predefined = new PredefinedRules< Iterator >();
            errHandler = new ErrorHandler< Iterator >();

            // Keyword table for matching keywords to parameter types (parser)
            keyword.add( "name", predefined->t_string );
            keyword.add( "id", predefined->t_int );
            keyword.add( "price", predefined->t_double );

            // TODO: Problem, that grammars are non-copyable objects -> wrapping to phoenix::ref() or something
            //qi::rule< Iterator, ascii::space_type, qi::locals< qi::rule< Iterator, ascii::space_type > > > iface = IfaceGrammar< Iterator >();
            //IfaceGrammar< Iterator > iface = IfaceGrammar< Iterator >();
            //nested.add( "interface", iface );

            // Head of top-level grammar
            cat_start %= lit( "hardware" ) > predefined->identifier;
            cat_start.name("cathegory start");

            // Trick for building the parser during parse time
            // TODO: Problem, that grammars are non-copyable objects -> wrapping to phoenix::ref() or something
            start = ( cat_start > +( ( keyword[ _a = _1 ] > lazy( _a ) ) /*|| ( nested[ _a = _1 ] >> lazy( _a ) )*/ ) >> lit( "end" ) );

            on_error< fail >( start, std::cout
                << phoenix::val( "Error! Expecting " ) << _4
                << phoenix::val( " here: \"" ) << phoenix::construct< std::string >( _3, _2 ) << phoenix::val( "\"" )
                << std::endl );
            
            //Failed to compile when trying to delegate error handling to separate class
            //on_error< fail >( start, *errHandler( _1, _2, _3, _4 ) );
        }

        ~MainGrammar()
        {
            delete predefined;
            delete errHandler;
        };

        qi::symbols< char, qi::rule< Iterator, ascii::space_type > > keyword;
        qi::symbols< char, qi::rule< Iterator, ascii::space_type, qi::locals< qi::rule< Iterator, ascii::space_type > > > > nested;
        
        qi::rule< Iterator, std::string(), std::string(), ascii::space_type > cat_start;
        qi::rule< Iterator, ascii::space_type, qi::locals< qi::rule< Iterator, ascii::space_type > > > start;

        PredefinedRules< Iterator >* predefined;
        ErrorHandler< Iterator >* errHandler;
    };
}



int main()
{
    using boost::spirit::ascii::space;
    typedef std::string::const_iterator iteratorType;
    typedef DeskaCLI::MainGrammar< iteratorType > MainGrammar;

    std::cout << "hardware <name> name <quoted string> id <integer> price <double> end" << std::endl;

    MainGrammar g;
    std::string str;
    while ( getline( std::cin, str ) )
    {
        if ( str.empty() || str[ 0 ] == 'q' || str[ 0 ] == 'Q' )
            break;

        std::string::const_iterator iter = str.begin();
        std::string::const_iterator end = str.end();
        bool r = phrase_parse( iter, end, g, space );

        if ( r && iter == end )
        {
            std::cout << "Parsing succeeded" << std::endl;
        }
        else
        {
            std::cout << "Parsing failed" << std::endl;
        }
    }
    return 0;
}
