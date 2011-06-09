/*
* Copyright (C) 2011 Tomáš Hubík <hubik.tomas@gmail.com>
*
* This file is part of the Deska, a tool for central administration of a grid site
* http://projects.flaska.net/projects/show/deska
*
* Inspired by SReadline
* Copyright (C) 2005 - 2006 Sergey Satskiy <sergesatsky@yahoo.com>
* http://freshmeat.net/projects/sreadline
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or the version 3 of the License.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; see the file COPYING.  If not, write to
* the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
* Boston, MA 02110-1301, USA.
* */

#ifndef DESKA_READLINE_WRAPPER_H
#define DESKA_READLINE_WRAPPER_H

#include <string>
#include <fstream>
#include <vector>

#include <readline/readline.h>
#include <readline/history.h>

namespace Deska
{
namespace ReadlineWrapper
{


/** @brief Custom completions generator */
class Completer
{
public:
    virtual ~Completer();
    
    /** @short Function for obtaining all possible lines.
    *
    *   @param line Line to be completed.
    *   @param start Beginning of the last word in the input.
    *   @param end Position of the input in the line.
    *   @return All possible lines, that can occur at this point. Whole lines will be generated, not only the endings.
    */
    virtual std::vector<std::string> getCompletions(const std::string &line,
                                                    std::string::const_iterator start,
                                                    std::string::const_iterator end) = 0;
};



/** @short Singleton class used to process and generate line completions. */
class CompletionHelper
{
public:

    /** @short Gets an instance of the singleton.
    *
    *   @return An instance of the singleton.
    */
    static CompletionHelper *getInstance();

    /** @short Sets the pointer to generator of possible lines.
    *
    *   @param customUserCompleter Pointer to the custom completer.
    *   @see Completer
    */
    void setUserCompleter(Completer *customUserCompleter);

    /** @short The function is called before trying to complete a token.
    *
    *   @param text A token to be completed
    *   @param start Position of the beginning of the token in the readline buffer
    *   @param end Position of the end of the token in the readline buffer
    */
    char **hGenerateCompletions(const char *text, int start, int end);

    /** @short Custom completion generator.
    *
    *   Used by generateCompletions() function.
    *
    *   @param text Pointer to a token to be completed
    *   @param State 0 for a first call, non 0 for all consequent calls
    *   @see generateCompletions();
    */
    char *hCompletionsGenerator(const char *text, int state);

private:
    CompletionHelper(int _x);
    CompletionHelper(const CompletionHelper&);
    CompletionHelper& operator=(const CompletionHelper&);
    ~CompletionHelper();

    /** @short Converts given string to list of tokens.
    *
    *   @param line String to be converted.
    *   @return Vector of tokens extracted from the given line.
    */
    std::vector<std::string> tokenize(const std::string &line);

    /** @short Compares two lists of tokens.
    *
    *   @param pattern Pattern tokens.
    *   @param input Input tokens.
    *   @return True if the pattern list contains tokens from input list (input list can be shorter).
    */
    bool tokensEqual(const std::vector<std::string> &pattern, const std::vector<std::string> &input);

    //@{
    /** Variables used in hCompletionsGenerator() that have to be kept for multiple calls. */
    int textlength;
    std::vector<std::vector<std::string> >::const_iterator completionsIterator;
    //@}

    /** Global storage of custom completions (whole lines). */
    std::vector<std::vector<std::string> > lineCompletions;
    /** Tokens storage for a single completion session. */
    std::vector<std::string> lineTokens;
    /** User custom generator of possible lines. */
    Completer* userCompleter;

    int x;
};



/** @short The readline library wrapper.
*
*   Supports editing, history and custom completers.
*/
class Readline
{
public:
    /** @short Sets the completion function to the readline and loads history.
    *
    *   @param historyStorageFileName File name where the history is stored at the beginning
               of the session and where will be saved at the end
    *   @param maxHistoryEntries Number of lines to be remembered as the history
    *   @param customCompleter Class from where the possible lines will be obtained
    *   @see Completer
    */
    Readline(const std::string &historyStorageFileName, const size_t maxHistoryEntries, Completer *customCompleter);

    /** @short Saves the history. */
    ~Readline();

    /** @short Prints prompt and gets a single line from a input
    *
    *   @param prompt Prompt string
    *   @return Read line
    */
    std::string getLine(const std::string &prompt);

    /** @short Saves the history to the given file
    *
    *   @param fileName File name where to save the history
    *   @return True if saving was successful, else false
    */
    bool saveHistory(const std::string &fileName);

    /** @short Loads a history from the given file
    *
    *   @param fileName File name where the history is stored
    *   @return True if loading was successful, else false
    */
    bool loadHistory(const std::string &fileName);

private:
    /** Number of lines to be remembered as the history. */
    size_t historyLimit;
    /** File name where the history is stored at the beginning of the session and where will be saved at the end. */
    std::string historyFileName;
};

}
}

#endif  // DESKA_READLINE_WRAPPER_H
