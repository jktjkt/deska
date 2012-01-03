/*
* Copyright (C) 2011 Tomáš Hubík <hubik.tomas@gmail.com>
*
* This file is part of the Deska, a tool for central administration of a grid site
* http://projects.flaska.net/projects/show/deska
*
* Inspired by an example from www.boost.org
*
* Copyright (c) 2001-2010 Hartmut Kaiser
* Copyright (c) 2001-2010 Joel de Guzman
* Copyright (c) 2003 Vaclav Vesely
*
* Distributed under the Boost Software License, Version 1.0. (See accompanying 
* file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
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

#include "ParserKeyword.h"

namespace Deska
{
namespace Cli
{


template <typename Tail>
typename traits::distinct_spec<Tail>::type distinct_spec(Tail const &tail)
{
    return repo::distinct(tail);
}



template <typename String>
typename traits::char_spec<String>::type char_spec(String const &str)
{
    return ascii::char_(str);
}



/////////////////////////Template instances for linker//////////////////////////

template traits::distinct_spec<traits::char_spec<std::string>::type>::type distinct_spec(traits::char_spec<std::string>::type const &tail);

template traits::char_spec<std::string>::type char_spec(std::string const &str);

}
}
