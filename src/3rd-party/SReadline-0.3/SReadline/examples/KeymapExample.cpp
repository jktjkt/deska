/*!
    @file  KeymapExample.cpp
    @brief Example of the SReadline wrapper usage with keymaps
 */
//
// Date:      11 April  2006
//            07 May    2006
//
// Copyright (c) Sergey Satskiy 2005 - 2006
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


    // User input prompt
const string        Prompt( "Please input your command> " );




    // First parameter is a file name to save / load the
    // commands history. The second is a max number of stored commands
    // Both parameters could be ommited. In this case the history size
    // will be 64 and no save/restore operations will be performed.
SReadline           Reader( "/tmp/.testhist", 32 );

    // Two keymaps to switch between them
SKeymap             Keymap1( emacs_standard_keymap );   // A usual keymap
SKeymap             Keymap2( false );                   // Printables are NOT bound




int  PrintInfo( int  Count, int  Key )
{
    cout << "Keymaps usage example v.1.0" << endl
         << "Press Ctrl-K to get back to the interactive mode." << endl;
    return 0;
}

int  SwitchToKeymap2( int  Count, int  Key )
{
    Reader.SetKeymap( Keymap2 );
    cout << endl
         << "Entered to non-interactive mode. Use Ctrl-I for information." << endl;
    return 0;
}
 
int  SwitchToKeymap1( int  Count, int  Key )
{
    Reader.SetKeymap( Keymap1 );
    cout << "Entering to interactive mode." << endl
         << "Press Ctrl-K to switch to the non-interactive mode." << endl;

        // Restore the command line as it was
    cout << Prompt << rl_line_buffer;
    return 0;
}
 


int  main( int  argc, char **  argv )
{ 
        // Prepare the keymap2
    Keymap2.Bind( 'a', PrintInfo );
    Keymap2.Bind( CTRL( 'I' ), PrintInfo );
    Keymap2.Bind( CTRL( 'K' ), SwitchToKeymap1 );

        // Bind a switch to keymap1
    Keymap1.Bind( CTRL( 'K' ), SwitchToKeymap2 );


        // Start with the usual keymap
    Reader.SetKeymap( Keymap1 );


    cout << "Use CTRL+K for switching keymaps" << endl;
    
        // Now we can ask user for a line
    string      UserInput;
    bool        EndOfInput( false );

    for ( ; ; )
    {
            // The last parameter could be ommited
        UserInput = Reader.GetLine( Prompt, EndOfInput );
        if ( EndOfInput )
        {
            cout << "End of the session. Exiting." << endl;
            break;
        }
        cout << "User input: '" << UserInput << "'." << endl;
        cout << "Press Ctrl+D for gracefull exit" << endl;
    }
        
    return 0;
}

