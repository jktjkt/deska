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

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>
#include "Revisions.h"

namespace Deska
{

template<typename T> T extractRevisionFromJson(const std::string &prefix, const std::string &name, const std::string &jsonStr)
{
    if (boost::starts_with(jsonStr, prefix)) {
        try {
            return T(boost::lexical_cast<unsigned int>(jsonStr.substr(prefix.size())));
        } catch (const boost::bad_lexical_cast &) {
            std::ostringstream s;
            s << "Value \"" << jsonStr << "\" can't be interpreted as a " << name << ".";
            throw std::domain_error(s.str());
        }
    } else {
        std::ostringstream s;
        s << "Value \"" << jsonStr << "\" does not look like a valid " << name << ".";
        throw std::domain_error(s.str());
    }

}

RevisionId RevisionId::null = RevisionId(0);

RevisionId::RevisionId(const unsigned int revision): r(revision)
{
}

RevisionId RevisionId::fromJson(const std::string &jsonStr)
{
    return extractRevisionFromJson<RevisionId>("r", "RevisionId", jsonStr);
}

bool operator==(const RevisionId a, const RevisionId b)
{
    return a.r == b.r;
}

bool operator!=(const RevisionId a, const RevisionId b)
{
    return !(a==b);
}

std::ostream& operator<<(std::ostream &stream, const RevisionId r)
{
    return stream << "r" << r.r;
}


TemporaryChangesetId TemporaryChangesetId::null = TemporaryChangesetId(0);

TemporaryChangesetId::TemporaryChangesetId(const unsigned int revision): t(revision)
{
}

TemporaryChangesetId TemporaryChangesetId::fromJson(const std::string &jsonStr)
{
    return extractRevisionFromJson<TemporaryChangesetId>("tmp", "TemporaryChangesetId", jsonStr);
}

bool operator==(const TemporaryChangesetId a, const TemporaryChangesetId b)
{
    return a.t == b.t;
}

bool operator!=(const TemporaryChangesetId a, const TemporaryChangesetId b)
{
    return !(a==b);
}

std::ostream& operator<<(std::ostream &stream, const TemporaryChangesetId t)
{
    return stream << "tmp" << t.t;
}

}
