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
#include "deska/db/Api.h"

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
                embeddedInto[*itk] = itr->target;
            }
            if (itr->kind == Db::RELATION_MERGE_WITH) {
                mergeWith[*itk].push_back(itr->target);
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
        // If the name of new object is specified, returned name should be the same
        if (!(pathToVector(objects.front().name).back().empty()))
            BOOST_ASSERT(newObjectName == objects.front().name);
        return ContextStackItem(objects.front().kind, pathToVector(newObjectName).back());
    }

    // FIXME: Wait for implementation of batched changes on server side.
    //std::vector<Db::ObjectModification> modifications;
    for (std::vector<ObjectDefinition>::iterator it = objects.begin(); it != objects.end(); ++it) {
        if (!objectExists(*it))
            //modifications.push_back(Db::CreateObjectModification(it->kind, it->name));
            m_api->createObject(it->kind, it->name);
    }
    //m_api->applyBatchedChanges(modifications);

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
        if (!objectExists(*it))
            m_api->restoreDeletedObject(it->kind, it->name);
    }
}



void DbInteraction::deleteObject(const ContextStack &context)
{
    BOOST_ASSERT(!context.empty());
    std::vector<ObjectDefinition> objects = expandContextStack(context);
    // FIXME: Wait for implementation of batched changes on server side.
    //std::vector<Db::ObjectModification> modifications;
    for (std::vector<ObjectDefinition>::iterator it = objects.begin(); it != objects.end(); ++it) {
        if (objectExists(*it))
            //modifications.push_back(Db::DeleteObjectModification(it->kind, it->name));
            m_api->deleteObject(it->kind, it->name);
    }
    //m_api->applyBatchedChanges(modifications);
}



void DbInteraction::renameObject(const ContextStack &context, const Db::Identifier &newName)
{
    BOOST_ASSERT(!context.empty());
    std::vector<ObjectDefinition> objects = expandContextStack(context);
    // FIXME: Wait for implementation of batched changes on server side.
    //std::vector<Db::ObjectModification> modifications;
    for (std::vector<ObjectDefinition>::iterator it = objects.begin(); it != objects.end(); ++it) {
        std::vector<Db::Identifier> newObjName = pathToVector(it->name);
        newObjName.back() = newName;
        //modifications.push_back(Db::RenameObjectModification(it->kind, it->name, vectorToPath(newObjName)));
        m_api->renameObject(it->kind, it->name, vectorToPath(newObjName));
    }
    //m_api->applyBatchedChanges(modifications);
}



void DbInteraction::setAttribute(const ContextStack &context,
                                 const AttributeDefinition &attribute)
{
    BOOST_ASSERT(!context.empty());
    std::vector<ObjectDefinition> objects = expandContextStack(context);
    // FIXME: Wait for implementation of batched changes on server side.
    //std::vector<Db::ObjectModification> modifications;
    for (std::vector<ObjectDefinition>::iterator it = objects.begin(); it != objects.end(); ++it) {
        //modifications.push_back(Db::SetAttributeModification(it->kind, it->name, attribute.attribute,
        //                                                     attribute.value));
        m_api->setAttribute(it->kind, it->name, attribute.attribute, attribute.value);
    }
    //m_api->applyBatchedChanges(modifications);
}



void DbInteraction::setAttributeInsert(const ContextStack &context, const Db::Identifier &set,
                                       const Db::Identifier &identifier)
{
    BOOST_ASSERT(!context.empty());
    std::vector<ObjectDefinition> objects = expandContextStack(context);
    // FIXME: Wait for implementation of batched changes on server side.
    //std::vector<Db::ObjectModification> modifications;
    for (std::vector<ObjectDefinition>::iterator it = objects.begin(); it != objects.end(); ++it) {
        //modifications.push_back(Db::SetAttributeInsertModification(it->kind, it->name, set,
        //                                                           identifier));
        m_api->setAttributeInsert(it->kind, it->name, set, identifier);
    }
    //m_api->applyBatchedChanges(modifications);
}



void DbInteraction::setAttributeRemove(const ContextStack &context, const Db::Identifier &set,
                                       const Db::Identifier &identifier)
{
    BOOST_ASSERT(!context.empty());
    std::vector<ObjectDefinition> objects = expandContextStack(context);
    // FIXME: Wait for implementation of batched changes on server side.
    //std::vector<Db::ObjectModification> modifications;
    for (std::vector<ObjectDefinition>::iterator it = objects.begin(); it != objects.end(); ++it) {
        //modifications.push_back(Db::SetAttributeRemoveModification(it->kind, it->name, set,
        //                                                           identifier));
        m_api->setAttributeRemove(it->kind, it->name, set, identifier);
    }
    //m_api->applyBatchedChanges(modifications);
}



