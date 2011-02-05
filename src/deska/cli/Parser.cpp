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


template<typename Iterator>
Deska::CLI::IfaceGrammar<Iterator>::IfaceGrammar(): IfaceGrammar::base_type( start )
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
    // This hell is here only for testing of the concept. The whole class will be deleted soon.
    keyword.add( "name", new boost::spirit::qi::rule< Iterator, boost::spirit::ascii::space_type >( predefined.getRule( "string" ) ) );
    keyword.add( "id", new boost::spirit::qi::rule< Iterator, boost::spirit::ascii::space_type >( predefined.getRule( "integer" ) ) );
    keyword.add( "ip", new boost::spirit::qi::rule< Iterator, boost::spirit::ascii::space_type >( predefined.getRule( "string" ) ) );

    // Head of top-level grammar
    cat_start %= lit( "interface" ) > predefined.getRule( "identifier" );

    // Trick for building the parser during parse time
    start = cat_start >> +( keyword[ _a = _1 ] >> lazy( *_a ) ) >> lit( "end" );
}

template<typename Iterator>
Deska::CLI::HardwareGrammar<Iterator>::HardwareGrammar(): HardwareGrammar::base_type( start )
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
    keyword.add( "name", new boost::spirit::qi::rule< Iterator, boost::spirit::ascii::space_type >( predefined.getRule( "string" ) ) );
    keyword.add( "id", new boost::spirit::qi::rule< Iterator, boost::spirit::ascii::space_type >( predefined.getRule( "integer" ) ) );
    keyword.add( "price", new boost::spirit::qi::rule< Iterator, boost::spirit::ascii::space_type >( predefined.getRule( "double" ) ) );

    // TODO: Problem, that grammars are non-copyable objects -> wrapping to phoenix::ref() or something
    //qi::rule< Iterator, ascii::space_type, qi::locals< qi::rule< Iterator, ascii::space_type >* > > iface = IfaceGrammar< Iterator >();
    //IfaceGrammar< Iterator > iface = IfaceGrammar< Iterator >();
   // nested.add( "interface", &iface );

    // Head of top-level grammar
    cat_start %= lit( "hardware" ) > predefined.getRule( "identifier" )[ std::cout << "Parsed: " << _1 << "\n" ] ;
    cat_start.name("category start");

    // Trick for building the parser during parse time
    // TODO: Problem, that grammars are non-copyable objects -> wrapping to phoenix::ref() or something
    start = ( cat_start > +( ( keyword[ _a = _1 ] > lazy( *_a )[ std::cout << "Parsed: " << _1 << "\n" ] ) /*|| ( nested[ _a = _1 ] >> lazy( *_a ) )*/ ) > lit( "end" ) );

    phoenix::function< ErrorHandler< Iterator> > wrappedError = ErrorHandler< Iterator >();
    on_error< fail >( start, wrappedError( _1, _2, _3, _4 ) );
}


template < typename Iterator >
Deska::CLI::KindGrammar< Iterator >::KindGrammar(
    const std::string kindName,
    boost::spirit::qi::rule< Iterator, std::string(), boost::spirit::ascii::space_type > identifierParser ): KindGrammar< Iterator >::base_type( start )
{ 
    using qi::_1;
    using qi::_2;
    using qi::_3;
    using qi::_4;
    using qi::_a;
    using qi::_val;
    using qi::on_error;
    using qi::fail;
    using qi::lit;

    name = kindName;

    identifierP = identifierParser;
    identifierP.name("kind name"); 

    //TODO only test - will be deleted
    KindGrammar neco( "bla" , boost::spirit::qi::lexeme[ *( boost::spirit::ascii::alnum | '_' ) ] );
    qi::rule< Iterator, ascii::space_type > neco2;
    neco2 = neco;

    // Trick for building the parser during parse time
    // TODO: Problem, that grammars are non-copyable objects -> wrapping to phoenix::ref() or something
   /* start = ( identifierP > +(
        ( attributes[ _a = _1 ] > lazy( *_a )[ std::cout << "Parsed: " << _1 << "\n" ] ) ||
        ( nestedGrammars[ _a = _1 ] > lazy( *_a ) ) ) > lit( "end" ) );*/

    phoenix::function< ErrorHandler< Iterator> > wrappedError = ErrorHandler< Iterator >();
    on_error< fail >( start, wrappedError( _1, _2, _3, _4 ) );
}



template < typename Iterator >
void Deska::CLI::KindGrammar< Iterator >::addAtrribute(
    const std::string attributeName,
    boost::spirit::qi::rule< Iterator, boost::spirit::ascii::space_type > attributeParser )
{
    attributesArray.push_back( attributeParser );
    attributes.add( attributeName, &( *( attributesArray.end() - 1 ) ) );
}



template < typename Iterator >
void Deska::CLI::KindGrammar< Iterator >::addNestedKind(
    const std::string kindName,
    qi::grammar<
        Iterator,
        ascii::space_type,
        qi::locals< qi::rule< Iterator, ascii::space_type >* > >* kindParser )
    //const Deska::CLI::KindGrammar< Iterator >* kindParser )
{
    nestedGrammars.add( kindName, kindParser );
}



template < typename Iterator >
std::string Deska::CLI::KindGrammar< Iterator >::getName() const
{
   return name;
}



