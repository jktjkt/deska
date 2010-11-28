//#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <complex>


namespace DeskaCLI
{

	namespace qi = boost::spirit::qi;
    namespace ascii = boost::spirit::ascii;

    template <typename Iterator>
    struct MainGrammar : qi::grammar<Iterator, int(), ascii::space_type>
    {
        MainGrammar() : MainGrammar::base_type(start)
        {
            using qi::int_;
            using qi::lit;
            using qi::double_;
            using qi::lexeme;
			using qi::_1;
			using qi::_val;
            using ascii::char_;

			//This works fine. Accepts permutation of subset of
			//<hardware, par1, par2, par3, end> and returns id of last item.
			start = lit("hardware") [ _val = 0 ]
				  ^ lit( "par1" ) [ _val = 1 ]
				  ^ lit( "par2" ) [ _val = 2 ]
				  ^ lit( "par3" ) [ _val = 3 ]
				  ^ lit( "end" ) [ _val = -1 ];
			
			//This crashes
			/*
			start = lit("hardware") [ _val = 0 ];
            start = start ^ lit( "par1" ) [ _val = 1 ];
            start = start ^ lit( "par2" ) [ _val = 2 ];
            start = start ^ lit( "par3" ) [ _val = 3 ];
            start = start ^ lit( "end" ) [ _val = -1 ];
			*/

        }

        qi::rule<Iterator, int(), ascii::space_type> start;
    };
}



int main()
{
    using boost::spirit::ascii::space;
    typedef std::string::const_iterator iterator_type;
    typedef DeskaCLI::MainGrammar<iterator_type> MainGrammar;

    MainGrammar g;
    std::string str;
    while (getline(std::cin, str))
    {
        if (str.empty() || str[0] == 'q' || str[0] == 'Q')
            break;

		int ret;
        std::string::const_iterator iter = str.begin();
        std::string::const_iterator end = str.end();
        bool r = phrase_parse(iter, end, g, space, ret);

        if (r && iter == end)
        {
            std::cout << "got: " << ret << std::endl;
        }
        else
        {
            std::cout << "Parsing failed\n";
        }
    }
    return 0;
}
