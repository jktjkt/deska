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
    namespace ascii = boost::spirit::ascii;

    /* Predefined rules for parsing single parameters
    *  TODO: Rewrite to some singleton or something.
    */
    template <typename Iterator>
    class PredefinedRules
    {
    public:
        PredefinedRules()
        {
            t_int %= qi::int_;
            t_string %= qi::lexeme[ '"' >> +( ascii::char_ - '"' ) >> '"' ];
            t_double %= qi::double_;
            identifier %= qi::lexeme[ *( ascii::alnum | '_' ) ];
        }

        qi::rule< Iterator, int(), ascii::space_type > t_int;
        qi::rule< Iterator, std::string(), ascii::space_type > t_string;
        qi::rule< Iterator, double(), ascii::space_type > t_double;
        qi::rule< Iterator, std::string(), ascii::space_type > identifier;
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
            using qi::_a;
            using qi::_val;
            using ascii::char_;

            predefined = new PredefinedRules< Iterator >();

            // Keyword table for matching keywords to parameter types (parser).
            // Keywords can be added during run time.
            keyword.add( "name", predefined->t_string );
            keyword.add( "id", predefined->t_int );
            keyword.add( "price", predefined->t_double );

            // Head of top-level grammar
            cat_start %= lit( "hardware" ) >> predefined->identifier;

            // Trick for building the parser during parse time
            start = cat_start >> +( keyword[ _a = _1 ] >> lazy( _a ) ) >> lit( "end" );
        }

        ~MainGrammar()
        {
            delete predefined;
        };

        qi::symbols< char, qi::rule< Iterator, ascii::space_type > > keyword;

        qi::rule< Iterator, std::string(), std::string(), ascii::space_type > cat_start;
        qi::rule< Iterator, ascii::space_type, qi::locals< qi::rule< Iterator, ascii::space_type > > > start;

        PredefinedRules< Iterator >* predefined;
    };
}



int main()
{
    using boost::spirit::ascii::space;
    typedef std::string::const_iterator iteratorType;
    typedef DeskaCLI::MainGrammar< iteratorType > MainGrammar;

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
            std::cout << "Parsing succeeded\n" << std::endl;
        }
        else
        {
            std::cout << "Parsing failed\n";
        }
    }
    return 0;
}
