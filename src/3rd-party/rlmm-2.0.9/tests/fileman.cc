//
// $Id: fileman.cc,v 1.6 2005/08/15 15:42:59 cholm Exp $ 
//  
//  simple.cc
//  Copyright (C) 2002 Christian Holm Christensen <cholm@nbi.dk> 
//
//  This library is free software; you can redistribute it and/or 
//  modify it under the terms of the GNU Lesser General Public License 
//  as published by the Free Software Foundation; either version 2.1 
//  of the License, or (at your option) any later version. 
//
//  This library is distributed in the hope that it will be useful, 
//  but WITHOUT ANY WARRANTY; without even the implied warranty of 
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
//  Lesser General Public License for more details. 
// 
//  You should have received a copy of the GNU Lesser General Public 
//  License along with this library; if not, write to the Free 
//  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 
//  02111-1307 USA 
//

#ifdef HAVE_CONFIG_H
# include <config.hh>
#endif
#ifndef rlmm_readline_hh
# include <rlmm/readline.hh>
#endif
#ifndef rlmm_buffer_hh
# include <rlmm/buffer.hh>
#endif
#ifndef rlmm_completion_hh
# include <rlmm/completion.hh>
#endif
#ifndef rlmm_terminal_hh
# include <rlmm/terminal.hh>
#endif
#ifndef rlmm_history_hh
# include <rlmm/history.hh>
#endif
#ifndef __IOSTREAM__
# include <iostream>
#endif
#ifndef __IOMANIP__
# include <iomanip>
#endif
#ifndef __SSTREAM__
# include <sstream>
#endif
#ifndef __LIST__
# include <list>
#endif
#include <sys/types.h>
#ifdef HAVE_SYS_FILE_H
# include <sys/file.h>
#endif
#include <sys/stat.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifndef __CERRNO__
# include <cerrno>
#endif
#if defined(__GNUC__) && __GNUC__ <= 2
# define CMP_ARG(x) x, size_t(0), x.size()
#else
# define CMP_ARG(x) size_t(0), x.size(), x
#endif
#define HIST_FILE ".fileman_history"
typedef struct stat stats;


/** @file   fileman.cc
    @author Christian Holm
    @date   Mon Oct  7 14:07:38 2002
    @brief  Elaborate example of using rlmm. */

using namespace std;
using namespace rlmm;

//____________________________________________________________________
namespace examples
{
  /** @namespace examples::fileman 
      @brief Namespace for file manager. */
  namespace fileman 
  {
    //__________________________________________________________________
    /// ABC for command classes.
    struct command 
    {
      /// Name of this command 
      const string _name;
      /// Help for this command 
      const string _help;
      /// CTOR
      command(const string& name=string(), const string& help=string()) 
	: _name(name), _help(help) {}
      /// What to do
      virtual bool operator()(const std::string& arg) = 0;
      /// Check argument
      bool valid_arg(const std::string& arg) {
	return arg.empty() ? false : true; }
    };
  

    //__________________________________________________________________
    // Forward declaration
    class manager;

    //__________________________________________________________________
    /// Custom class for completion. 
    class completer : public completion 
    {
    private:
      /// Pointer to file manager object
      manager* _manager;
      /// Possible completions 
      std::list<command*>::const_iterator _current;
      /// Are we at start of line
      bool _start;
    public: 
      /** CTOR.  Set internal pointer. */
      completer(manager* fm) : _manager(fm) { this->activate(); }
      /** Construct list of posibilities. 
	  This just reinitialises the iterator to point at the beginning
	  of the command list.
	  @param what What  to do completion for. */
      void possibilities(const string& what);
      /** Return next completion. 
	  If we're doing command completion (_start is true), then this
	  loops until it finds a (possible) partial match in the command
	  list and then return the name of the command.  Otherwise if
	  calls the rlmm::completion::filename_complete member
	  function to do 
	  file name completion. 
	  @param what what to match. 
	  @return name of command partial match found, or empty
	  string. */ 
      string complete(const string& what);
      /** The first thing to try. 
	  If we at the beginning of a line (@p start is 0) then we do
	  command completion via normal completion functions.  Otherwise
	  we do filename completion. 
	  @param what What to complete. 
	  @param start position in line of @p what. 
	  @param stop end position in line of @p what.
	  @return a list of possible completions. */ 
      std::list<std::string> alternate_completer(const string& what, 
						 int start, int stop);
    };

    //__________________________________________________________________
    /// File manager class.
    class manager : public rlmm::readline 
    {
    private:
      /// Pointer to completion object.  
      completion*  _completer;
      /// Pointer to history object.
      history* _history;
      /// List of known commands. 
      std::list<fileman::command*> _commands;
      /// Are we done?
      bool _done;
    public:
      /// CTOR.  Does various setup.
      manager();
      /// CTOR. Does nothing. 
      virtual ~manager() { _history->write(HIST_FILE); }
      /// Main method. 
      int run();
      /// Show help.
      bool help(const string& arg);
      /// Quit 
      bool quit() { return _done = true; }
      /// Get a reference to the list of commands 
      const list<fileman::command*>& commands() const { return _commands; }
      /// Make the current prompt
      string make_prompt();
    };

