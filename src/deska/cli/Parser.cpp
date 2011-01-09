#include "Parser.h"



template < typename Iterator >
void DeskaCLI::ErrorHandler< Iterator >::operator()(
    Iterator start,
    Iterator end,
    Iterator errorPos,
    const spirit::info& what ) const
{
    std::cout
        << "Error! Expecting " << what
        << " here: \"" << phoenix::construct< std::string >( errorPos, end ) << "\""
        << std::endl ;
};


template < typename Iterator >
DeskaCLI::PredefinedRules< Iterator >::PredefinedRules()
{
    t_int %= qi::int_;
    t_int.name( "integer" );
    
    t_string %= qi::lexeme[ '"' >> +( ascii::char_ - '"' ) >> '"' ];
    t_string.name( "quoted string" );
    
    t_double %= qi::double_;
    t_double.name( "double" );

    identifier %= qi::lexeme[ *( ascii::alnum | '_' ) ];
    identifier.name( "identifier (alphanumerical letters and _)" );
};



template void DeskaCLI::ErrorHandler< std::string::const_iterator >::operator()(
    std::string::const_iterator start,
    std::string::const_iterator end,
    std::string::const_iterator errorPos,
    const spirit::info& what ) const;

template DeskaCLI::PredefinedRules< std::string::const_iterator >::PredefinedRules();