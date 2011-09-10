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

#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include "UnixFdIO.h"

namespace Deska {
namespace Db {

UnixFdIO::UnixFdIO(const int readingFd, const int writingFd)
{
    namespace bp = boost::process;
    bp::detail::file_handle fhRead(readingFd);
    bp::detail::file_handle fhWrite(writingFd);
    reading_.reset(new bp::pistream(fhRead));
    writing_.reset(new bp::postream(fhWrite));
    dynamic_cast<bp::detail::systembuf*>(reading_->rdbuf())->event_read_data.connect(
                boost::phoenix::bind(&UnixFdIO::slotReadData, this, boost::phoenix::arg_names::_1));
}

UnixFdIO::~UnixFdIO()
{
}

std::ostream *UnixFdIO::writeStream()
{
    return writing_.get();
}

std::istream *UnixFdIO::readStream()
{
    m_recentlyReadData.clear();
    return reading_.get();
}

void UnixFdIO::slotReadData(const std::string &data)
{
    m_recentlyReadData.append(data);
}

std::string UnixFdIO::recentlyReadData() const
{
    return m_recentlyReadData;
}

}
}
