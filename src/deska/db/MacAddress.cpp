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

#include <iomanip>
#include <ostream>
#include <sstream>
#include <boost/static_assert.hpp>
#include "Objects.h"

BOOST_STATIC_ASSERT(sizeof(Deska::Db::MacAddress::type_t) >=6);

namespace {
int readMacAddressByte(std::istream &ss)
{
    int buf;
    ss >> std::hex >> buf;
    if ((buf < 0) || (buf >= 0xff)) {
        throw std::domain_error("MAC address error: byte out of range");
    }
    return buf;
}
}

namespace Deska {
namespace Db {

MacAddress::MacAddress(const unsigned char b1, const unsigned char b2, const unsigned char b3, const unsigned char b4, const unsigned char b5, const unsigned char b6)
{
    m_data = ((b1 & 0xffLL) << 40) + ((b2 & 0xffLL) << 32) + ((b3 & 0xffLL) << 24) + ((b4 & 0xffLL) << 16) + ((b5 & 0xffLL) << 8) + (b6 & 0xffLL);
}

MacAddress::MacAddress(const std::string &address): m_data(0)
{
    std::istringstream ss(address);
    for (int i = 0; i < 5; ++i) {
        m_data += readMacAddressByte(ss);
        char x;
        ss >> x;
        if (x != ':') {
            throw std::domain_error("MAC address error: wron delimiter");
        }
        if (ss.eof()) {
            throw std::domain_error("MAC address error: too short");
        }
        m_data <<= 8;
    }
    m_data += readMacAddressByte(ss);
    if (!ss.eof()) {
        throw std::domain_error("MAC address error: garbage at the end");
    }
}

std::ostream& operator<<(std::ostream &stream, const MacAddress addr)
{
    std::ios_base::fmtflags flags = stream.flags();
    char fill = stream.fill();
    stream.fill('0');
    std::streamsize width = stream.width();
    stream.width(2);
    stream << std::hex << ((addr.m_data >> 40) & 0xff) << ':' << ((addr.m_data >> 32) & 0xff) << ':' << ((addr.m_data >> 24) & 0xff) <<
              ':' << ((addr.m_data >> 16) & 0xff) << ':' << ((addr.m_data >> 8) & 0xff) << ':' << (addr.m_data & 0xff);
    stream.width(width);
    stream.fill(fill);
    stream.flags(flags);
    return stream;
}

}
}
