#include "Parser.h"
#include "../db/FakeApi.h"



/* Now this parses:
*
*  hardware <name> name <quoted string> id <integer> price <double> end
*
*  Its only an example how this can be handled.
*/



int main()
{
    using boost::spirit::ascii::space;
    typedef std::string::const_iterator iteratorType;
    typedef Deska::CLI::HardwareGrammar< iteratorType > HardwareGrammar;

    std::cout << "hardware <name> name <quoted string> id <integer> price <double> end" << std::endl;

    HardwareGrammar g;
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