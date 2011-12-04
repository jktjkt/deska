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
#include <sstream>
#include "DbInteraction.h"

namespace Deska
{

namespace Cli
{


DbInteraction::DbInteraction(Db::Api *api):
    m_api(api)
{
    allKinds = m_api->kindNames();
    bool isEmbedded = false;
    for (std::vector<Db::Identifier>::iterator itk = allKinds.begin(); itk != allKinds.end(); ++itk) {
        std::vector<Db::ObjectRelation> relations = m_api->kindRelations(*itk);
        isEmbedded = false;
        for (std::vector<Db::ObjectRelation>::iterator itr = relations.begin(); itr != relations.end(); ++itr) {
            if (itr->kind == Db::RELATION_EMBED_INTO) {
                isEmbedded = true;
                embeds[itr->target].push_back(*itk);
                embeddedInto[*itk] = std::make_pair<Db::Identifier, Db::Identifier>(itr->column, itr->target);
            }

            if (itr->kind == Db::RELATION_CONTAINABLE) {
                containable[*itk].push_back(itr->target);
                referringAttrs[*itk][itr->column] = itr->target;
                readonlyAttributes[*itk].push_back(itr->column);
            }

            if (itr->kind == Db::RELATION_CONTAINS) {
                contains[*itk].push_back(itr->target);
                referringAttrs[*itk][itr->column] = itr->target;
                readonlyAttributes[*itk].push_back(itr->column);
            }

        }
        if (!isEmbedded)
            pureTopLevelKinds.push_back(*itk);
    }
}



ContextStackItem DbInteraction::createObject(const ContextStack &context)
{
    BOOST_ASSERT(!context.empty());
    std::vector<ObjectDefinition> objects = expandContextStack(context);

    BOOST_ASSERT(!objects.empty());

    if (objects.size() == 1) {
        BOOST_ASSERT(!objectExists(objects.front()));
        Db::Identifier newObjectName = m_api->createObject(objects.front().kind, objects.front().name);
        if (stableView)
            objectExistsCache[objects.front()] = true;
        // If the name of new object is specified, returned name should be the same
        if (!(pathToVector(objects.front().name).back().empty()))
            BOOST_ASSERT(newObjectName == objects.front().name);
        return ContextStackItem(objects.front().kind, pathToVector(newObjectName).back());
    }

    std::vector<Db::ObjectModificationCommand> modifications;
    for (std::vector<ObjectDefinition>::iterator it = objects.begin(); it != objects.end(); ++it) {
        if (!objectExists(*it)) {
            modifications.push_back(Db::CreateObjectModification(it->kind, it->name));
            if (stableView)
                objectExistsCache[*it] = true;
        }
    }
    m_api->applyBatchedChanges(modifications);

    if (context.back().name.empty())
        return ContextStackItem(context.back().kind, Db::Filter(
        Db::SpecialExpression(Db::FILTER_SPECIAL_EMBEDDED_LAST_ONE, context.back().kind)));
    else
        return ContextStackItem(context.back().kind, context.back().name);
}



void DbInteraction::restoreDeletedObject(const ContextStack &context)
{
    BOOST_ASSERT(!context.empty());
    std::vector<ObjectDefinition> objects = expandContextStack(context);
    for (std::vector<ObjectDefinition>::iterator it = objects.begin(); it != objects.end(); ++it) {
        if (!objectExists(*it)) {
            m_api->restoreDeletedObject(it->kind, it->name);
            if (stableView)
                objectExistsCache[*it] = true;
        }
    }
}



void DbInteraction::deleteObject(const ContextStack &context)
{
    BOOST_ASSERT(!context.empty());
    std::vector<ObjectDefinition> objects = expandContextStack(context);
    deleteObjects(objects);
}



void DbInteraction::deleteObjects(const std::vector<ObjectDefinition> &objects)
{
    std::vector<Db::ObjectModificationCommand> modifications;
    for (std::vector<ObjectDefinition>::const_iterator it = objects.begin(); it != objects.end(); ++it) {
        if (objectExists(*it)) {
            modifications.push_back(Db::DeleteObjectModification(it->kind, it->name));
            if (stableView)
                objectExistsCache[*it] = false;
        }
    }
    m_api->applyBatchedChanges(modifications);
}



void DbInteraction::renameObject(const ContextStack &context, const Db::Identifier &newName)
{
    BOOST_ASSERT(!context.empty());
    std::vector<ObjectDefinition> objects = expandContextStack(context);
    renameObjects(objects, newName);
}



void DbInteraction::renameObjects(const std::vector<ObjectDefinition> &objects, const Db::Identifier &newName)
{
    std::vector<Db::ObjectModificationCommand> modifications;
    for (std::vector<ObjectDefinition>::const_iterator it = objects.begin(); it != objects.end(); ++it) {
        std::vector<Db::Identifier> newObjName = pathToVector(it->name);
        newObjName.back() = newName;
        modifications.push_back(Db::RenameObjectModification(it->kind, it->name, vectorToPath(newObjName)));
        if (stableView) {
            objectExistsCache[*it] = false;
            objectExistsCache[ObjectDefinition(it->kind, vectorToPath(newObjName))] = true;
        }
    }
    m_api->applyBatchedChanges(modifications);
}



void DbInteraction::setAttribute(const ContextStack &context,
                                 const AttributeDefinition &attribute)
{
    BOOST_ASSERT(!context.empty());
    std::vector<ObjectDefinition> objects = expandContextStack(context);
    std::vector<Db::ObjectModificationCommand> modifications;
    for (std::vector<ObjectDefinition>::iterator it = objects.begin(); it != objects.end(); ++it) {
        modifications.push_back(Db::SetAttributeModification(it->kind, it->name, attribute.attribute,
                                                             attribute.value));
    }
    m_api->applyBatchedChanges(modifications);
}



void DbInteraction::setAttributeInsert(const ContextStack &context, const Db::Identifier &set,
                                       const Db::Identifier &identifier)
{
    BOOST_ASSERT(!context.empty());
    std::vector<ObjectDefinition> objects = expandContextStack(context);
    std::vector<Db::ObjectModificationCommand> modifications;
    for (std::vector<ObjectDefinition>::iterator it = objects.begin(); it != objects.end(); ++it) {
        modifications.push_back(Db::SetAttributeInsertModification(it->kind, it->name, set,
                                                                   identifier));
    }
    m_api->applyBatchedChanges(modifications);
}



void DbInteraction::setAttributeRemove(const ContextStack &context, const Db::Identifier &set,
                                       const Db::Identifier &identifier)
{
    BOOST_ASSERT(!context.empty());
    std::vector<ObjectDefinition> objects = expandContextStack(context);
    std::vector<Db::ObjectModificationCommand> modifications;
    for (std::vector<ObjectDefinition>::iterator it = objects.begin(); it != objects.end(); ++it) {
        modifications.push_back(Db::SetAttributeRemoveModification(it->kind, it->name, set,
                                                                   identifier));
    }
    m_api->applyBatchedChanges(modifications);
}



void DbInteraction::removeAttribute(const ContextStack &context,
                                    const Db::Identifier &attribute)
{
    BOOST_ASSERT(!context.empty());
    std::vector<ObjectDefinition> objects = expandContextStack(context);
    std::vector<Db::ObjectModificationCommand> modifications;
    for (std::vector<ObjectDefinition>::iterator it = objects.begin(); it != objects.end(); ++it) {
        modifications.push_back(Db::SetAttributeModification(it->kind, it->name, attribute, Deska::Db::Value()));
    }
    m_api->applyBatchedChanges(modifications);
}



std::vector<Db::Identifier> DbInteraction::kindNames()
{
    return allKinds;
}



std::vector<Db::Identifier> DbInteraction::topLevelKinds()
{
    return pureTopLevelKinds;
}



std::vector<ObjectDefinition> DbInteraction::kindInstances(const Db::Identifier &kindName)
{
    std::vector<ObjectDefinition> objects;
    BOOST_FOREACH(const Deska::Db::Identifier &objectName, m_api->kindInstances(kindName)) {
        objects.push_back(ObjectDefinition(kindName, objectName));
        if (stableView)
            objectExistsCache[ObjectDefinition(kindName, objectName)] = true;
    }
    return objects;
}



std::vector<AttributeDefinition> DbInteraction::allAttributes(const ObjectDefinition &object)
{
    std::vector<AttributeDefinition> attributes;

    // Check whether this kind contains any attributes. If not, return empty list.
    if (m_api->kindAttributes(object.kind).empty())
        return attributes;

    typedef std::map<Deska::Db::Identifier, Deska::Db::Value> ObjectDataMap;
    BOOST_FOREACH(const ObjectDataMap::value_type &x, m_api->objectData(object.kind, object.name)) {
        attributes.push_back(AttributeDefinition(x.first, x.second));
    }
    return attributes;
}



std::vector<AttributeDefinition> DbInteraction::allAttributes(const ContextStack &context)
{
    if (!context.empty())
        return allAttributes(ObjectDefinition(context.back().kind, contextStackToPath(context)));
    else
        return std::vector<AttributeDefinition>();
}



std::vector<std::pair<AttributeDefinition, Db::Identifier> > DbInteraction::allAttributesResolvedWithOrigin(
    const ObjectDefinition &object)
{
    std::vector<std::pair<AttributeDefinition, Db::Identifier> > attributes;

    // Check whether this kind contains any attributes. If not, return empty list.
    if (m_api->kindAttributes(object.kind).empty())
        return attributes;

    typedef std::map<Db::Identifier, std::pair<boost::optional<Db::Identifier>, Db::Value> > ObjectDataMap;
    BOOST_FOREACH(const ObjectDataMap::value_type &x, m_api->resolvedObjectDataWithOrigin(object.kind, object.name)) {
        attributes.push_back(std::make_pair<AttributeDefinition, Db::Identifier>(
            AttributeDefinition(x.first, x.second.second),
                                 (!x.second.first || *(x.second.first) == object.name ? "" : *(x.second.first))));
    }
    return attributes;
}



std::vector<std::pair<AttributeDefinition, Db::Identifier> > DbInteraction::allAttributesResolvedWithOrigin(
    const ContextStack &context)
{
    if (!context.empty())
        return allAttributesResolvedWithOrigin(ObjectDefinition(context.back().kind, contextStackToPath(context)));
    else
        return std::vector<std::pair<AttributeDefinition, Db::Identifier> >();
}



std::vector<ObjectDefinition> DbInteraction::allNestedObjects(const ObjectDefinition &object)
{
    if (object.name.empty() || pathToVector(object.name).back().empty())
        throw std::logic_error("Deska::Cli::DbInteraction::allNestedObjects: Can not find nested objects for context stack without names.");

    std::vector<ObjectDefinition> kinds;
    for (std::vector<Db::Identifier>::iterator it = embeds[object.kind].begin(); it != embeds[object.kind].end(); ++it) {
        std::vector<Db::Identifier> emb = m_api->kindInstances(*it, Db::Filter(
            Db::AttributeExpression(Db::FILTER_COLUMN_EQ, object.kind, "name", Db::Value(object.name))));
        for (std::vector<Db::Identifier>::iterator ite = emb.begin(); ite != emb.end(); ++ite) {
            kinds.push_back(ObjectDefinition(*it, *ite));
            if (stableView)
                objectExistsCache[ObjectDefinition(*it, *ite)] = true;
        }
    }
    return kinds;
}



std::vector<ObjectDefinition> DbInteraction::allNestedObjects(const ContextStack &context)
{
    if (context.empty())
        return std::vector<ObjectDefinition>();
    std::vector<ObjectDefinition> objects = expandContextStack(context);
    std::vector<ObjectDefinition> nestObjects;
    for (std::vector<ObjectDefinition>::iterator it = objects.begin(); it != objects.end(); ++it) {
        std::vector<ObjectDefinition> tmpObj = allNestedObjects(*it);
        nestObjects.insert(nestObjects.begin(), tmpObj.begin(), tmpObj.end());
    }

    return nestObjects;
}



std::vector<ObjectDefinition> DbInteraction::allNestedObjectsTransitively(const ObjectDefinition &object)
{
    if (object.name.empty() || pathToVector(object.name).back().empty())
        throw std::logic_error("Deska::Cli::DbInteraction::allNestedObjectsTransitively: Can not find nested objects for context stack without names.");

    std::vector<ObjectDefinition> nestedObjects;
    allNestedObjectsTransitivelyRec(object, nestedObjects);

    return nestedObjects;
}



std::vector<ObjectDefinition> DbInteraction::allNestedObjectsTransitively(const ContextStack &context)
{
    if (context.empty())
        return std::vector<ObjectDefinition>();
    std::vector<ObjectDefinition> objects = expandContextStack(context);
    std::vector<ObjectDefinition> nestObjects;
    for (std::vector<ObjectDefinition>::iterator it = objects.begin(); it != objects.end(); ++it) {
        std::vector<ObjectDefinition> tmpObj = allNestedObjectsTransitively(*it);
        nestObjects.insert(nestObjects.begin(), tmpObj.begin(), tmpObj.end());
    }

    return nestObjects;
}



bool DbInteraction::objectExists(const ObjectDefinition &object)
{
    if (object.name.empty())
        return false;

    if (stableView) {
        std::map<ObjectDefinition, bool>::iterator it = objectExistsCache.find(object);
        if (it != objectExistsCache.end()) {
            return it->second;
        }
    }

    std::vector<Db::Identifier> instances = m_api->kindInstances(object.kind,
        Db::Filter(Db::AttributeExpression(Db::FILTER_COLUMN_EQ, object.kind, "name", Db::Value(object.name))));
    if (stableView) {
        objectExistsCache[object] = !instances.empty();
    }
    return (!instances.empty());
}



bool DbInteraction::objectExists(const ContextStack &context)
{
    BOOST_ASSERT(!context.empty());
    for (ContextStack::const_iterator it = context.begin(); it != context.end(); ++it) {
        if (!it->filter && it->name.empty())
            return false;
    }
    std::vector<ObjectDefinition> objects = expandContextStack(context);
    for (std::vector<ObjectDefinition>::iterator it = objects.begin(); it != objects.end(); ++it) {
        if (!objectExists(*it))
            return false;
    }
    return true;
}



std::vector<ObjectDefinition> DbInteraction::containedObjects(const ObjectDefinition &object)
{
    if (object.name.empty() || pathToVector(object.name).back().empty())
        throw std::logic_error("Deska::Cli::DbInteraction::containedObjects: Can not find contained objects for context stack without names.");

    // Kinds, that this kind contains
    std::vector<ObjectDefinition> containedObjects;
    for (std::vector<Db::Identifier>::iterator it =
        contains[object.kind].begin(); it != contains[object.kind].end(); ++it) {
        std::vector<Db::Identifier> instances = m_api->kindInstances(*it,
            Db::Filter(Db::AttributeExpression(Db::FILTER_COLUMN_EQ, *it, "name", Db::Value(object.name))));
        BOOST_ASSERT(instances.size() <= 1);
        if (!instances.empty()) {
            BOOST_ASSERT(instances.front() == object.name);
            ObjectDefinition mObj(*it, object.name);
            if (stableView)
                objectExistsCache[mObj] = true;
            if (std::find(containedObjects.begin(), containedObjects.end(), mObj) == containedObjects.end())
                containedObjects.push_back(mObj);
        }
    }

    return containedObjects;
}



std::vector<ObjectDefinition> DbInteraction::containedObjects(const ContextStack &context)
{
    if (context.empty())
        return std::vector<ObjectDefinition>();
    std::vector<ObjectDefinition> objects = expandContextStack(context);
    std::vector<ObjectDefinition> contObjects;
    for (std::vector<ObjectDefinition>::iterator it = objects.begin(); it != objects.end(); ++it) {
        std::vector<ObjectDefinition> tmpObj = containedObjects(*it);
        contObjects.insert(contObjects.begin(), tmpObj.begin(), tmpObj.end());
    }

    return contObjects;
}



std::vector<ObjectDefinition> DbInteraction::connectedObjectsTransitively(const ObjectDefinition &object)
{
    if (object.name.empty() || pathToVector(object.name).back().empty())
        throw std::logic_error("Deska::Cli::DbInteraction::connectedObjectsTransitively: Can not find connected objects for context stack without names.");

    std::vector<ObjectDefinition> containedObjects;
    connectedObjectsTransitivelyRec(object, containedObjects);

    return containedObjects;
}



std::vector<ObjectDefinition> DbInteraction::connectedObjectsTransitively(const ContextStack &context)
{
    if (context.empty())
        return std::vector<ObjectDefinition>();
    std::vector<ObjectDefinition> objects = expandContextStack(context);
    std::vector<ObjectDefinition> connObjects;
    for (std::vector<ObjectDefinition>::iterator it = objects.begin(); it != objects.end(); ++it) {
        std::vector<ObjectDefinition> tmpObj = connectedObjectsTransitively(*it);
        connObjects.insert(connObjects.begin(), tmpObj.begin(), tmpObj.end());
    }

    return connObjects;
}



Db::Identifier DbInteraction::referredKind(const Db::Identifier &kind, const Db::Identifier &attribute)
{
    std::map<Db::Identifier, std::map<Db::Identifier, Db::Identifier> >::const_iterator it = referringAttrs.find(kind);
    if (it == referringAttrs.end())
        return Db::Identifier();
    std::map<Db::Identifier, Db::Identifier>::const_iterator it2 = it->second.find(attribute);
    if (it2 == it->second.end())
        return Db::Identifier();
    else
        return it2->second;
}



std::vector<Db::PendingChangeset> DbInteraction::allPendingChangesets()
{
    return m_api->pendingChangesets();
}



Db::TemporaryChangesetId DbInteraction::createNewChangeset()
{
    clearCache();
    stableView = true;
    return m_api->startChangeset();   
}



void DbInteraction::resumeChangeset(const Db::TemporaryChangesetId &changesetId)
{
    clearCache();
    stableView = true;
    m_api->resumeChangeset(changesetId);
}



Db::RevisionId DbInteraction::commitChangeset(const std::string &message)
{
    clearCache();
    stableView = false;
    return m_api->commitChangeset(message);
}



void DbInteraction::detachFromChangeset(const std::string &message)
{
    clearCache();
    stableView = false;
    m_api->detachFromCurrentChangeset(message);
}



void DbInteraction::abortChangeset()
{
    clearCache();
    stableView = false;
    m_api->abortCurrentChangeset();
}



Db::RevisionId DbInteraction::restoringCommit(const std::string &message, const std::string &author, const boost::posix_time::ptime &timestamp)
{
    clearCache();
    stableView = false;
    return m_api->restoringCommit(message, author, timestamp);
}



std::vector<Db::RevisionMetadata> DbInteraction::allRevisions()
{
    return m_api->listRevisions();
}



std::vector<Db::RevisionMetadata> DbInteraction::filteredRevisions(const Db::Filter &filter)
{
    return m_api->listRevisions(filter);
}



std::vector<Db::ObjectModificationResult> DbInteraction::revisionsDifference(const Db::RevisionId &revisionA,
                                                                       const Db::RevisionId &revisionB)
{
    return m_api->dataDifference(revisionA, revisionB);
}



std::vector<Db::ObjectModificationResult> DbInteraction::revisionsDifferenceChangeset(const Db::TemporaryChangesetId &changeset)
{
    return m_api->dataDifferenceInTemporaryChangeset(changeset);
}



Db::RevisionId DbInteraction::changesetParent(const Db::TemporaryChangesetId &changeset)
{
    std::vector<Db::PendingChangeset> changesets = m_api->pendingChangesets(
        Db::Filter(Db::MetadataExpression(Db::FILTER_COLUMN_EQ, "changeset", changeset)));
    if (changesets.empty()) {
        std::ostringstream ostr;
        ostr << "Deska::Cli::DbInteraction::changesetParent: No parent revision found for " << changeset;
        throw std::out_of_range(ostr.str());
    }   
    BOOST_ASSERT(changesets.size() == 1);
    return changesets.back().parentRevision;
}



std::string DbInteraction::configDiff(const Db::Api::ConfigGeneratingMode forceRegenerate)
{
    return m_api->showConfigDiff(forceRegenerate);
}



std::vector<ObjectDefinition> DbInteraction::expandContextStack(const ContextStack &context)
{
    if (context.empty())
        return std::vector<ObjectDefinition>();

    std::vector<ObjectDefinition> objects;

    try {
        // If the context stack does not contain filters, we can extract the object directly
        objects.push_back(ObjectDefinition(context.back().kind, contextStackToPath(context)));
    } catch (ContextStackConversionError &e) {
        for (ContextStack::const_iterator it = context.begin(); it != context.end(); ++it) {
            if (objects.empty()) {
                if (it->itemType == ContextStackItem::CONTEXT_STACK_ITEM_TYPE_FILTER) {
                    // If we have not any objects extracted yet, get some using filter when in context stack is filter
                    std::vector<Db::Identifier> instances = m_api->kindInstances(it->kind, it->filter);
                    if (instances.empty()) {
                        // If the filter does not match any objects, return empty vector, because it does not make
                        // a sense to extract objects using the rest of the context now.
                        return std::vector<ObjectDefinition>();
                    } else {
                        // If we matched some objects, push them in the vector.
                        for (std::vector<Db::Identifier>::iterator
                             iti = instances.begin(); iti != instances.end(); ++iti) {
                            objects.push_back(ObjectDefinition(it->kind, *iti));
                        }
                    }
                } else {
                    // If we have not any obects extracted yet and in the context is object, push it in the vector
                    objects.push_back(ObjectDefinition(it->kind, it->name));
                }
            } else {
                for (std::vector<ObjectDefinition>::iterator itoe = objects.begin(); itoe != objects.end(); ++itoe) {
                    if (pathToVector(itoe->name).back().empty())
                        throw std::logic_error("Deska::Cli::DbInteraction::expandContextStack: Can not expand context stack with objects with empty names.");
                }
                if (it->itemType == ContextStackItem::CONTEXT_STACK_ITEM_TYPE_FILTER) {
                    // If there is a filter, we have to construct filter for all extracted objects at first.
                    std::vector<Db::Filter> currObjects;
                    for (std::vector<ObjectDefinition>::iterator ito = objects.begin(); ito != objects.end(); ++ito) {
                        currObjects.push_back(Db::AttributeExpression(
                            Db::FILTER_COLUMN_EQ, ito->kind, "name", Db::Value(ito->name)));
                    }
                    Db::Filter currObjectsFilter = Db::OrFilter(currObjects);
                    std::vector<Db::Filter> finalFilter;
                    finalFilter.push_back(currObjectsFilter);
                    if (it->filter)
                        finalFilter.push_back(*(it->filter));
                    // Now when we have filter for all extracted objects, we can chain it with the filter in the context
                    // using Db::AndFilter and get instances of all objects satisfying this filter.
                    std::vector<Db::Identifier> instances = m_api->kindInstances(it->kind, Db::Filter(
                        Db::AndFilter(finalFilter)));
                    if (instances.empty()) {
                        // If the filter does not match any objects, return empty vector, because it does not make
                        // a sense to extract objects using the rest of the context now.
                        return std::vector<ObjectDefinition>();
                    } else {
                        // When we matched some objects, we replace the content of the vector with new objects.
                        objects.clear();
                        for (std::vector<Db::Identifier>::iterator
                             iti = instances.begin(); iti != instances.end(); ++iti) {
                            objects.push_back(ObjectDefinition(it->kind, *iti));
                        }
                    }

                } else {
                    // If we have already some objects and in the context is object now, step in its context
                    // for each extracted object.
                    for (size_t i = 0; i < objects.size(); ++i) {
                        objects[i] = stepInContext(objects[i], ObjectDefinition(it->kind, it->name));
                    }
                }
            }
        }
        if (stableView)
            for (std::vector<ObjectDefinition>::iterator it = objects.begin(); it != objects.end(); ++it)
                objectExistsCache[*it] = true;
    }

    return objects;
}



bool DbInteraction::readonlyAttribute(const Db::Identifier &kind, const Db::Identifier &attribute)
{
    return (std::find(readonlyAttributes[kind].begin(), readonlyAttributes[kind].end(), attribute) !=
        readonlyAttributes[kind].end());
}



void DbInteraction::lockCurrentChangeset()
{
    m_api->lockCurrentChangeset();
}



void DbInteraction::unlockCurrentChangeset()
{
    m_api->unlockCurrentChangeset();
}



void DbInteraction::freezeView()
{
    stableView = true;
    clearCache();
    m_api->freezeView();
}



void DbInteraction::unFreezeView()
{
    stableView = false;
    clearCache();
    m_api->unFreezeView();
}



void DbInteraction::clearCache()
{
    objectExistsCache.clear();
}



void DbInteraction::connectedObjectsTransitivelyRec(const ObjectDefinition &object,
                                                 std::vector<ObjectDefinition> &containedObjects)
{
    // Kinds, that this kind contains
    for (std::vector<Db::Identifier>::iterator it =
        contains[object.kind].begin(); it != contains[object.kind].end(); ++it) {
        std::vector<Db::Identifier> instances = m_api->kindInstances(*it,
            Db::Filter(Db::AttributeExpression(Db::FILTER_COLUMN_EQ, *it, "name", Db::Value(object.name))));
        BOOST_ASSERT(instances.size() <= 1);
        if (!instances.empty()) {
            BOOST_ASSERT(instances.front() == object.name);
            ObjectDefinition mObj(*it, object.name);
            if (stableView)
                objectExistsCache[mObj] = true;
            if (std::find(containedObjects.begin(), containedObjects.end(), mObj) == containedObjects.end()) {
                containedObjects.push_back(mObj);
                connectedObjectsTransitivelyRec(mObj, containedObjects);
            }
        }
    }

    // Kinds containined by this kind
    for (std::vector<Db::Identifier>::iterator it =
        containable[object.kind].begin(); it != containable[object.kind].end(); ++it) {
        std::vector<Db::Identifier> instances = m_api->kindInstances(*it,
            Db::Filter(Db::AttributeExpression(Db::FILTER_COLUMN_EQ, *it, "name", Db::Value(object.name))));
        BOOST_ASSERT(instances.size() <= 1);
        if (!instances.empty()) {
            BOOST_ASSERT(instances.front() == object.name);
            ObjectDefinition mObj(*it, object.name);
            if (stableView)
                objectExistsCache[mObj] = true;
            if (std::find(containedObjects.begin(), containedObjects.end(), mObj) == containedObjects.end()) {
                containedObjects.push_back(mObj);
                connectedObjectsTransitivelyRec(mObj, containedObjects);
            }
        }
    }
}



void DbInteraction::allNestedObjectsTransitivelyRec(const ObjectDefinition &object,
                                                    std::vector<ObjectDefinition> &nestedObjects)
{
    for (std::vector<Db::Identifier>::iterator it = embeds[object.kind].begin(); it != embeds[object.kind].end(); ++it) {
        std::vector<Db::Identifier> emb = m_api->kindInstances(*it, Db::Filter(
            Db::AttributeExpression(Db::FILTER_COLUMN_EQ, object.kind, "name", Db::Value(object.name))));
        for (std::vector<Db::Identifier>::iterator ite = emb.begin(); ite != emb.end(); ++ite) {
            ObjectDefinition nObj(*it, *ite);
            if (stableView)
                objectExistsCache[nObj] = true;
            if (std::find(nestedObjects.begin(), nestedObjects.end(), nObj) == nestedObjects.end()) {
                nestedObjects.push_back(nObj);
                allNestedObjectsTransitivelyRec(nObj, nestedObjects);
            }
        }
    }
}



}
}
