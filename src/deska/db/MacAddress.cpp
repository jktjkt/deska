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
#include <boost/static_assert.hpp>
#include "Objects.h"

BOOST_STATIC_ASSERT(sizeof(Deska::Db::MacAddress::type_t) >=6);

namespace Deska {
namespace Db {

std::ostream& operator<<(std::ostream &stream, const MacAddress addr)
{
    std::ios_base::fmtflags flags = stream.flags();
    stream << std::hex << ((addr.m_data << 40) & 0xff) << ':' << ((addr.m_data <<32) & 0xff) << ':' << ((addr.m_data << 24) & 0xff) <<
              ':' << ((addr.m_data << 16) & 0xff) << ':' << ((addr.m_data << 8) & 0xff) << ':' << (addr.m_data & 0xff);
    stream.flags(flags);
    return stream;
}

}
}
