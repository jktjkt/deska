/*!
    @file  CommandsExample.cpp
    @brief Example of the SReadline wrapper usage to implement 
           a commands executor.
 */
//
// Date:      27 December  2005
//
// Copyright (c) Sergey Satskiy 2005
//               <sergesatsky@yahoo.com>
//
// Permission to copy, use, modify, sell and distribute this software 
// is granted provided this copyright notice appears in all copies. 
// This software is provided "as is" without express or implied
// warranty, and with no claim as to its suitability for any purpose.
//

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <list>
#include <algorithm>
using namespace std;


#include <boost/function.hpp>
#include <boost/functional.hpp>
using namespace boost;

    // The wrapper is in a single header file.
#include "SReadline.h"
using namespace swift;


    
    // The following typedefs and the struct are
    // for "advanced" completers
typedef function< void ( const vector< string > & ) >   Func;
typedef pair< string, Func >                            Element;

    
struct MyElement : public Element
{
    operator string () const { return first; }
    MyElement( const string & Arg1, Func  Arg2 ) : Element( Arg1, Arg2 ) {}
};

typedef list< MyElement >     MyContainer;


    // A functor to look for a command in the 
    // container of completers
class LookupFunctor
{
    public:
            // Creates a functor and memorises tokens
        LookupFunctor( const vector< string > &  tokens ) :
            Tokens( tokens )
        {}

            // Compares the first token only
        bool operator()( const MyElement &  Element ) const
        {
            return strncmp( Tokens.begin()->c_str(),
                            (static_cast< string >( Element )).c_str(),
                            Tokens.begin()->size() ) == 0;
        }

    private:
        const vector< string > &  Tokens;
};





    // The 'help' command processor
class SHelpExecutor
{
    public:
            // Memorises the list of completers
        SHelpExecutor( const MyContainer &  completers ) :
            Completers( completers )
        {}

            // Prints all the completers on the std out
        void operator() ( const vector< string > &  Unused )
        {
            cout << "List of the command variations:" << endl;
            for ( MyContainer::const_iterator  k( Completers.begin() );
                  k != Completers.end(); ++k )
            {
                cout << static_cast< string >( *k ) << endl;
            }
        }
    private:
        const MyContainer &   Completers;
};



    // The 'timeofday' command executor
    // It prints the number of seconds and microsecond since the Epoch
void TimeOfDay( const vector< string > &    Unused );
void TimeOfDay( const vector< string > &    Unused )
{
    struct timeval      tv;
    gettimeofday( &tv, 0 );
    cout << "gettimefday() reports: " << tv.tv_sec << '.' << tv.tv_usec << endl;
}


    // The 'filesize' command executor
    // It prints the size of the given file in bytes
void FileSize( const vector< string > &  Tokens );
void FileSize( const vector< string > &  Tokens )
{
    if ( Tokens.size() != 2 )
    {
        cout << "The filesize command supposes the file name" << endl;
        return;
    }

    struct stat     Stat;
    if ( stat( Tokens[1].c_str(), &Stat ) != 0 )
    {
        cout << "Cannot get info for the given '" 
             << Tokens[1] << "' file. Check the name." << endl;
        return;
    }
    
    cout << "File '" << Tokens[1] << "' size: " 
         << Stat.st_size << " bytes" << endl;
}




int  main( int  argc, char **  argv )
{ 
        // First parameter is a file name to save / load the
        // commands history. The second is a max number of stored commands
        // Both parameters could be ommited. In this case the history size
        // will be 64 and no save/restore operations will be performed.
    SReadline           Reader( "/tmp/.testhist", 32 );

        // Completers with functions to be called
    MyContainer         AdvancedCompleters;

        // Help executor functor
    SHelpExecutor       HelpExecutor( AdvancedCompleters );


        // The following is supported:
        // - "identifiers"
        // - special identifier %file - means to perform a file name completion
    AdvancedCompleters.push_back( MyElement( "help", 
                                             boost::bind1st( 
                                                 boost::mem_fun( &SHelpExecutor::operator() 
                                                               ), &HelpExecutor 
                                                           ) 
                                           ) 
                                );
    AdvancedCompleters.push_back( MyElement( "exit", 0 ) );
    AdvancedCompleters.push_back( MyElement( "timeofday", TimeOfDay ) );
    AdvancedCompleters.push_back( MyElement( "filesize %file", FileSize ) );


        // Now register the completers.
        // Actually it is possible to re-register another set at any time
    Reader.RegisterCompletions( AdvancedCompleters );
        
    
        // true when we should exit
    bool                    EndOfInput( false );
        // List of the entered tokens
    vector< string >        Tokens;
        // Search iterator
    MyContainer::iterator   Found( AdvancedCompleters.end() );


    for ( ; ; )
    {
            // We get the list of tokens
        Reader.GetLine( "Please input your command> ", Tokens, EndOfInput );
        if ( EndOfInput )
        {
            cout << "End of the session. Exiting." << endl;
            break;
        }

        if ( !Tokens.empty() )
        {
            if ( *Tokens.begin() == "exit" )    break;

            Found = find_if( AdvancedCompleters.begin(), AdvancedCompleters.end(), 
                             LookupFunctor( Tokens ) );

                // If found the first token in the list of completers -
                // call the stored function
            if ( Found != AdvancedCompleters.end() )
            {
                if ( Found->second != 0 ) Found->second( Tokens );
            }
            else
            {
                cout << "Unknown command. Type 'help' for the list of acceptable commands" 
                     << endl;
            }
        }
        
        cout << "Press Ctrl+D for gracefull exit" << endl;
    }
        
    return 0;
}

