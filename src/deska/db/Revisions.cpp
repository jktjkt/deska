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
#include <boost/algorithm/string/predicate.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>
#include "Revisions.h"

namespace Deska {
namespace Db {

RevisionId::RevisionId(const unsigned int revision): r(revision)
{
    if (revision == 0) {
        throw std::runtime_error("Revision r0 is invalid");
    }
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


TemporaryChangesetId::TemporaryChangesetId(const unsigned int revision): t(revision)
{
    if (revision == 0) {
        throw std::runtime_error("Changeset tmp0 is invalid");
    }
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

PendingChangeset::PendingChangeset(const TemporaryChangesetId revision_, const std::string &author_,
    const boost::posix_time::ptime timestamp_, const RevisionId parentRevision_, const std::string &message_,
    const AttachStatus attachStatus_, const boost::optional<std::string> &activeConnection_):
    revision(revision_), author(author_), timestamp(timestamp_), parentRevision(parentRevision_), message(message_),
    attachStatus(attachStatus_), activeConnectionInfo(activeConnection_)
{
}

bool operator==(const PendingChangeset &a, const PendingChangeset &b)
{
    return a.revision == b.revision && a.author == b.author && a.timestamp == b.timestamp &&
           a.parentRevision == b.parentRevision && a.message == b.message &&
           a.attachStatus == b.attachStatus && a.activeConnectionInfo == b.activeConnectionInfo;
}

bool operator!=(const PendingChangeset &a, const PendingChangeset &b)
{
    return !(a==b);
}

std::ostream& operator<<(std::ostream &stream, const PendingChangeset &p)
{
    return stream << "PendingChangeset(" << p.revision << ", " << p.author << ", " <<
                     p.timestamp << ", " << p.parentRevision << ", " << p.message << ", " <<
                     p.attachStatus << ", " <<
                     (!p.activeConnectionInfo ? std::string("[null]") : *p.activeConnectionInfo) << ")";
}

std::ostream& operator<<(std::ostream &stream, const PendingChangeset::AttachStatus status)
{
    switch (status) {
    case PendingChangeset::ATTACH_DETACHED:
        return stream << "DETACHED";
    case PendingChangeset::ATTACH_IN_PROGRESS:
        return stream << "IN_PROGRESS";
    }
    return stream << "[Invalid PendingChangeset::AttachStatus: " << static_cast<int>(status) << "]";
}

RevisionMetadata::RevisionMetadata(const RevisionId revision_, const std::string &author_,
    const boost::posix_time::ptime timestamp_, const std::string &commitMessage_):
    revision(revision_), author(author_), timestamp(timestamp_), commitMessage(commitMessage_)
{
}

bool operator==(const RevisionMetadata &a, const RevisionMetadata &b)
{
    return a.revision == b.revision && a.author == b.author && a.timestamp == b.timestamp && a.commitMessage == b.commitMessage;
}

bool operator!=(const RevisionMetadata &a, const RevisionMetadata &b)
{
    return !(a==b);
}

std::ostream& operator<<(std::ostream &stream, const RevisionMetadata &a)
{
    return stream << "RevisionMetadata(" << a.revision << ", " << a.author << ", " << a.timestamp << ", " << a.commitMessage << ")";
}


template<typename T> T extractRevisionFromJson(const std::string &prefix, const std::string &name, const std::string &str)
{
    if (boost::starts_with(str, prefix)) {
        try {
            return T(boost::lexical_cast<unsigned int>(str.substr(prefix.size())));
        } catch (const boost::bad_lexical_cast &) {
            std::ostringstream s;
            s << "Value \"" << str << "\" can't be interpreted as a " << name << ".";
            throw std::runtime_error(s.str());
        }
    } else {
        std::ostringstream s;
        s << "Value \"" << str << "\" does not look like a valid " << name << ".";
        throw std::runtime_error(s.str());
    }

}

RevisionId RevisionId::fromString(const std::string &s)
{
    return extractRevisionFromJson<RevisionId>("r", "RevisionId", s);
}

TemporaryChangesetId TemporaryChangesetId::fromString(const std::string &s)
{
    return extractRevisionFromJson<TemporaryChangesetId>("tmp", "TemporaryChangesetId", s);
}


}
}
