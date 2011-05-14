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

#include <boost/foreach.hpp>
#include "CliInteraction.h"

namespace Deska
{

namespace Cli
{


CliInteraction::CliInteraction(Db::Api *api):
    m_api(api)
{
}



void CliInteraction::createObject(const Db::ObjectDefinition &object)
{
    m_api->createObject(object.kind, object.name);
}



void CliInteraction::deleteObject(const Db::ObjectDefinition &object)
{
    m_api->deleteObject(object.kind, object.name);
}



void CliInteraction::setAttribute(const Db::ObjectDefinition &object, const Db::AttributeDefinition &attribute)
{
    m_api->setAttribute(object.kind, object.name, attribute.attribute, attribute.value);
}



std::vector<Db::ObjectDefinition> CliInteraction::allObjects()
{
    std::vector<Db::ObjectDefinition> objects;
    BOOST_FOREACH(const Deska::Db::Identifier &kindName, m_api->kindNames()) {
        BOOST_FOREACH(const Deska::Db::Identifier &objectName, m_api->kindInstances(kindName)) {
            objects.push_back(Db::ObjectDefinition(kindName, objectName));
        }
    }
    return objects;
}



std::vector<Db::AttributeDefinition> CliInteraction::allAttributes(const Db::ObjectDefinition &object)
{
    std::vector<Db::AttributeDefinition> attributes;
    typedef std::map<Deska::Db::Identifier, Deska::Db::Value> ObjectDataMap;
    BOOST_FOREACH(const ObjectDataMap::value_type &x, m_api->objectData(object.kind, object.name)) {
        attributes.push_back(Db::AttributeDefinition(x.first, x.second));
    }
    return attributes;
}



std::vector<Db::PendingChangeset> CliInteraction::allPendingChangesets()
{
    return m_api->pendingChangesets();
}



Db::TemporaryChangesetId CliInteraction::createNewChangeset()
{
    return m_api->startChangeset();
}



void CliInteraction::resumeChangeset(const Db::TemporaryChangesetId &changesetId)
{
    m_api->resumeChangeset(changesetId);
}



void CliInteraction::commitChangeset(const std::string &message)
{
    m_api->commitChangeset(message);
}



void CliInteraction::detachFromChangeset(const std::string &message)
{
    m_api->detachFromCurrentChangeset(message);
}



void CliInteraction::abortChangeset()
{
    m_api->abortCurrentChangeset();
}


}
}
