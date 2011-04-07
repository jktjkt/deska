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

#ifndef DESKA_DB_REVISIONS_H
#define DESKA_DB_REVISIONS_H

#include <iosfwd>
#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace Deska {
namespace Db {

/** @short An identification of a persistent revision in the DB */
struct RevisionId {
    explicit RevisionId(const unsigned int revision);
    static RevisionId fromJson(const std::string &jsonStr);
    unsigned int r;
    static RevisionId null;
};

bool operator==(const RevisionId a, const RevisionId b);
bool operator!=(const RevisionId a, const RevisionId b);
std::ostream& operator<<(std::ostream &stream, const RevisionId r);

/** @short An identification of a temporary changeset in the DB */
struct TemporaryChangesetId {
    explicit TemporaryChangesetId(const unsigned int tmp);
    static TemporaryChangesetId fromJson(const std::string &jsonStr);
    unsigned int t;
    static TemporaryChangesetId null;
};

bool operator==(const TemporaryChangesetId a, const TemporaryChangesetId b);
bool operator!=(const TemporaryChangesetId a, const TemporaryChangesetId b);
std::ostream& operator<<(std::ostream &stream, const TemporaryChangesetId t);

/** @short Metadata for a pending changeset */
struct PendingChangeset {
    /** @short Temporary changeset number */
    TemporaryChangesetId revision;
    /** @short Author of the changeset */
    std::string author;
    /** @short Is this changeset attached to by someone at this time? */
    bool isAttachedNow;
    /** @short Timestamp of when this reviosion was modified the last time */
    boost::posix_time::ptime timestamp;
    /** @short Indetification of a revision which serves as a parent for this one */
    RevisionId parentRevision;
    /** @short The commit message */
    std::string message;
    PendingChangeset(const TemporaryChangesetId revision, const std::string &author, const bool isAttachedNow,
                     const boost::posix_time::ptime timestamp, const RevisionId parentRevision, const std::string &message);
};

bool operator==(const PendingChangeset &a, const PendingChangeset &b);
bool operator!=(const PendingChangeset &a, const PendingChangeset &b);
std::ostream& operator<<(std::ostream &stream, const PendingChangeset &p);

}
}

#endif // DESKA_DB_REVISIONS_H
