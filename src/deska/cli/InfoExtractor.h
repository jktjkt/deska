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

#ifndef DESKA_CLI_INFO_EXTRACTOR_H
#define DESKA_CLI_INFO_EXTRACTOR_H

#include <boost/spirit/include/qi.hpp>


namespace Deska
{
namespace Cli
{


/** @short Extracts keywords from boost::spirit::info into a vector of strings
*
*   This class is used as a visitor of boost::spirit::info to extract keywords from it to a std::vector passed
*   to the constructor.
*/
class InfoExtractor
{

public:

    /** @short Constructor only saves pointer to lists, where the data will be extracted.
    *   
    *   @param keywordsList Pointer to list, where keywords will be extracted
    *   @param typesList Pointer to list, where names of value types will be extracted
    */
    InfoExtractor(std::vector<std::string> *keywordsList, std::vector<std::string> *typesList);

    /** @short Function used for extractin the keywords from boost::spirit::info.
    *
    *   Extracts info into lists given in constructor.
    *
    *   @param tag Tag, that is the name of value type, when keyword is empty
    *   @param value Keyword or empty string
    */
    void element(boost::spirit::utf8_string const& tag, boost::spirit::utf8_string const& value, int) const;

private:

    /** List for extracted keywords. */
    std::vector<std::string> *kList;
    /** List for extracted names of value types. */
    std::vector<std::string> *tList;
};


}
}


#endif // DESKA_CLI_INFO_EXTRACTOR_H
