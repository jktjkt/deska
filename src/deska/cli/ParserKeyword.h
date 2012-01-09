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

#ifndef DESKA_PARSERKEYWORD_H
#define DESKA_PARSERKEYWORD_H

#include <boost/spirit/include/qi_char.hpp>
#include <boost/spirit/include/qi_string.hpp>
#include <boost/spirit/include/qi_nonterminal.hpp>
#include <boost/spirit/include/qi_numeric.hpp>
#include <boost/spirit/include/qi_action.hpp>
#include <boost/spirit/include/qi_operator.hpp>
#include <boost/spirit/repository/include/qi_distinct.hpp>

namespace Deska
{
namespace Cli
{

namespace spirit = boost::spirit;
namespace ascii = boost::spirit::ascii;
namespace repo = boost::spirit::repository;


/** @short Metafunctions allowing to compute the type of the distinct() and ascii::char_() constructs */
namespace traits
{
    /** @short Metafunction allowing to get the type of any repository::distinct(...) construct */
    template <typename Tail>
    struct distinct_spec: spirit::result_of::terminal<repo::tag::distinct(Tail)>
    {
    };

    /** @short Metafunction allowing to get the type of any ascii::char_(...) construct */
    template <typename String>
    struct char_spec: spirit::result_of::terminal<spirit::tag::ascii::char_(String)>
    {
    };
};



/** @short Helper function allowing to create a ascii::char_() construct from an arbitrary string representation */
template <typename Tail>
typename traits::distinct_spec<Tail>::type distinct_spec(Tail const& tail);



/** @short Helper function allowing to create a ascii::char_() construct from an arbitrary string representation */
template <typename String>
typename traits::char_spec<String>::type char_spec(String const& str);



std::string const keyword_spec("0-9a-zA-Z_-");
std::string const value_spec("0-9a-zA-Z_->.:[],\"\'");

/** @short New Qi 'keyword' directive usable as a shortcut for a repository::distinct(char_(std::string("0-9a-zA-Z_->"))) */
traits::distinct_spec<traits::char_spec<std::string>::type>::type const keyword = distinct_spec(char_spec(keyword_spec));
/** @short New Qi 'parer_value' directive usable as a shortcut for a repository::distinct(char_(std::string("0-9a-zA-Z_->.:[],\"\'"))) */
traits::distinct_spec<traits::char_spec<std::string>::type>::type const parser_value = distinct_spec(char_spec(value_spec));


}
}

#endif  // DESKA_PARSERKEYWORD_H
