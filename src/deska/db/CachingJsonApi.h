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

#ifndef DESKA_DB_CACHINGAPI_H
#define DESKA_DB_CACHINGAPI_H

#include "JsonApi.h"

namespace Deska {
namespace Db {

class FakeApi;

/** @short API implementation that caches metadata queries

This implementation of the DB API will ask for the metadata (the DB scheme) only once, upon the first request, and then
use that information to answer eventual further questions.  All functions for performing modifications are forwarded to
the real API implementation for execution.

In future, this could be extended to remember other stuff, like lists of object names etc etc, but none of these is
implemented at this point.

It looks like one can't create stuff like CachingApi<JsonApiParser> p( [JsonApiParser's constructor arguments go here] ),
so we have to stick with having having this "decorator" implemented as a dumb class setting the type of the base class in
stone.
*/
class CachingJsonApi: public JsonApiParser
{
public:
    CachingJsonApi();
    virtual ~CachingJsonApi();

    // Querying schema definition
    virtual std::vector<Identifier> kindNames() const;
    virtual std::vector<KindAttributeDataType> kindAttributes( const Identifier &kindName ) const;
    virtual std::vector<ObjectRelation> kindRelations( const Identifier &kindName ) const;

    // Other functions are not overriden
private:
    /** @short Ask the real underlying database for the metadata */
    void askForMetadata() const;

    /** @short Storage for values which were already retrieved */
    FakeApi *m_cache;

    /** @short Did we ask for the metadata already? */
    mutable bool m_askedForMetadata;
};

}
}

#endif // DESKA_DB_CACHINGAPI_H
