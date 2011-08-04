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
#include <boost/spirit/include/phoenix.hpp>
#include "Api.h"

namespace Deska {
namespace Db {

Api::~Api()
{
}

std::vector<KindAttributeDataType> Api::kindAttributesWithoutRelation(const Identifier &kindName) const
{
    std::vector<KindAttributeDataType> attrs = kindAttributes(kindName);
    std::vector<KindAttributeDataType>::iterator begin = attrs.begin(), end = attrs.end();
    BOOST_FOREACH(const ObjectRelation &relation, kindRelations(kindName)) {
        using namespace boost::phoenix;
        switch (relation.kind) {
        case RELATION_EMBED_INTO:
            end = std::remove_if(begin, end, bind(&KindAttributeDataType::name, arg_names::_1) == relation.target);
            break;
        case RELATION_REFERS_TO:
        case RELATION_TEMPLATIZED:
        case RELATION_MERGE_WITH:
            // no special cases
            break;
        case RELATION_INVALID:
            BOOST_ASSERT(false);
            break;
        }
    }
    attrs.erase(end, attrs.end());
    return attrs;
}

RemoteDbError::RemoteDbError(const std::string &message): std::runtime_error(message)
{
}

/** @short Virtual destructor

The has to be defined and declared in order to force the vtable construction, which is needed for selectively catching
these eceptions.
*/
RemoteDbError::~RemoteDbError() throw ()
{
}

std::string RemoteDbError::whatWithBacktrace() const throw()
{
    // We're required not to throw, so we have to use a generic catch-all block here
    try {
        std::ostringstream ss;
        ss << "* " << backtrace("\n * ") << what() << std::endl;
        if (m_rawResponseData) {
            ss << "Server response was: " << *m_rawResponseData << std::endl;
        }
        return ss.str();
    } catch (...) {
        return what();
    }
}

void RemoteDbError::setRawResponseData(const std::string &data)
{
    m_rawResponseData = data;
}

#define REMOTEEXCEPTION(CLASS) \
CLASS::CLASS(const std::string &message): RemoteDbError(message) {} \
CLASS::~CLASS() throw () {}

REMOTEEXCEPTION(NotFoundError)
REMOTEEXCEPTION(InvalidKindError)
REMOTEEXCEPTION(InvalidAttributeError)
REMOTEEXCEPTION(NoChangesetError)
REMOTEEXCEPTION(ChangesetAlreadyOpenError)
REMOTEEXCEPTION(FreezingError)
REMOTEEXCEPTION(FilterError)
REMOTEEXCEPTION(ReCreateObjectError)
REMOTEEXCEPTION(RevisionParsingError)
REMOTEEXCEPTION(RevisionRangeError)
REMOTEEXCEPTION(ChangesetParsingError)
REMOTEEXCEPTION(ConstraintError)
REMOTEEXCEPTION(ObsoleteParentError)
REMOTEEXCEPTION(SqlError)
REMOTEEXCEPTION(ServerError)

#undef REMOTEEXCEPTION

}
}
