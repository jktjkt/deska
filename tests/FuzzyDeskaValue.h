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

#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/test_tools.hpp>

/** @short Fuzzy comparator for Deska::Value which can dela with floating point values

@see json_objectData
*/
struct FuzzyTestCompareDeskaValue: public boost::static_visitor<>
{
    template <typename T, typename U> void operator()(const T &, const U &) const
    {
        BOOST_ERROR("Cannot compare different types for equality");
    }

    template <typename T> void operator()(const T &a, const T &b) const
    {
        BOOST_CHECK_EQUAL(a, b);
    }

    void operator()(const double &a, const double &b) const
    {
        BOOST_CHECK_CLOSE(a, b, 0.01);
    }

    void operator()(const int &a, const double &b) const
    {
        BOOST_CHECK_CLOSE(static_cast<double>(a), b, 0.01);
    }

    void operator()(const double &a, const int &b) const
    {
        BOOST_CHECK_CLOSE(a, static_cast<double>(b), 0.01);
    }
};

