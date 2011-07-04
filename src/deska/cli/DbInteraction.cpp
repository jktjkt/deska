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
#include "deska/db/Api.h"

namespace Deska
{

namespace Cli
{


DbInteraction::DbInteraction(Db::Api *api):
    m_api(api)
{
}



void DbInteraction::createObject(const ContextStack &context)
{
    BOOST_ASSERT(!context.empty());
    m_api->createObject(context.back().kind, contextStackToPath(context));
}



void DbInteraction::restoreDeletedObject(const ContextStack &context)
{
    BOOST_ASSERT(!context.empty());
    m_api->restoreDeletedObject(context.back().kind, contextStackToPath(context));
}



void DbInteraction::deleteObject(const ContextStack &context)
{
    BOOST_ASSERT(!context.empty());
    m_api->deleteObject(context.back().kind, contextStackToPath(context));
}



void DbInteraction::renameObject(const ContextStack &context, const Db::Identifier &newName)
{
    BOOST_ASSERT(!context.empty());
    ContextStack newContext = context;
    newContext.back().name = newName;
    m_api->renameObject(context.back().kind, contextStackToPath(context), contextStackToPath(newContext));
}



void DbInteraction::setAttribute(const ContextStack &context,
                                 const Db::AttributeDefinition &attribute)
{
    BOOST_ASSERT(!context.empty());
    m_api->setAttribute(context.back().kind, contextStackToPath(context), attribute.attribute, attribute.value);
}



void DbInteraction::removeAttribute(const ContextStack &context,
                                    const Db::Identifier &attribute)
{
    BOOST_ASSERT(!context.empty());
    m_api->setAttribute(context.back().kind, contextStackToPath(context), attribute, Deska::Db::Value());
}



std::vector<Db::Identifier> DbInteraction::kindNames()
{
    return m_api->kindNames();
}



std::vector<Db::ObjectDefinition> DbInteraction::kindInstances(const Db::Identifier &kindName)
{
    std::vector<Db::ObjectDefinition> objects;
    BOOST_FOREACH(const Deska::Db::Identifier &objectName, m_api->kindInstances(kindName)) {
        objects.push_back(Db::ObjectDefinition(kindName, objectName));
    }
    return objects;
}



std::vector<Db::AttributeDefinition> DbInteraction::allAttributes(const Db::ObjectDefinition &object)
{
    std::vector<Db::AttributeDefinition> attributes;

    // Check whether this kind contains any attributes. If not, return empty list.
    if (m_api->kindAttributes(object.kind).empty())
        return attributes;

    typedef std::map<Deska::Db::Identifier, Deska::Db::Value> ObjectDataMap;
    BOOST_FOREACH(const ObjectDataMap::value_type &x, m_api->objectData(object.kind, object.name)) {
        attributes.push_back(Db::AttributeDefinition(x.first, x.second));
    }
    return attributes;
}



std::vector<Db::AttributeDefinition> DbInteraction::allAttributes(const ContextStack &context)
{
    std::vector<Db::AttributeDefinition> attributes;

    if (!context.empty()) {

        // Check whether this kind contains any attributes. If not, return empty list.
        if (m_api->kindAttributes(context.back().kind).empty())
            return attributes;

        typedef std::map<Deska::Db::Identifier, Deska::Db::Value> ObjectDataMap;
        BOOST_FOREACH(const ObjectDataMap::value_type &x, m_api->objectData(context.back().kind, contextStackToPath(context))) {
            attributes.push_back(Db::AttributeDefinition(x.first, x.second));
        }
    }
    return attributes;
}



std::vector<Db::ObjectDefinition> DbInteraction::allNestedKinds(const ContextStack &context)
{
    BOOST_ASSERT(!context.empty());
    std::vector<Db::ObjectDefinition> kinds;
    // TODO: Obtain list of nested kinds.
    return kinds;
}



bool DbInteraction::objectExists(const ContextStack &context)
{
    BOOST_ASSERT(!context.empty());
    std::vector<Db::Identifier> instances = m_api->kindInstances(context.back().kind);
    if (std::find(instances.begin(), instances.end(), contextStackToPath(context)) == instances.end()) {
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
