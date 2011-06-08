/*
* Copyright (C) 2011 Tomáš Hubík <hubik.tomas@gmail.com>
*
* This file is part of the Deska, a tool for central administration of a grid site
* http://projects.flaska.net/projects/show/deska
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

#include <boost/algorithm/string/trim.hpp>
#include <boost/tokenizer.hpp>

#include "ReadlineWrapper.h"


namespace Deska
{
namespace ReadlineWrapper
{


Completer::~Completer()
{
}



Readline::Readline(const std::string &historyStorageFileName, const size_t maxHistoryEntries, Completer *customCompleter):
    historyLimit(maxHistoryEntries),
    historyFileName(historyStorageFileName)
{
    using_history();
    loadHistory(historyFileName);
    rl_attempted_completion_function = generateCompletions;
    CompletionHelper::getInstance()->setUserCompleter(customCompleter);
}



Readline::~Readline()
{
    saveHistory(historyFileName);
}



std::string Readline::getLine(const std::string &prompt)
{
    char* cline(readline(prompt.c_str()));
    if (cline == 0)
        return std::string();
    std::string line(cline);
    free(cline);
    boost::algorithm::trim(line);
    if (!line.empty()) {
        if ((history_length == 0) || (line != history_list()[history_length - 1]->line)) {
            add_history(line.c_str());
            stifle_history(historyLimit);
        }
    }
    return line;
}



bool Readline::saveHistory(const std::string &fileName)
{
    if (fileName.empty())
        return false;
    std::ofstream  ofs(fileName.c_str());
    for (int i = 0; i < history_length; ++i)
    {
        ofs << history_list()[i]->line << std::endl;
    }
    return true;
}



bool Readline::loadHistory(const std::string &fileName)
{
    if (fileName.empty())
        return false;
    std::ifstream ifs(fileName.c_str());
    if (!ifs)
        return false;
    clear_history();
    std::string histLine;
    while (!getline(ifs, histLine).eof()) {
        boost::algorithm::trim(histLine);
        if ((history_length == 0) || (histLine != history_list()[history_length - 1]->line)) {
            add_history(histLine.c_str());
        }
    }
    stifle_history(historyLimit);
    return true;
}



CompletionHelper *CompletionHelper::getInstance()
{
    static CompletionHelper inst(0);
    return &inst;
}



void CompletionHelper::setUserCompleter(Completer *customUserCompleter)
{
    userCompleter = customUserCompleter;
}



char **CompletionHelper::hGenerateCompletions(const char *text, int start, int end)
{
    // Disabling of default completion
    rl_attempted_completion_over = 1;
    
    std::string complLine(rl_line_buffer);
    std::string::const_iterator iStart = complLine.begin() + start;
    std::string::const_iterator iEnd = complLine.begin() + end;
    std::vector<std::string> customCompletions = userCompleter->getCompletions(complLine, iStart, iEnd);
    completions.clear();
    for (std::vector<std::string>::const_iterator it = customCompletions.begin();
         it != customCompletions.end(); ++it) {
        completions.push_back(tokenize(*it));
    }
    
    if (completions.empty())
        return 0;

    std::string preInput(rl_line_buffer, start);
    tokens = tokenize(preInput);

    for (std::vector<std::vector<std::string> >::const_iterator it = completions.begin(); it != completions.end(); ++it)
    {
        if ((!tokensEqual(*it,tokens)) | (it->size() <= tokens.size()))
            continue;
        if (it->at(tokens.size()) == "%file") {
            // Standard file name completer called for the "%file" keyword
            return rl_completion_matches( text, rl_filename_completion_function );
        }
        return rl_completion_matches(text, completionsGenerator);
    }

    return 0;
}



char *CompletionHelper::hCompletionsGenerator(const char *text, int state)
{
    if (state == 0) {
        completionsIterator = completions.begin();
        textlength = strlen(text);
    }

    for (; completionsIterator != completions.end(); ++completionsIterator) {
        if ((!tokensEqual(*completionsIterator, tokens)) |
            (completionsIterator->size() <= tokens.size()) |
            (completionsIterator->at(tokens.size()) == "%file" ))
            continue;
        if (strncmp(text, completionsIterator->at(tokens.size()).c_str(), textlength) == 0) {
            // readline will free the allocated memory
            char *complString((char*)malloc(strlen(completionsIterator->at(tokens.size()).c_str()) + 1));
            strcpy(complString, completionsIterator->at(tokens.size()).c_str());
            ++completionsIterator;
            return complString;
        }
    }
    return 0;
}



CompletionHelper::CompletionHelper(int _x): x(_x)
{
    textlength = 0;
    completionsIterator = completions.begin();
}



CompletionHelper::~CompletionHelper()
{
}



std::vector<std::string> CompletionHelper::tokenize(const std::string &line)
{
    std::vector<std::string> tokens;
    boost::char_separator<char> separators(" \t\n");
    boost::tokenizer<boost::char_separator<char> > tokenizer(line, separators);
    std::string token;
    
    for (boost::tokenizer<boost::char_separator<char> >::const_iterator it = tokenizer.begin();
         it != tokenizer.end(); ++it) {
        token = *it;
        boost::algorithm::trim(token);
        tokens.push_back(token);
    }

    return tokens;
}



bool CompletionHelper::tokensEqual(const std::vector<std::string> &pattern, const std::vector<std::string> &input)
{
    if (input.size() > pattern.size())
        return false;
    std::vector<std::string>::const_iterator p = pattern.begin();
    std::vector<std::string>::const_iterator i = input.begin();
    for (; i != input.end(); ++p, ++i) {
        if (*p == "%file")
            continue;
        if (*p != *i)
            return false;
    }
    return true;
}



namespace
{

char **generateCompletions(const char *text, int start, int end)
{
    return CompletionHelper::getInstance()->hGenerateCompletions(text, start, end);
}



char *completionsGenerator(const char *text, int state)
{
    return CompletionHelper::getInstance()->hCompletionsGenerator(text, state);
}

}


}
}