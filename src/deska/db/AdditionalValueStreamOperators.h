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

#ifndef DESKA_DB_ADDITIONAL_VALUE_STREAM_OPERATORS_H
#define DESKA_DB_ADDITIONAL_VALUE_STREAM_OPERATORS_H

#include <set>
#include <boost/date_time/posix_time/time_formatters.hpp>

namespace boost {
// FIXME: is there any safer way of doing this?
namespace gregorian {

/** @short Got to provide specialization in order for the linker to be happy */
inline std::ostream &operator<<(std::ostream &s, const boost::gregorian::date &d)
{
    return s << boost::gregorian::to_simple_string(d);
}

}

namespace posix_time {

/** @short Got to provide specialization in order for the linker to be happy */
inline std::ostream &operator<<(std::ostream &s, const boost::posix_time::ptime &t)
{
    return s << boost::posix_time::to_simple_string(t);
}

}

}

namespace std {
inline std::ostream &operator<<(std::ostream &stream, const std::set<std::string> &set)
{
    stream << "[";
    for (std::set<std::string>::const_iterator it = set.begin(); it != set.end(); ++it) {
        if (it != set.begin()) {
            stream << ", ";
        }
        stream << *it;
    }
    return stream << "]";
}

}

#endif // DESKA_DB_ADDITIONAL_VALUE_STREAM_OPERATORS_H
