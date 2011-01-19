#include "Parser.h"



template < typename Iterator >
void Deska::CLI::ErrorHandler< Iterator >::operator()(
    Iterator start,
    Iterator end,
    Iterator errorPos,
    const spirit::info& what ) const
{
    std::cout
        << "Error! Expecting " << what
        << " here: \"" << phoenix::construct< std::string >( errorPos, end ) << "\""
        << std::endl ;
}


template < typename Iterator >
Deska::CLI::PredefinedRules< Iterator >::PredefinedRules()
{
    qi::rule< Iterator, int(), ascii::space_type > t_int;
    t_int %= boost::spirit::qi::int_;
    t_int.name( "integer" );
    rulesMap[ "integer" ] = t_int;
    
    qi::rule< Iterator, std::string(), ascii::space_type > t_string;
    t_string %= boost::spirit::qi::lexeme[ '"' >> +( boost::spirit::ascii::char_ - '"' ) >> '"' ];
    t_string.name( "quoted string" );
    rulesMap[ "quoted_string" ] = t_string;
    
    qi::rule< Iterator, double(), ascii::space_type > t_double;
    t_double %= boost::spirit::qi::double_;
    t_double.name( "double" );
    rulesMap[ "double" ] = t_double;

    qi::rule< Iterator, std::string(), ascii::space_type > identifier;
    identifier %= boost::spirit::qi::lexeme[ *( boost::spirit::ascii::alnum | '_' ) ];
    identifier.name( "identifier (alphanumerical letters and _)" );
    rulesMap[ "identifier" ] = identifier;
}



template < typename Iterator >
boost::spirit::qi::rule< Iterator, boost::spirit::ascii::space_type > Deska::CLI::PredefinedRules< Iterator >::getRule( const std::string typeName )
{
    return rulesMap[ typeName ];
}



template void Deska::CLI::ErrorHandler< std::string::const_iterator >::operator()(
    std::string::const_iterator start,
    std::string::const_iterator end,
    std::string::const_iterator errorPos,
    const spirit::info& what ) const;

template Deska::CLI::PredefinedRules< std::string::const_iterator >::PredefinedRules();

template boost::spirit::qi::rule< std::string::const_iterator, boost::spirit::ascii::space_type > Deska::CLI::PredefinedRules< std::string::const_iterator >::getRule( const std::string typeName );
