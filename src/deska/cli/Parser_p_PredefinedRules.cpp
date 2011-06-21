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

#include <boost/assert.hpp>
#include "Parser_p_PredefinedRules.h"
#include "deska/db/Api.h"

namespace Deska
{
namespace Cli
{


template <typename Iterator>
PredefinedRules<Iterator>::PredefinedRules()
{
    tQuotedString %= qi::lexeme['"' >> +(ascii::char_ - '"') >> '"'];
    tSimpleString %= qi::lexeme[+(ascii::char_ - ('"' | ascii::space))];
    tIdentifier %= qi::lexeme[+(ascii::alnum | '_')];
    tIPv4Octet %= qi::raw[qi::lexeme[!(qi::lit("0") >> qi::digit) >> qi::uint_parser<boost::uint8_t, 10, 1, 3>()]];
    tIPv4Addr %= qi::raw[qi::lexeme[qi::repeat(3)[tIPv4Octet >> qi::lit(".")] >> tIPv4Octet]];
    tMACHexPair %= qi::raw[qi::lexeme[qi::repeat(2)[ascii::xdigit]]];
    tMACAddr %= qi::raw[qi::lexeme[qi::repeat(5)[tMACHexPair >> qi::lit(":")] >> tMACHexPair]];
    tIPv6HexQuat %= qi::raw[qi::lexeme[qi::repeat(1, 4)[ascii::xdigit]]];
    tIPv6Addr %= qi::raw[qi::lexeme[(qi::repeat(7)[tIPv6HexQuat >> qi::lit(":")] >> tIPv6HexQuat) |
                 (qi::lit("::") >> qi::repeat(6)[tIPv6HexQuat >> qi::lit(":")] >> tIPv6HexQuat) |
                 (-(tIPv6HexQuat) >> qi::lit("::") >> qi::repeat(5)[tIPv6HexQuat >> qi::lit(":")] >> tIPv6HexQuat) |
                 (-(qi::repeat(0, 1)[(tIPv6HexQuat >> qi::lit(":")) - (tIPv6HexQuat >> qi::lit("::"))] >> tIPv6HexQuat) >>
                     qi::lit("::") >> qi::repeat(4)[tIPv6HexQuat >> qi::lit(":")] >> tIPv6HexQuat) |
                 (-(qi::repeat(0, 2)[(tIPv6HexQuat >> qi::lit(":")) - (tIPv6HexQuat >> qi::lit("::"))] >> tIPv6HexQuat) >>
                     qi::lit("::") >> qi::repeat(3)[tIPv6HexQuat >> qi::lit(":")] >> tIPv6HexQuat) |
                 (-(qi::repeat(0, 3)[(tIPv6HexQuat >> qi::lit(":")) - (tIPv6HexQuat >> qi::lit("::"))] >> tIPv6HexQuat) >>
                     qi::lit("::") >> qi::repeat(2)[tIPv6HexQuat >> qi::lit(":")] >> tIPv6HexQuat) |
                 (-(qi::repeat(0, 4)[(tIPv6HexQuat >> qi::lit(":")) - (tIPv6HexQuat >> qi::lit("::"))] >> tIPv6HexQuat) >>
                     qi::lit("::") >> tIPv6HexQuat >> qi::lit(":") >> tIPv6HexQuat) |
                 (-(qi::repeat(0, 5)[(tIPv6HexQuat >> qi::lit(":")) - (tIPv6HexQuat >> qi::lit("::"))] >> tIPv6HexQuat) >>
                     qi::lit("::") >> tIPv6HexQuat) |
                 (-(qi::repeat(0, 6)[(tIPv6HexQuat >> qi::lit(":")) - (tIPv6HexQuat >> qi::lit("::"))] >> tIPv6HexQuat) >>
                     qi::lit("::"))]];
    // FIXME: Does not restrict tDate enough to avoid entering wrong dates like 2008-02-31
    tDay %= qi::raw[qi::lexeme[(qi::char_("0-2") >> qi::digit) | (qi::char_("3") >> qi::char_("0-1"))]];
    tMonth %= qi::raw[qi::lexeme[(qi::char_("0") >> qi::digit) | (qi::char_("1") >> qi::char_("0-2"))]];
    tYear %= qi::raw[qi::lexeme[(qi::lit("19") >> qi::repeat(2)[qi::digit]) | (qi::char_("2-9") >> qi::repeat(3)[qi::digit])]];
    tDate %= qi::raw[qi::lexeme[tYear >> qi::lit("-") >> tMonth >> qi::lit("-") >> tDay]];
    tHour %= qi::raw[qi::lexeme[(qi::char_("0-1") >> qi::digit) | (qi::char_("2") >> qi::char_("0-3"))]];
    tMinOrSec %= qi::raw[qi::lexeme[qi::char_("0-5") >> qi::digit]];
    tTimeStamp %= qi::raw[qi::lexeme[tYear >> qi::lit("-") >> tMonth >> qi::lit("-") >> tDay >> qi::lit(" ") >>
                                     tHour >> qi::lit(":") >> tMinOrSec >> qi::lit(":") >> tMinOrSec]];

    rulesMap[Db::TYPE_IDENTIFIER] = tIdentifier
        [qi::_val = phoenix::static_cast_<std::string>(qi::_1)];
    rulesMap[Db::TYPE_IDENTIFIER].name("identifier (alphanumerical letters and _)");

    rulesMap[Db::TYPE_STRING] = (tQuotedString | tSimpleString)
        [qi::_val = phoenix::static_cast_<std::string>(qi::_1)];
    rulesMap[Db::TYPE_STRING].name("string");

    rulesMap[Db::TYPE_INT] = qi::int_
        [qi::_val = phoenix::static_cast_<int>(qi::_1)];
    rulesMap[Db::TYPE_INT].name("integer");

    rulesMap[Db::TYPE_DOUBLE] = qi::double_
        [qi::_val = phoenix::static_cast_<double>(qi::_1)];
    rulesMap[Db::TYPE_DOUBLE].name("double");

    rulesMap[Db::TYPE_IPV4_ADDRESS] = tIPv4Addr
        [qi::_val = phoenix::static_cast_<std::string>(qi::_1)];
    rulesMap[Db::TYPE_IPV4_ADDRESS].name("IPv4 address");

    rulesMap[Db::TYPE_IPV6_ADDRESS] = tIPv6Addr
        [qi::_val = phoenix::static_cast_<std::string>(qi::_1)];
    rulesMap[Db::TYPE_IPV6_ADDRESS].name("IPv6 address");

    rulesMap[Db::TYPE_MAC_ADDRESS] = tMACAddr
        [qi::_val = phoenix::static_cast_<std::string>(qi::_1)];
    rulesMap[Db::TYPE_MAC_ADDRESS].name("MAC address");

    rulesMap[Db::TYPE_DATE] = tDate
        [qi::_val = phoenix::static_cast_<std::string>(qi::_1)];
    rulesMap[Db::TYPE_DATE].name("Date in YYYY-MM-DD format");

    rulesMap[Db::TYPE_TIMESTAMP] = tTimeStamp
        [qi::_val = phoenix::static_cast_<std::string>(qi::_1)];
    rulesMap[Db::TYPE_TIMESTAMP].name("Timestamp in YYY-MM-DD HH:MM:SS format");

    objectIdentifier %= tIdentifier.alias();
    objectIdentifier.name("object identifier (alphanumerical letters and _)");
}



template <typename Iterator>
const qi::rule<Iterator, Db::Value(), ascii::space_type>& PredefinedRules<Iterator>::getRule(const Db::Type attrType)
{
    typename std::map<Db::Type, qi::rule<Iterator, Db::Value(), ascii::space_type> >::const_iterator
        it = rulesMap.find(attrType);
    if (it == rulesMap.end()) {
        // Normally, we'd simply assert() here (see git history), but this is a place which would get hit
        // when people add their own low-level data types to Deska. Given that asserts are optimized away
        // in release mode and that I (Jan) can imagine people not bothering with debug builds (you really
        // should build in debug, though), it's better to be explicit here and maybe even save some poor
        // guy some head scratching in future.
        // So, to some future fellow: if we just saved you time, please buy some beer to Tomas :).
        std::stringstream ss;
        ss << "PredefinedRules::getRule: no available grammar rule for parsing of low-level data type " <<
              attrType << ". See " << __FILE__ << ":" << __LINE__ << " for details.";
        throw std::domain_error(ss.str());
    } else {
        return it->second;
    }
}



template <typename Iterator>
const qi::rule<Iterator, Db::Identifier(), ascii::space_type>& PredefinedRules<Iterator>::getObjectIdentifier()
{
    return objectIdentifier;
}



/////////////////////////Template instances for linker//////////////////////////

template PredefinedRules<iterator_type>::PredefinedRules();
template const qi::rule<iterator_type, Db::Value(), ascii::space_type>& PredefinedRules<iterator_type>::getRule(const Db::Type attrType);
template const qi::rule<iterator_type, Db::Identifier(), ascii::space_type>& PredefinedRules<iterator_type>::getObjectIdentifier();

}
}
