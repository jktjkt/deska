/* Copyright (C) 2011 Jan Kundrát <kundratj@fzu.cz>
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

#ifndef DESKA_CLI_INTERACTION_H
#define DESKA_CLI_INTERACTION_H

#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/signals2/trackable.hpp>
#include "deska/db/Api.h"
#include "deska/db/Objects.h"
#include "deska/db/Revisions.h"

namespace Deska {
namespace Db {

class Api;

}

namespace Cli {

class Parser;
class ParserException;



/** @short Tie up the real command line and the Parser together */
class CliInteraction: public boost::noncopyable, public boost::signals2::trackable
{
public:
    CliInteraction(Db::Api *api, Parser *parser);

    /** @short Enter the loop and interact with the user */
    void run();

    void createObject(const Db::ObjectDefinition &object);
    void deleteObject(const Db::ObjectDefinition &object);
    void setAttribute(const Db::ObjectDefinition &object, const Db::AttributeDefinition &attribute);

    /** @short Dump everything in the DB */
    void dumpDbContents();

    std::vector<Db::ObjectDefinition> getAllObjects();

    std::vector<Db::AttributeDefinition> getAllAttributes(const Db::ObjectDefinition &object);

    

    std::vector<Db::PendingChangeset> getAllPendingChangesets();
    Db::TemporaryChangesetId createNewChangeset();



private:
    void slotCategoryEntered(const Db::Identifier &kind, const Db::Identifier &name);
    void slotParserError(const ParserException &e);
    void slotSetAttribute(const Db::Identifier &name, const Db::Value &attributeData);

    

    /** @short Ask the user whether she wants to proceed with something */
    bool askForConfirmation(const std::string &prompt);

    /** @short Implementation of the "event loop" which interacts with the user */
    void eventLoop();

private:
    Db::Api *m_api;
    Parser *m_parser;
    bool m_ignoreParserActions;
};



}

}



#endif  // DESKA_CLI_INTERACTION_H
