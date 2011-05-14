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
#include "deska/db/Api.h"
#include "deska/db/Objects.h"
#include "deska/db/Revisions.h"

namespace Deska {

namespace Cli {


/** @short Tie up the real command line and the Parser together */
class CliInteraction: public boost::noncopyable
{
public:
    CliInteraction(Db::Api *api);

    void createObject(const Db::ObjectDefinition &object);
    void deleteObject(const Db::ObjectDefinition &object);
    void setAttribute(const Db::ObjectDefinition &object, const Db::AttributeDefinition &attribute);

    std::vector<Db::ObjectDefinition> allObjects();

    std::vector<Db::AttributeDefinition> allAttributes(const Db::ObjectDefinition &object);

    

    std::vector<Db::PendingChangeset> allPendingChangesets();
    Db::TemporaryChangesetId createNewChangeset();
    void resumeChangeset(const Db::TemporaryChangesetId &changesetId);
    void commitChangeset(const std::string &message);
    void detachFromChangeset(const std::string &message);
    void abortChangeset();



private:
    Db::Api *m_api;

};



}

}



#endif  // DESKA_CLI_INTERACTION_H
