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
    rulesMap[ "integer" ] = t_int.alias();
    
    qi::rule< Iterator, std::string(), ascii::space_type > t_string;
    t_string %= boost::spirit::qi::lexeme[ '"' >> +( boost::spirit::ascii::char_ - '"' ) >> '"' ];
    t_string.name( "quoted string" );
    rulesMap[ "quoted_string" ] = t_string.alias();
    
    qi::rule< Iterator, double(), ascii::space_type > t_double;
    t_double %= boost::spirit::qi::double_;
    t_double.name( "double" );
    rulesMap[ "double" ] = t_double.alias();

    qi::rule< Iterator, std::string(), ascii::space_type > identifier;
    identifier %= boost::spirit::qi::lexeme[ *( boost::spirit::ascii::alnum | '_' ) ];
    identifier.name( "identifier (alphanumerical letters and _)" );
    rulesMap[ "identifier" ] = identifier.alias();
}



template < typename Iterator >
boost::spirit::qi::rule< Iterator, boost::spirit::ascii::space_type > Deska::CLI::PredefinedRules< Iterator >::getRule( const std::string typeName )
{
    return rulesMap[ typeName ].alias();
}



template < typename Iterator >
Deska::CLI::KindGrammar< Iterator >::KindGrammar(
    const std::string kindName,
    boost::spirit::qi::rule< Iterator, std::string(), boost::spirit::ascii::space_type > identifierParser ): KindGrammar< Iterator >::base_type( start )
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

    name = kindName;

    identifierP = identifierParser;
    identifierP.name("kind name");

    // Trick for building the parser during parse time
    // TODO: Problem, that grammars are non-copyable objects -> wrapping to phoenix::ref() or something
    start = ( identifierParser > +(
        ( attributes[ _a = _1 ] > lazy( _a )[ std::cout << "Parsed: " << _1 << "\n" ] ) ||
        ( nestedGrammars[ _a = _1 ] > lazy( _a ) ) ) > lit( "end" ) );

    phoenix::function< ErrorHandler< Iterator> > wrappedError = ErrorHandler< Iterator >();
    on_error< fail >( start, wrappedError( _1, _2, _3, _4 ) );
}



template < typename Iterator >
void Deska::CLI::KindGrammar< Iterator >::addAtrribute(
    const std::string attributeName,
    boost::spirit::qi::rule< Iterator, boost::spirit::ascii::space_type > attributeParser )
{
    attributes.add( attributeName, attributeParser );
}



template < typename Iterator >
void Deska::CLI::KindGrammar< Iterator >::addNestedKind(
    const std::string kindName,
    qi::grammar<
        Iterator,
        ascii::space_type,
        qi::locals< qi::rule< Iterator, ascii::space_type > > > kindParser )
{
    nestedGrammars.add( kindName, kindParser );
}



template void Deska::CLI::ErrorHandler< std::string::const_iterator >::operator()(
    std::string::const_iterator start,
    std::string::const_iterator end,
    std::string::const_iterator errorPos,
    const spirit::info& what ) const;

template Deska::CLI::PredefinedRules< std::string::const_iterator >::PredefinedRules();

template boost::spirit::qi::rule< std::string::const_iterator, boost::spirit::ascii::space_type > Deska::CLI::PredefinedRules< std::string::const_iterator >::getRule( const std::string typeName );

template Deska::CLI::KindGrammar< std::string::const_iterator >::KindGrammar(
    const std::string kindName,
    boost::spirit::qi::rule< std::string::const_iterator, std::string(), boost::spirit::ascii::space_type > identifierParser );

template void Deska::CLI::KindGrammar< std::string::const_iterator >::addAtrribute(
    const std::string attributeName,
    boost::spirit::qi::rule< std::string::const_iterator, boost::spirit::ascii::space_type > attributeParser );

template void Deska::CLI::KindGrammar< std::string::const_iterator >::addNestedKind(
    const std::string kindName,
    qi::grammar<
        std::string::const_iterator,
        ascii::space_type,
        qi::locals< qi::rule< std::string::const_iterator, ascii::space_type > > > kindParser );
