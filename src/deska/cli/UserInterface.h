/*
* Copyright (C) 2011 Jan Kundrát <kundratj@fzu.cz>
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

#ifndef DESKA_USER_INTERFACE_H
#define DESKA_USER_INTERFACE_H

#include <string>
#include <iostream>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/noncopyable.hpp>

//#include "rlmm/readline.hh"

#include "CliInteraction.h"
#include "Parser.h"
#include "Exceptions.h"


namespace Deska
{
namespace Cli
{


class UserInterface: public boost::noncopyable//: public rlmm::readline
{
public:

    UserInterface(std::ostream &outStream, std::ostream &errStream, std::istream &inStream,
                  CliInteraction *dbInteraction, Parser* parser);

    void applyCategoryEntered(const std::vector<Db::ObjectDefinition> &context,
                         const Db::Identifier &kind, const Db::Identifier &object);
    void applySetAttribute(const std::vector<Db::ObjectDefinition> &context,
                      const Db::Identifier &attribute, const Db::Value &value);
    void applyFunctionShow(const std::vector<Db::ObjectDefinition> &context);
    void applyFunctionDelete(const std::vector<Db::ObjectDefinition> &context);

    bool confirmCategoryEntered(const std::vector<Db::ObjectDefinition> &context,
                         const Db::Identifier &kind, const Db::Identifier &object);
    bool confirmSetAttribute(const std::vector<Db::ObjectDefinition> &context,
                      const Db::Identifier &attribute, const Db::Value &value);
    bool confirmFunctionShow(const std::vector<Db::ObjectDefinition> &context);
    bool confirmFunctionDelete(const std::vector<Db::ObjectDefinition> &context);

    void reportError(const std::string &errorMessage);

    bool askForConfirmation(const std::string &prompt);

    /** @short Dump everything in the DB */
    void dumpDbContents();

    void printAttributes(const Db::ObjectDefinition &object);

    void commitChangeset();
    void detachFromChangeset();
    void abortChangeset();

    void run();
    void eventLoop();

private:

    std::ostream out;
    std::ostream err;
    std::istream in;
    
    CliInteraction *m_dbInteraction;
    Parser* m_parser;
    
};


}
}


#endif // DESKA_USER_INTERFACE_H
