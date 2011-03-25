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

CachingJsonApi::CachingJsonApi(): m_askedForMetadata(false)
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

}
}
