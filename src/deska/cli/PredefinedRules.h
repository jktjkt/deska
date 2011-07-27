/*
* Copyright (C) 2011 Jan Kundrát <kundratj@fzu.cz>
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

#ifndef DESKA_CLI_PREDEFINEDRULES_H
#define DESKA_CLI_PREDEFINEDRULES_H

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>

#include <deska/db/Objects.h>

namespace Deska
{
namespace Cli
{

namespace ascii = boost::spirit::ascii;
namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

/** @short Predefined rules for parsing single attribute values and identifiers.
*   
*   Any new type have to be defined here.
*   String types have to be defined using one extra rule in this way:
*       qi::rule<Iterator, std::string(), ascii::space_type> tStringValue %= ....
*       rulesMap[Db::TYPE_NAME] = tStringValue[qi::_val = phoenix::static_cast_<std::string>(qi::_1)];
*   This is because of proper initialization of boost::variant and because without the extra rule boost will
*   push every character in the variant separately, that is very unefficient.
*/
template <typename Iterator>
class PredefinedRules
{

public:

    /** @short Fills internal map with predefined rules. */
    PredefinedRules();

    /** @short Function for getting single rules, that can be used in attributes grammar.
    *
    *   @param attrType Type of the attribute in question, @see Type.
    *   @return Rule that parses specific type of attribute.
    */
    const qi::rule<Iterator, Db::Value(), ascii::space_type>& getRule(const Db::Type attrType);

    /** @short Function for getting rule used to parse identifier of top-level objects. */
    const qi::rule<Iterator, Db::Identifier(), ascii::space_type>& getObjectIdentifier();

private:

    //@{
    /** Extra rules used for definition of string types. */
    qi::rule<Iterator, std::string(), ascii::space_type> tQuotedString;
    qi::rule<Iterator, std::string(), ascii::space_type> tSimpleString;
    qi::rule<Iterator, std::string(), ascii::space_type> tIdentifier;
    //@}

    //@{
    /** Extra rules used for definition of IPv4 addresses. */
    qi::rule<Iterator, std::string()> tIPv4Octet;
    qi::rule<Iterator, std::string(), ascii::space_type> tIPv4Addr;
    //@}

    //@{
    /** Extra rules used for definition of IPv6 addresses. */
    qi::rule<Iterator, std::string()> tIPv6HexQuat;
    qi::rule<Iterator, std::string(), ascii::space_type> tIPv6Addr;
    //@}

    //@{
    /** Extra rules used for definition of MAC addresses. */
    qi::rule<Iterator, std::string()> tMACHexPair;
    qi::rule<Iterator, std::string(), ascii::space_type> tMACAddr;
    //@}

    //@{
    /** Extra rules used for definition of date and timestamp. */
    qi::rule<Iterator, std::string()> tDay;
    qi::rule<Iterator, std::string()> tMonth;
    qi::rule<Iterator, std::string()> tYear;
    qi::rule<Iterator, std::string()> tHour;
    qi::rule<Iterator, std::string()> tMinOrSec;
    qi::rule<Iterator, std::string(), ascii::space_type> tDate;
    qi::rule<Iterator, std::string(), ascii::space_type> tTimeStamp;
    //@}

    /** Map where all rules are stored, @see Type. */
    std::map<Db::Type, qi::rule<Iterator, Db::Value(), ascii::space_type> > rulesMap;
    /** Rule for parsing identifiers of top-level objects. */
    qi::rule<Iterator, Db::Identifier(), ascii::space_type> objectIdentifier;

};

}
}

#endif  // DESKA_CLI_PREDEFINEDRULES_H