template < typename Iterator >
Deska::CLI::MainGrammar< Iterator >::MainGrammar(): MainGrammar< Iterator >::base_type( start )
{
    using qi::_1;
    using qi::_2;
    using qi::_3;
    using qi::_4;
    using qi::_a;
    using qi::_val;
    using qi::on_error;
    using qi::fail;

    // TODO: Problem, that grammars are non-copyable objects -> wrapping to phoenix::ref() or something
   // start = +( kindGrammars[ _a = _1 ] > lazy( *_a )[ std::cout << "Parsed: " << _1 << "\n" ] );

    phoenix::function< ErrorHandler< Iterator> > wrappedError = ErrorHandler< Iterator >();
    on_error< fail >( start, wrappedError( _1, _2, _3, _4 ) );
}



template < typename Iterator >
Deska::CLI::MainGrammar< Iterator >::~MainGrammar()
{
    for( typename std::vector< KindGrammar< Iterator >* >::iterator it = kindGrammarsArray.begin(); it != kindGrammarsArray.end(); ++it )
        delete *it;
}



template < typename Iterator >
void Deska::CLI::MainGrammar< Iterator >::addKindGrammar( KindGrammar< Iterator >* grammar )
{
    kindGrammars.add( grammar->getName(), grammar );
    kindGrammarsArray.push_back( grammar );
}



template < typename Iterator >
Deska::CLI::ParserBuilder< Iterator >::ParserBuilder( Api* DBApi )
{
    predefined = PredefinedRules< Iterator >();
    api = DBApi;
}


template < typename Iterator >
Deska::CLI::MainGrammar< Iterator >* Deska::CLI::ParserBuilder< Iterator >::buildParser()
{
    
    std::map< std::string, KindGrammar< Iterator >* > kindGrammars;
    
    // Build single parsers with attributes
    std::vector< std::string > kinds = api->kindNames();
    
    for( std::vector< std::string >::iterator it = kinds.begin(); it != kinds.end(); ++it )
    {
        kindGrammars[ *it ] = new KindGrammar< Iterator >( *it, predefined.getRule( "identifier" ) );

        std::vector< KindAttributeDataType > attributes = api->kindAttributes( *it );
        for( std::vector< KindAttributeDataType >::iterator it2 = attributes.begin(); it2 != attributes.end(); ++it2 )
        {
            kindGrammars[ *it ]->addAtrribute( it2->name, predefined.getRule( it2->type ) );
        }
    }

    // Embed parsers, that should be embedded
    for( std::vector< std::string >::iterator it = kinds.begin(); it != kinds.end(); ++it )
    {
        std::vector< ObjectRelation > relations = api->kindRelations( *it );
        for( std::vector< ObjectRelation >::iterator itRel = relations.begin(); itRel != relations.end(); ++itRel )
        {
            if( itRel->kind == RELATION_EMBED_INTO )
            {
                typename std::map< std::string, KindGrammar< Iterator >* >::iterator itEmb = kindGrammars.find( itRel->tableName );
                if( itEmb != kindGrammars.end() )
                {
                    itEmb->second->addNestedKind( *it, kindGrammars[ *it ] );
                }
            }
        }
    }

    // Build main grammar
    Deska::CLI::MainGrammar< Iterator >* grammar = new Deska::CLI::MainGrammar< Iterator >();
    for( typename std::map< std::string, KindGrammar< Iterator >* >::iterator it = kindGrammars.begin(); it != kindGrammars.end(); ++it )
    {
        grammar->addKindGrammar( it->second );
    }

    return grammar;
}



template < typename Iterator >
Deska::CLI::Parser< Iterator >::Parser()
{
    grammar = 0;
}



template < typename Iterator >
Deska::CLI::Parser< Iterator >::~Parser()
{
    if ( grammar != 0 )
        delete grammar;
}



template < typename Iterator >
void Deska::CLI::Parser< Iterator >::initParser( Api* DBApi )
{
    ParserBuilder< Iterator > builder( DBApi );
    grammar = builder.buildParser();
}



template < typename Iterator >
bool Deska::CLI::Parser< Iterator >::parse( Iterator iter, Iterator end )
{
    bool r = phrase_parse( iter, end, *grammar, boost::spirit::ascii::space );
    return r;
}



//TEMPLATE INSTANCES FOR LINKER

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
        qi::locals< qi::rule< std::string::const_iterator, ascii::space_type >* > >* kindParser );
   // const Deska::CLI::KindGrammar< std::string::const_iterator >* kindParser );

template std::string Deska::CLI::KindGrammar< std::string::const_iterator >::getName() const;

template Deska::CLI::MainGrammar< std::string::const_iterator >::MainGrammar();

template void Deska::CLI::MainGrammar< std::string::const_iterator >::addKindGrammar( KindGrammar< std::string::const_iterator >* grammar );

template Deska::CLI::ParserBuilder< std::string::const_iterator >::ParserBuilder( Api* DBApi );

template Deska::CLI::MainGrammar< std::string::const_iterator >* Deska::CLI::ParserBuilder< std::string::const_iterator >::buildParser();

template Deska::CLI::IfaceGrammar< std::string::const_iterator >::IfaceGrammar();

template Deska::CLI::HardwareGrammar< std::string::const_iterator >::HardwareGrammar();

template Deska::CLI::Parser< std::string::const_iterator >::Parser();

template Deska::CLI::Parser< std::string::const_iterator >::~Parser();

template void Deska::CLI::Parser< std::string::const_iterator >::initParser( Api* DBApi );

template bool Deska::CLI::Parser< std::string::const_iterator >::parse( std::string::const_iterator iter, std::string::const_iterator end );
