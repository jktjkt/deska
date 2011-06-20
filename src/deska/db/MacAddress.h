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

#ifndef DESKA_DB_MACADDRESS_H
#define DESKA_DB_MACADDRESS_H

#include <iosfwd>

namespace Deska {
namespace Db {

/** @short Representation of a MAC address */
class MacAddress {
    friend std::ostream& operator<<(std::ostream&, const MacAddress);
public:
    typedef unsigned long long type_t;

    /** @short Construct a MAC address from the list of byte values */
    MacAddress(const unsigned char b1, const unsigned char b2, const unsigned char b3, const unsigned char b4, const unsigned char b5, const unsigned char b6);

    /** @short Construct a MAC address from its string representation */
    explicit MacAddress(const std::string &address);

    /** @short Compare two MAC addresses for equality */
    bool operator==(const MacAddress other) const throw() { return m_data == other.m_data; }
    /** @short Compare two MAC addresses for not being equal */
    bool operator!=(const MacAddress other) const throw() { return m_data != other.m_data; }
    /** @short Is this address smaller than the other one? */
    bool operator<(const MacAddress other) const throw() { return m_data < other.m_data; }

private:
    type_t m_data;
};
/** @short Print the MAC address into an ostream */
std::ostream& operator<<(std::ostream &, const MacAddress);

}
}

#endif // DESKA_DB_MACADDRESS_H
