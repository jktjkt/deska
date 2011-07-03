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

#include "Filter.h"

namespace Deska {
namespace Db {

std::ostream& operator<<(std::ostream &stream, ComparisonOperator o)
{
    switch (o) {
    case FILTER_COLUMN_EQ:
        return stream << "==";
    case FILTER_COLUMN_NE:
        return stream << "!=";
    case FILTER_COLUMN_GT:
        return stream << ">";
    case FILTER_COLUMN_GE:
        return stream << ">=";
    case FILTER_COLUMN_LT:
        return stream << "<";
    case FILTER_COLUMN_LE:
        return stream << "<=";
    }
    return stream << "[Invalid operator:" << static_cast<int>(o) << "]";
}

MetadataExpression::MetadataExpression(const ComparisonOperator comparison_, const Identifier &metadata_, const MetadataValue &constantValue_):
    comparison(comparison_), metadata(metadata_), constantValue(constantValue_)
{
}

bool operator==(const MetadataExpression &a, const MetadataExpression &b)
{
    return a.comparison == b.comparison && a.metadata == b.metadata && a.constantValue == b.constantValue;
}

bool operator!=(const MetadataExpression &a, const MetadataExpression &b)
{
    return !(a==b);
}

std::ostream& operator<<(std::ostream &stream, const MetadataExpression &m)
{
    return stream << m.metadata << " " << m.comparison << " " << m.constantValue;
}

AttributeExpression::AttributeExpression(const ComparisonOperator comparison_, const Identifier &kind_, const Identifier &attribute_,
                                         const Value &constantValue_):
    comparison(comparison_), kind(kind_), attribute(attribute_), constantValue(constantValue_)
{
}

bool operator==(const AttributeExpression &a, const AttributeExpression &b)
{
    return a.comparison == b.comparison && a.kind == b.kind && a.attribute == b.attribute && a.constantValue == b.constantValue;
}

bool operator!=(const AttributeExpression &a, const AttributeExpression &b)
{
    return !(a==b);
}

std::ostream& operator<<(std::ostream &stream, const AttributeExpression &a)
{
    return stream << a.kind << "." << a.attribute << " " << a.comparison << " " << a.constantValue;
}

OrFilter::OrFilter(const std::vector<Filter> &operands_):
    operands(operands_)
{
}
/*
std::ostream& operator<<(std::ostream &stream, const OrFilter &o)
{
    for (std::vector<Filter>::const_iterator it = o.operands.begin(); it != o.operands.end(); ++it) {
        if (it != o.operands.begin())
            stream << " & ";
        stream << *it;
    }
    return stream;
}
*/
AndFilter::AndFilter(const std::vector<Filter> &operands_):
    operands(operands_)
{
}
/*
std::ostream& operator<<(std::ostream &stream, const AndFilter &a)
{
    for (std::vector<Filter>::const_iterator it = a.operands.begin(); it != a.operands.end(); ++it) {
        if (it != a.operands.begin())
            stream << " | ";
        stream << *it;
    }
    return stream;
}
*/
bool operator!=(const Expression &a, const Expression &b)
{
    return !(a==b);
}
/*
std::ostream& operator<<(std::ostream &stream, const Filter &f)
{
    return stream << "(" << f << ")";
}*/

}
}
