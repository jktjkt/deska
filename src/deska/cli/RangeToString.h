/*
* Copyright (C) 2011 Tomáš Hubík <hubik.tomas@gmail.com>
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

#ifndef DESKA_RANGETOSTRING_H
#define DESKA_RANGETOSTRING_H

#include <boost/range/iterator_range.hpp>
#include <string>

#include "ParserIterator.h"


namespace Deska
{
namespace Cli
{

/** @short Convert boost::iterator_range<class> to std::string */
template <typename Iterator>
class RangeToString
{
public:
    template <typename, typename>
        struct result { typedef void type; };

    void operator()(const boost::iterator_range<Iterator> &range, std::string &str) const
    {
        str.assign(range.begin(), range.end());
    }
};


/////////////////////////Template instances for linker//////////////////////////

template void RangeToString<iterator_type>::operator()(const boost::iterator_range<iterator_type> &rng, std::string &str) const;

}

}



#endif  // DESKA_RANGETOSTRING_H