void DbInteraction::removeAttribute(const ContextStack &context,
                                    const Db::Identifier &attribute)
{
    BOOST_ASSERT(!context.empty());
    std::vector<ObjectDefinition> objects = expandContextStack(context);
    // FIXME: Wait for implementation of batched changes on server side.
    //std::vector<Db::ObjectModification> modifications;
    for (std::vector<ObjectDefinition>::iterator it = objects.begin(); it != objects.end(); ++it) {
        //modifications.push_back(Db::SetAttributeModification(it->kind, it->name, attribute, Deska::Db::Value()));
        m_api->setAttribute(it->kind, it->name, attribute, Deska::Db::Value());
    }
    //m_api->applyBatchedChanges(modifications);
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

    // FIXME: Allow obtaining really resolved data.
    /*typedef std::map<Db::Identifier, std::pair<Db::Identifier, Db::Value> > ObjectDataMap;
    BOOST_FOREACH(const ObjectDataMap::value_type &x, m_api->resolvedObjectDataWithOrigin(object.kind, object.name)) {
        attributes.push_back(std::make_pair<AttributeDefinition, Db::Identifier>(
            AttributeDefinition(x.first, x.second.second), (x.second.first == object.name ? "" : x.second.first)));*/
    typedef std::map<Db::Identifier, Db::Value> ObjectDataMap;
    BOOST_FOREACH(const ObjectDataMap::value_type &x, m_api->objectData(object.kind, object.name)) {
        attributes.push_back(std::make_pair<AttributeDefinition, Db::Identifier>(
            AttributeDefinition(x.first, x.second), ""));
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
    std::vector<ObjectDefinition> kinds;
    for (std::vector<Db::Identifier>::iterator it = embeds[object.kind].begin(); it != embeds[object.kind].end(); ++it) {
        // FIXME: Db::FilterError: "Item 'column' is missing in condition."
        /*std::vector<Db::Identifier> emb = m_api->kindInstances(*it, Db::Filter(
            Db::AttributeExpression(Db::FILTER_COLUMN_EQ, object.kind, "name", Db::Value(object.name))));
        for (std::vector<Db::Identifier>::iterator ite = emb.begin(); ite != emb.end(); ++ite)
            kinds.push_back(ObjectDefinition(*it, *ite));*/
    }
    return kinds;
}



std::vector<ObjectDefinition> DbInteraction::allNestedObjects(const ContextStack &context)
{
    if (!context.empty())
        return allNestedObjects(ObjectDefinition(context.back().kind, contextStackToPath(context)));
    else
        return std::vector<ObjectDefinition>();
}



bool DbInteraction::objectExists(const ObjectDefinition &object)
{
    if (object.name.empty())
        return false;

    std::vector<Db::Identifier> instances = m_api->kindInstances(object.kind,
        Db::Filter(Db::AttributeExpression(Db::FILTER_COLUMN_EQ, object.kind, "name", Db::Value(object.name))));
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



std::vector<ObjectDefinition> DbInteraction::mergedObjects(const ObjectDefinition &object)
{
    if (object.name.empty())
        throw std::logic_error("Deska::Cli::DbInteraction::mergedObjects: Can not find merged objects for context stack with filters or kinds without names.");

    std::vector<ObjectDefinition> mergedObjects;
    for (std::vector<Db::Identifier>::iterator it =
        mergeWith[object.kind].begin(); it != mergeWith[object.kind].end(); ++it) {
        std::vector<Db::Identifier> instances = m_api->kindInstances(*it,
            Db::Filter(Db::AttributeExpression(Db::FILTER_COLUMN_EQ, *it, "name", Db::Value(object.name))));
        BOOST_ASSERT(instances.size() <= 1);
        if (!instances.empty()) {
            BOOST_ASSERT(instances.front() == object.name);
            mergedObjects.push_back(ObjectDefinition(*it, object.name));
        }
    }
   
    return mergedObjects;
}



std::vector<ObjectDefinition> DbInteraction::mergedObjects(const ContextStack &context)
{
    BOOST_ASSERT(!context.empty());
    for (ContextStack::const_iterator it = context.begin(); it != context.end(); ++it) {
        if (it->filter || it->name.empty())
            throw std::logic_error("Deska::Cli::DbInteraction::mergedObjects: Can not find merged objects for context stack with filters or kinds without names.");
    }

    return mergedObjects(ObjectDefinition(context.back().kind, contextStackToPath(context)));
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



std::string DbInteraction::configDiff(bool forceRegenerate)
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
    } catch (std::runtime_error &e) {
        for (ContextStack::const_iterator it = context.begin(); it != context.end(); ++it) {
            if (objects.empty()) {
                if (it->filter) {
                    // If we have not any objects extracted yet, get some using filter when in context stack is filter
                    std::vector<Db::Identifier> instances = m_api->kindInstances(it->kind, *(it->filter));
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
                if (it->filter) {
                    // If there is a filter, we have to construct filter for all extracted objects at first.
                    std::vector<Db::Filter> currObjects;
                    for (std::vector<ObjectDefinition>::iterator ito = objects.begin(); ito != objects.end(); ++ito) {
                        currObjects.push_back(Db::AttributeExpression(
                            Db::FILTER_COLUMN_EQ, ito->kind, "name", Db::Value(ito->name)));
                    }
                    Db::Filter currObjectsFilter = Db::OrFilter(currObjects);
                    std::vector<Db::Filter> finalFilter;
                    finalFilter.push_back(currObjectsFilter);
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
    }

    return objects;
}



void DbInteraction::lockCurrentChangeset()
{
    m_api->lockCurrentChangeset();
}



void DbInteraction::unlockCurrentChangeset()
{
    m_api->unlockCurrentChangeset();
}



}
}
