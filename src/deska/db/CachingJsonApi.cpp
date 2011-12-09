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
#include "CachingJsonApi.h"
#include "FakeApi.h"

namespace Deska {
namespace Db {

CachingJsonApi::CachingJsonApi(): m_askedForMetadata(false), m_commandBatching(SEND_IMMEDIATELY)
{
    m_cache = new FakeApi();
}

CachingJsonApi::~CachingJsonApi()
{
    delete m_cache;
}

void CachingJsonApi::askForMetadata() const
{
    if (m_askedForMetadata)
        return;

    std::vector<Identifier> topLevelObjects = JsonApiParser::kindNames();
    BOOST_FOREACH(const Identifier &name, topLevelObjects) {
        std::vector<KindAttributeDataType> attrs = JsonApiParser::kindAttributes(name);
        m_cache->attrs[name] = attrs;
        std::vector<ObjectRelation> relations = JsonApiParser::kindRelations(name);
        m_cache->relations[name] = relations;
    }

    m_askedForMetadata = true;
}

std::vector<Identifier> CachingJsonApi::kindNames() const
{
    askForMetadata();
    return m_cache->kindNames();
}

std::vector<KindAttributeDataType> CachingJsonApi::kindAttributes(const Identifier &kindName) const
{
    askForMetadata();
    return m_cache->kindAttributes(kindName);
}

std::vector<ObjectRelation> CachingJsonApi::kindRelations(const Identifier &kindName) const
{
    askForMetadata();
    return m_cache->kindRelations(kindName);
}

void CachingJsonApi::deleteObject(const Identifier &kindName, const Identifier &objectName)
{
    switch (m_commandBatching) {
    case SEND_IMMEDIATELY:
        JsonApiParser::deleteObject(kindName, objectName);
        break;
    case CACHE_SAME_OBJECT:
        if (!isPreviousSameObjectAs(kindName, objectName))
            flushCommandsNow();
        m_pendingModifications.push_back(DeleteObjectModification(kindName, objectName));
        break;
    }
}

void CachingJsonApi::restoreDeletedObject(const Identifier &kindName, const Identifier &objectName)
{
    // There's no batched equivalent for this command (which is by design, it is expected to be used as a direct result of
    // interactive actions where extra bit of latency does not hurt so much.
    // Hence we just have to flush stuff and carry on.
    flushCommandsNow();
    JsonApiParser::restoreDeletedObject(kindName, objectName);
}

Identifier CachingJsonApi::createObject(const Identifier &kindName, const Identifier &objectName)
{
    switch (m_commandBatching) {
    case SEND_IMMEDIATELY:
        return JsonApiParser::createObject(kindName, objectName);
    case CACHE_SAME_OBJECT:
        // This command *always* forces a queue flush in this mode
        flushCommandsNow();
        if (!objectName.empty()) {
            // If the command is for creating an object with a known name, we can safely cache the command for later, as the
            // Deska server is forbidden to change the name by the DBAPI spec.
            m_pendingModifications.push_back(CreateObjectModification(kindName, objectName));
            // Yes, this is a small cheat. It is guaranteed to work, though.
            return objectName;
        } else {
            // Okay, we're asked to create an object and return the name which the server decided to assign. Clearly we cannot
            // cache the command at this point
            return JsonApiParser::createObject(kindName, objectName);
        }
        break;
    }
    BOOST_ASSERT(false);
}

void CachingJsonApi::renameObject(const Identifier &kindName, const Identifier &oldObjectName, const Identifier &newObjectName)
{
    switch (m_commandBatching) {
    case SEND_IMMEDIATELY:
        JsonApiParser::renameObject(kindName, oldObjectName, newObjectName);
        break;
    case CACHE_SAME_OBJECT:
        // The renameObject() is indeed special, but this special case is already taken care of by the ExtractObjectId visitor
        // which is used by the isPreviousSameObjectAs(), so we are safe here.
        if (!isPreviousSameObjectAs(kindName, oldObjectName))
            flushCommandsNow();
        m_pendingModifications.push_back(RenameObjectModification(kindName, oldObjectName, newObjectName));
        break;
    }
}

void CachingJsonApi::setAttribute(const Identifier &kindName, const Identifier &objectName, const Identifier &attributeName, const Value &attributeData)
{
    switch (m_commandBatching) {
    case SEND_IMMEDIATELY:
        JsonApiParser::setAttribute(kindName, objectName, attributeName, attributeData);
        break;
    case CACHE_SAME_OBJECT:
        if (!isPreviousSameObjectAs(kindName, objectName))
            flushCommandsNow();
        m_pendingModifications.push_back(SetAttributeModification(kindName, objectName, attributeName, attributeData));
        break;
    }
}

void CachingJsonApi::setAttributeInsert(const Identifier &kindName, const Identifier &objectName, const Identifier &attributeName, const Identifier &attributeData)
{
    switch (m_commandBatching) {
    case SEND_IMMEDIATELY:
        JsonApiParser::setAttributeInsert(kindName, objectName, attributeName, attributeData);
        break;
    case CACHE_SAME_OBJECT:
        if (!isPreviousSameObjectAs(kindName, objectName))
            flushCommandsNow();
        m_pendingModifications.push_back(SetAttributeInsertModification(kindName, objectName, attributeName, attributeData));
        break;
    }
}

void CachingJsonApi::setAttributeRemove(const Identifier &kindName, const Identifier &objectName, const Identifier &attributeName, const Identifier &attributeData)
{
    switch (m_commandBatching) {
    case SEND_IMMEDIATELY:
        JsonApiParser::setAttributeRemove(kindName, objectName, attributeName, attributeData);
        break;
    case CACHE_SAME_OBJECT:
        if (!isPreviousSameObjectAs(kindName, objectName))
            flushCommandsNow();
        m_pendingModifications.push_back(SetAttributeRemoveModification(kindName, objectName, attributeName, attributeData));
        break;
    }
}

void CachingJsonApi::applyBatchedChanges(const std::vector<ObjectModificationCommand> &modifications)
{
    switch (m_commandBatching) {
    case SEND_IMMEDIATELY:
        JsonApiParser::applyBatchedChanges(modifications);
        break;
    case CACHE_SAME_OBJECT:
        // Theoretically, we are supposed to check the whole new list to see whether we can safely merge the comands together.
        // On the other hand, we already know that the list of modifications which we are given at this point is explicitly merged
        // together. We're going to try the gamble and merge them together, no matter what. Let's hope that this is safe.
        std::copy(modifications.begin(), modifications.end(), std::back_inserter(m_pendingModifications));
        break;
    }
}

/** @short Extract a pair of (kindName, objectName) from any object modification command */
struct ExtractObjectId: public boost::static_visitor<std::pair<Identifier, Identifier> >
{
    template<typename T>
    result_type operator()(const T &a) const
    {
        return std::make_pair(a.kindName, a.objectName);
    }

    result_type operator()(const RenameObjectModification &a) const
    {
        // This is a special case -- if we rename an object, the further commands shall reference its new name.
        return std::make_pair(a.kindName, a.newObjectName);
    }
};

bool CachingJsonApi::isPreviousSameObjectAs(const Identifier &kindName, const Identifier &objectName)
{
    if (m_pendingModifications.empty())
        return false;

    return boost::apply_visitor(ExtractObjectId(), m_pendingModifications.back()) == std::make_pair(kindName, objectName);
}

void CachingJsonApi::flushCommandsNow()
{
    if (m_pendingModifications.empty()) {
        // We don't really want to execute an extra DBAPI call just for fun here
        return;
    }

    JsonApiParser::applyBatchedChanges(m_pendingModifications);
    m_pendingModifications.clear();
}

void CachingJsonApi::setCommandBatching(const CommandBatchingMode mode)
{
    if (mode < m_commandBatching)
        flushCommandsNow();
    m_commandBatching = mode;
}


}
}
