/*!
    @file  BasicExample.cpp
    @brief Example of the basic SReadline wrapper usage
 */
//
// Date:      22 December  2005
//
// Copyright (c) Sergey Satskiy 2005
//               <sergesatsky@yahoo.com>
//
// Permission to copy, use, modify, sell and distribute this software 
// is granted provided this copyright notice appears in all copies. 
// This software is provided "as is" without express or implied
// warranty, and with no claim as to its suitability for any purpose.
//


#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <list>
using namespace std;

    // The wrapper is in a single header file.
#include "SReadline.h"
using namespace swift;

    
 

int  main( int  argc, char **  argv )
{ 
        // First parameter is a file name to save / load the
        // commands history. The second is a max number of stored commands
        // Both parameters could be ommited. In this case the history size
        // will be 64 and no save/restore operations will be performed.
    SReadline           Reader( "/tmp/.testhist", 32 );


        // Prepare the list of my own completers
    vector< string >    Completers;
        
        // The following is supported:
        // - "identifiers"
        // - special identifier %file - means to perform a file name completion
    Completers.push_back( "command1 opt1" );
    Completers.push_back( "command1 opt2" );
    Completers.push_back( "command1 opt3 %file" );
    Completers.push_back( "command2 opt4" );
    Completers.push_back( "command2 opt5 %file %file" );


        // Now register the completers.
        // Actually it is possible to re-register another set at any time
    Reader.RegisterCompletions( Completers );
        
        // Now we can ask user for a line
    string      UserInput;
    bool        EndOfInput( false );

    for ( ; ; )
    {
            // The last parameter could be ommited
        UserInput = Reader.GetLine( "Please input your command> ", EndOfInput );
        if ( EndOfInput )
        {
            cout << "End of the session. Exiting." << endl;
            break;
        }
        cout << "User input: '" << UserInput << "'." << endl;
        cout << "Press Ctrl+D for gracefull exit" << endl;
    }
        
        // The history could be saved to an arbitrary file at any time
    Reader.SaveHistory( "/tmp/BackupFileJustInCase" );

        // The history could be cleared
    Reader.ClearHistory();

        // And the history could be loaded at any time
    Reader.LoadHistory( "/tmp/BackupFileJustInCase" );

    return 0;
}

