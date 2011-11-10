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
#include <boost/optional.hpp>

namespace Deska {
namespace Db {

/** @short An identification of a persistent revision in the DB */
struct RevisionId {
    explicit RevisionId(const unsigned int revision);
    /** @short Construct a RevisionId from a properly formatted string */
    static RevisionId fromString(const std::string &s);
    unsigned int r;
};

bool operator==(const RevisionId a, const RevisionId b);
bool operator!=(const RevisionId a, const RevisionId b);
std::ostream& operator<<(std::ostream &stream, const RevisionId r);

/** @short An identification of a temporary changeset in the DB */
struct TemporaryChangesetId {
    explicit TemporaryChangesetId(const unsigned int tmp);
    /** @short Construct a TemporaryChangesetId from a properly formatted string */
    static TemporaryChangesetId fromString(const std::string &s);
    unsigned int t;
};

bool operator==(const TemporaryChangesetId a, const TemporaryChangesetId b);
bool operator!=(const TemporaryChangesetId a, const TemporaryChangesetId b);
std::ostream& operator<<(std::ostream &stream, const TemporaryChangesetId t);

/** @short Metadata for a pending changeset */
struct PendingChangeset {
    /** @short Attached/detached state of a changeset */
    typedef enum {ATTACH_IN_PROGRESS, ATTACH_DETACHED} AttachStatus;

    /** @short Temporary changeset number */
    TemporaryChangesetId revision;
    /** @short Author of the changeset */
    std::string author;
    /** @short Timestamp of when this reviosion was modified the last time */
    boost::posix_time::ptime timestamp;
    /** @short Indetification of a revision which serves as a parent for this one */
    RevisionId parentRevision;
    /** @short The commit message */
    std::string message;
    /** @short Is this changeset attached to by someone at this time? */
    AttachStatus attachStatus;
    /** @short Optional free-text identification of who is currently attached to this changeset

    This field is meant as a debug hint for specifying who is currently working on this changeset.
    The specification does not mandate any particular form for the format of this field. It is recomended
    that DB implementations return something that a human user could use to locate a user who is currently
    connected to the database.
    */
    boost::optional<std::string> activeConnectionInfo;

    PendingChangeset(const TemporaryChangesetId revision, const std::string &author, const boost::posix_time::ptime timestamp,
                     const RevisionId parentRevision, const std::string &message, const AttachStatus attachStatus,
                     const boost::optional<std::string> &activeConnectionInfo);
};

bool operator==(const PendingChangeset &a, const PendingChangeset &b);
bool operator!=(const PendingChangeset &a, const PendingChangeset &b);
std::ostream& operator<<(std::ostream &stream, const PendingChangeset &p);
std::ostream& operator<<(std::ostream &stream, const PendingChangeset::AttachStatus status);

struct RevisionMetadata {
    /** @short Revision number */
    RevisionId revision;
    /** @short Author of the changeset */
    std::string author;
    /** @short Timestamp of when this reviosion was modified the last time */
    boost::posix_time::ptime timestamp;
    /** @short The commit message */
    std::string commitMessage;

    RevisionMetadata(const RevisionId revision, const std::string &author, const boost::posix_time::ptime timestamp,
                     const std::string &commitMessage);
};

bool operator==(const RevisionMetadata &a, const RevisionMetadata &b);
bool operator!=(const RevisionMetadata &a, const RevisionMetadata &b);
std::ostream& operator<<(std::ostream &stream, const RevisionMetadata &a);

}
}

#endif // DESKA_DB_REVISIONS_H