    //==================================================================
    /// ABC for dangerous functions. 
    struct danger : public command 
    {
      /** CTOR
	  @param name Name of dangerous function. 
	  @param help Help string for dangerous function. */
      danger(const string& name=string(), const string& help=string()) 
	: command(name, help) {}
      /// Make a warning. 
      bool operator() (const std::string& arg) {
	std::cerr << "DANGER: " << _name
		  << " too dangerous to distribute" << std::endl;
	return false;
      }
    };
  
    //__________________________________________________________________
    /// List directory. 
    struct ls : public command
    {
      /// CTOR
      ls() : command("ls", "list files in directory") {}
      /** Do the listing. 
	  @param arg Argument to system @c ls. */
      bool operator() (const std::string& arg) {
	string c("ls -FClg ");
	c.append(arg);
	return system(c.c_str()) == 0 ? true : false;
      }
    };
  
    //__________________________________________________________________
    /// Show contents of a file. 
    struct more : public command
    {
      /// CTOR
      more() : command("more", "View contents of  a file") {}
      /** Do the listing. 
	  @param arg Argument to system @c more (a filename). */
      bool operator() (const std::string& arg) {
	if (!valid_arg(arg)) 
	  return false;
	string c("more ");
	c.append(arg);
	return system(c.c_str()) == 0 ? true : false;
      }
    };

    //__________________________________________________________________
    /// Statistics on a file
    struct stat : public command
    {
      /// CTOR
      stat() : command("stat", "Stat a file") {}
      /** Show the statistics. 
	  @param arg File to show stats for. */
      bool operator() (const std::string& arg) {
	if (!valid_arg(arg)) 
	  return false;
	stats s;
	if (::stat(arg.c_str(), &s) == -1) {
	  cerr << strerror(errno) << endl;
	  return false;
	}
	std::cout << "Statistics for " << arg << ":" << std::endl
		  << " " << arg << " has " 
		  << s.st_nlink << " link" << (s.st_nlink > 1 ? 's' : ' ') 
		  << " and size of " << s.st_size << " byte"
		  << (s.st_size > 1 ? 's' : ' ') << std::endl
		  << " Inode last change: " << ctime(&s.st_ctime)
		  << " Last access:       " << ctime(&s.st_atime)
		  << " Last Modified:     " << ctime(&s.st_mtime);
	return true;
      }
    };

    //__________________________________________________________________
    /// Show avaliable commands 
    struct help  : public command 
    {
      /// Pointer to manager object. 
      manager* _manager;
      /** CTOR
	  @param m Manager class. */
      help(manager* m) : command("help", "Show this help"), _manager(m) {}
      /** call examples::manager::help(arg);
	  @param arg Optional argument to help. */
      bool operator() (const std::string& arg) { 
	return _manager->help(arg);
      }
    
    };
  
    //__________________________________________________________________
    /// Print working directory
    struct pwd : public command
    {
      /// CTOR.
      pwd() : command("pwd", "Print working directory") {}
      /// Print it.
      bool operator() (const std::string& arg) 
      {
	static char dir[1024], *s;
	s = getcwd(dir, sizeof(dir) - 1);
	if (!s) {
	  cerr << "Error getting pwd: " << dir << endl;
	  return false;
	}
	cout << "Current directory: " << dir << endl;
	return true;
      }
    };

    //__________________________________________________________________
    /// Change directory. 
    struct cd : public command
    {
      /// CTOR
      cd() : command("cd", "Change directory") {}
      /** Do the change. 
	  @param arg directory to go to. */
      bool operator() (const std::string& arg) 
      {
	if (chdir(arg.c_str()) == -1) {
	  cerr << strerror(errno) << endl;
	  return false;
	}
	examples::fileman::pwd p;
	p("");
	return true;
      }
    };
  
    //__________________________________________________________________
    /// Quit the manager
    struct quit : public command 
    {
      /// Pointer to manager object.
      manager* _manager;
      /// CTOR.
      quit(manager* m) : command("quit","Quit the file manager"), 
			 _manager(m) {}
      /// Signal the manager to quit. 
      bool operator() (const std::string& arg) {
	return _manager->quit();
      }
    };

    //__________________________________________________________________
    /// Remove a file - too dangerous - does nothing. 
    struct rm : public danger
    {
      /// CTOR
      rm() : danger("rm", "Remove a file") {}
    };
  
