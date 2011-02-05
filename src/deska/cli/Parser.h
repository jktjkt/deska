#ifndef DESKA_PARSER_H
#define DESKA_PARSER_H

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

#include "deska/db/Api.h"

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
    class IfaceGrammar: public qi::grammar< Iterator, ascii::space_type, qi::locals< qi::rule< Iterator, ascii::space_type >* > >
    {
    public:
        IfaceGrammar();

        qi::symbols< char, qi::rule< Iterator, ascii::space_type >* > keyword;
        qi::symbols< char, qi::grammar< Iterator, ascii::space_type, qi::locals< qi::rule< Iterator, ascii::space_type >* > >* > nested;

        qi::rule< Iterator, std::string(), std::string(), ascii::space_type > cat_start;
        qi::rule< Iterator, ascii::space_type, qi::locals< qi::rule< Iterator, ascii::space_type >* > > start;

        PredefinedRules< Iterator > predefined;
    };

    template <typename Iterator>
    class HardwareGrammar: public qi::grammar< Iterator, ascii::space_type, qi::locals< qi::rule< Iterator, ascii::space_type >* > >
    {
    public:
        HardwareGrammar();

        qi::symbols< char, qi::rule< Iterator, ascii::space_type >* > keyword;
        qi::symbols< char, qi::rule< Iterator, ascii::space_type, qi::locals< qi::rule< Iterator, ascii::space_type >* > >* > nested;
        
        qi::rule< Iterator, std::string(), std::string(), ascii::space_type > cat_start;
        qi::rule< Iterator, ascii::space_type, qi::locals< qi::rule< Iterator, ascii::space_type >* > > start;

        PredefinedRules< Iterator > predefined;
    };



    template <typename Iterator>
    class KindGrammar: public qi::grammar< Iterator, ascii::space_type, qi::locals< qi::rule< Iterator, ascii::space_type >* > >
    {
    public:
        KindGrammar( const std::string kindName, qi::rule< Iterator, std::string(), ascii::space_type > identifierParser );

        void addAtrribute(
            const std::string attributeName,
            qi::rule< Iterator, ascii::space_type > attributeParser );
        void addNestedKind(
            const std::string kindName,
            qi::grammar< Iterator, ascii::space_type, qi::locals< qi::rule< Iterator, ascii::space_type >* > >* kindParser );
            //const KindGrammar* kindParser );

        std::string getName() const;

    private:
        qi::symbols<
            char,
            qi::rule< Iterator, ascii::space_type >* > attributes;

//        qi::symbols<
//            char,
//            qi::rule<
//                Iterator,
//                ascii::space_type,
//                qi::locals< qi::rule< Iterator, ascii::space_type >* > >* > nestedGrammars;

        qi::symbols<
            char,
            qi::grammar<
            Iterator,
            ascii::space_type,
            qi::locals< qi::rule< Iterator, ascii::space_type >* > >* > nestedGrammars;
        
        qi::rule< Iterator, std::string(), std::string(), ascii::space_type > identifierP;
        qi::rule< Iterator, ascii::space_type, qi::locals< qi::rule< Iterator, ascii::space_type >* > > start;

        std::string name;

        std::vector< qi::rule< Iterator, ascii::space_type > > attributesArray;
    };



    template <typename Iterator>
    class MainGrammar: public qi::grammar< Iterator, ascii::space_type, qi::locals< qi::rule< Iterator, ascii::space_type >* > >
    {
    public:
        MainGrammar();
        ~MainGrammar();

        void addKindGrammar( KindGrammar< Iterator >* grammar );

    private:
        qi::symbols<
            char,
            qi::grammar<
                Iterator,
                ascii::space_type,
                qi::locals< qi::rule< Iterator, ascii::space_type >* > >* > kindGrammars;

        qi::rule< Iterator, ascii::space_type, qi::locals< qi::rule< Iterator, ascii::space_type >* > > start;

        std::vector< KindGrammar< Iterator >* > kindGrammarsArray;
    };



    template <typename Iterator>
    class ParserBuilder
    {
    public:
        ParserBuilder( Api* DBApi );
        MainGrammar< Iterator >* buildParser();

    private:
        PredefinedRules< Iterator > predefined;
        Api* api;
    };



    template <typename Iterator>
    class Parser
    {
    public:
        Parser();
        ~Parser();

        void initParser( Api* DBApi );
        bool parse( Iterator iter, Iterator end );
        
    private:
        MainGrammar< Iterator >* grammar;
    };

}

}



#endif  //DESKA_PARSER_H
