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

#include <boost/foreach.hpp>
#include "DbInteraction.h"

namespace Deska
{

namespace Cli
{


DbInteraction::DbInteraction(Db::Api *api):
    m_api(api)
{
}



void DbInteraction::createObject(const Db::ContextStack &context)
{
    BOOST_ASSERT(!context.empty());
    m_api->createObject(context.back().kind, Db::toPath(context));
}



void DbInteraction::deleteObject(const Db::ContextStack &context)
{
    BOOST_ASSERT(!context.empty());
    m_api->deleteObject(context.back().kind, Db::toPath(context));
}



void DbInteraction::setAttribute(const Db::ContextStack &context,
                                 const Db::AttributeDefinition &attribute)
{
    BOOST_ASSERT(!context.empty());
    m_api->setAttribute(context.back().kind, Db::toPath(context), attribute.attribute, attribute.value);
}



std::vector<Db::Identifier> DbInteraction::kindInstances(const Db::Identifier &kindName)
{
    return m_api->kindInstances(kindName);
}



std::vector<Db::ObjectDefinition> DbInteraction::allObjects()
{
    std::vector<Db::ObjectDefinition> objects;
    BOOST_FOREACH(const Deska::Db::Identifier &kindName, m_api->kindNames()) {
        BOOST_FOREACH(const Deska::Db::Identifier &objectName, m_api->kindInstances(kindName)) {
            objects.push_back(Db::ObjectDefinition(kindName, objectName));
        }
    }
    return objects;
}



std::vector<Db::AttributeDefinition> DbInteraction::allAttributes(const Db::ObjectDefinition &object)
{
    std::vector<Db::AttributeDefinition> attributes;
    typedef std::map<Deska::Db::Identifier, Deska::Db::Value> ObjectDataMap;
    BOOST_FOREACH(const ObjectDataMap::value_type &x, m_api->objectData(object.kind, object.name)) {
        attributes.push_back(Db::AttributeDefinition(x.first, x.second));
    }
    return attributes;
}



std::vector<Db::AttributeDefinition> DbInteraction::allAttributes(const Db::ContextStack &context)
{
    std::vector<Db::AttributeDefinition> attributes;
    if (!context.empty()) {
        typedef std::map<Deska::Db::Identifier, Deska::Db::Value> ObjectDataMap;
        BOOST_FOREACH(const ObjectDataMap::value_type &x, m_api->objectData(context.back().kind, Db::toPath(context))) {
            attributes.push_back(Db::AttributeDefinition(x.first, x.second));
        }
    }
    return attributes;
}



std::vector<Db::ObjectDefinition> DbInteraction::allNestedKinds(const Db::ContextStack &context)
{
    std::vector<Db::ObjectDefinition> kinds;
    if (!context.empty()) {
        // TODO: Obtain list of nested kinds.
    } else {
        kinds = allObjects();
    }
    return kinds;
}



bool DbInteraction::objectExists(const Db::ContextStack &context)
{
    std::vector<Db::Identifier> instances = kindInstances(context.back().kind);
    if (std::find(instances.begin(), instances.end(), Db::toPath(context)) == instances.end()) {
        return false;
    } else {
        return true;
    }
}



std::vector<Db::PendingChangeset> DbInteraction::allPendingChangesets()
{
    return m_api->pendingChangesets();
}



Db::TemporaryChangesetId DbInteraction::createNewChangeset()
{
    return m_api->startChangeset();
}



void DbInteraction::resumeChangeset(const Db::TemporaryChangesetId &changesetId)
{
    m_api->resumeChangeset(changesetId);
}



void DbInteraction::commitChangeset(const std::string &message)
{
    m_api->commitChangeset(message);
}



void DbInteraction::detachFromChangeset(const std::string &message)
{
    m_api->detachFromCurrentChangeset(message);
}



void DbInteraction::abortChangeset()
{
    m_api->abortCurrentChangeset();
}


}
}