    //__________________________________________________________________
    /// Move a file - too dangerous - does nothing. 
    struct mv : public danger
    {
      /// CTOR
      mv() : danger("mv", "Move a file") { }
    };
    //__________________________________________________________________
    /// Move a file - too dangerous - does nothing. 
    struct cp : public danger
    {
      /// CTOR
      cp() : danger("cp", "Copy a file") { }
    };
    //__________________________________________________________________
    /// Move a file - too dangerous - does nothing. 
    struct hist : public command
    {
      /// CTOR
      hist() : command("hist", "show history") { }
      bool operator() (const std::string& arg) 
      {
	int n = -1;
	if (!arg.empty()) {
	  std::stringstream ss(arg);
	  ss >> n;
	}
	rlmm::history& h = rlmm::history::active();
	const rlmm::history::entry_list& l = h.list();
	size_t j = 0;
	for (rlmm::history::entry_list::const_iterator i = l.begin(); 
	     i != l.end(); ++i, ++j) {
	  if (n >= 0 && n != j) continue;
	  std::cout << std::setw(5) << j << ": "
		    << (*i).time() << ":\t" << (*i).line() 
		    << std::endl;
	  if (n >= 0 && n == j) break;
	}
	return true;
      }
      
    };
  }
}

//____________________________________________________________________
examples::fileman::manager::manager() 
{
  name("fileman");
  _completer = new examples::fileman::completer(this);
  _history   = new history();
  _history->read(HIST_FILE);
  _history->activate();
  _done      = false;
  
  // register commands
  examples::fileman::help* h = new examples::fileman::help(this);
  examples::fileman::quit* q = new examples::fileman::quit(this);
  _commands.push_back(h);
  _commands.push_back(q);
  _commands.push_back(new ls);
  _commands.push_back(new fileman::more);
  _commands.push_back(new stat);
  _commands.push_back(new cd);
  _commands.push_back(new rm);
  _commands.push_back(new mv);
  _commands.push_back(new cp);
  _commands.push_back(new hist);

}

//____________________________________________________________________
bool examples::fileman::manager::help(const string& arg) 
{
  int printed = 0;
  for (list<fileman::command*>::iterator i = _commands.begin(); 
       i != _commands.end(); i++) {
    if (arg.empty() || !(*i)->_name.compare(CMP_ARG(arg))) {
      cout << (*i)->_name << "\t" << (*i)->_help << endl;
      printed++;
    }
  }
  if (printed) 
    return true;
  cout << "No commands match `" << arg << "'. Possiblities are:" << endl;
  for (list<fileman::command*>::iterator i = _commands.begin(); 
       i != _commands.end(); i++) {
    if (printed == 6) {
      cout << endl;
      printed = 0;
    }
    cout << (*i)->_name << "\t";
    printed++;
  }
  if (printed)
    cout << endl;
  return true;
}

//____________________________________________________________________
string examples::fileman::manager::make_prompt() 
{
  static char str[32];
  sprintf(str, "fileman [%d] ", _history->where());
  return string(str);
}

//____________________________________________________________________
int examples::fileman::manager::run() 
{
  string line;
  while (!_done) {
    if (!read(make_prompt(), line)) 
      break;
    
    if (line.empty()) 
      continue;

    _history->add(line);
    string word = _history->arguments(0, 0, line);
    fileman::command* c = 0;
    list<string> words = _completer->matches(word);
    if (words.size() > 1) {
      cerr << "Ambiguous command " << word << std::endl;
      continue;
    }
    if (words.size() == 1) {
      word = *(words.begin());
      for (list<fileman::command*>::iterator i = _commands.begin(); 
	   i != _commands.end(); i++) {
	if (word == (*i)->_name) {
	  c = (*i);
	  break;
	}
      }
    }
    if (!c) {
      cerr << "No such command: `" << word << "'" << endl;
      continue;
    }
    
    word = "";
    string arg;
    int n = 1;
    do {
      arg = _history->arguments(n, n, line);
      if (arg.empty()) break;
      if (n != 1) word.append(" ");
      word.append(arg);
      n++;
    } while(true);
    c->operator()(word);
  }
  return 0;
}


//____________________________________________________________________
std::list<std::string> 
examples::fileman::completer::alternate_completer(const string& what, 
					int start, int stop)
{
  _start = true;
  std::list<std::string> l;
  if (start == 0) 
    return l = matches(what);
  _start = false;
  return l;
}

//____________________________________________________________________
void examples::fileman::completer::possibilities(const std::string& what) 
{
  _current = _manager->commands().begin();  
}

//____________________________________________________________________
string examples::fileman::completer::complete(const std::string& what) 
{
  if (!_start) 
    return filename_complete(what);

  string ret;
  while (_current != _manager->commands().end()) {
    if (!(*_current)->_name.compare(CMP_ARG(what))) {
      ret = (*_current)->_name;
      break;
    }
    _current++;
  }
  if (_current != _manager->commands().end())
    _current++;
  return ret;
}

//____________________________________________________________________
int main(int argc, char** argv) 
{
  examples::fileman::manager fm;
  return fm.run();
}

//____________________________________________________________________
//
// EOF
//

    
