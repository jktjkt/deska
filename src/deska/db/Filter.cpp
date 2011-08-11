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

#include <boost/foreach.hpp>
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
    case FILTER_COLUMN_CONTAINS:
        return stream << "contains";
    case FILTER_COLUMN_NOT_CONTAINS:
        return stream << "not contains";
    }
    return stream << "[Invalid operator:" << static_cast<int>(o) << "]";
}

std::ostream& operator<<(std::ostream &stream, const SpecialFilterType f)
{
    switch (f) {
    case FILTER_SPECIAL_EMBEDDED_LAST_ONE:
        return stream << "embeddedLastOne";
    }
    return stream << "[Invalid operator:" << static_cast<int>(f) << "]";
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

SpecialExpression::SpecialExpression(const SpecialFilterType type_, const Identifier &kind_):
    type(type_), kind(kind_)
{
}

bool operator==(const SpecialExpression &a, const SpecialExpression &b)
{
    return a.type == b.type && a.kind == b.kind;
}

bool operator!=(const SpecialExpression &a, const SpecialExpression &b)
{
    return !(a==b);
}

std::ostream& operator<<(std::ostream &stream, const SpecialExpression &e)
{
    return stream << "SpecialExpression(" << e.kind << " " << e.type << ")";
}

/** @short Variant visitor that returns the type name of a Deska::Db::MetadataValue */
struct DeskaMetadataValueTypeName: public boost::static_visitor<std::string>
{
    result_type operator()(const Value &v) const
    {
        return "Value";
    }
    result_type operator()(const RevisionId &v) const
    {
        return "RevisionId";
    }
    result_type operator()(const TemporaryChangesetId &v) const
    {
        return "TemporaryChangesetId";
    }
    result_type operator()(const PendingChangeset::AttachStatus &v) const
    {
        return "PendingChangeset::AttachStatus";
    }
};

/** @short Variant visitor; helper for Deska::Db::MetadataValue's __repr__ */
struct DeskaMetadataValueRepr: public boost::static_visitor<std::string>
{
    result_type operator()(const Value &v) const
    {
        return repr_Value(v);
    }

    template<typename T>
    result_type operator()(const T &v) const
    {
        std::ostringstream ss;
        ss << v;
        return ss.str();
    }
};

/** @short Variant visitor; helper for Deska::Db::MetadataValue's __str__ */
struct DeskaMetadataValueToString: public boost::static_visitor<std::string>
{
    result_type operator()(const Value &v) const
    {
        return str_Value(v);
    }

    template<typename T>
    result_type operator()(const T &v) const
    {
        std::ostringstream ss;
        ss << v;
        return ss.str();
    }
};

std::string repr_MetadataValue(const MetadataValue &v)
{
    std::ostringstream ss;
    ss << "MetadataValue<" << boost::apply_visitor(DeskaMetadataValueTypeName(), v) << ">(" << boost::apply_visitor(DeskaMetadataValueRepr(), v) << ")";
    return ss.str();
}

std::string str_MetadataValue(const MetadataValue &v)
{
    return boost::apply_visitor(DeskaMetadataValueToString(), v);
}

std::ostream& operator<<(std::ostream &stream, const MetadataExpression &e)
{
    return stream << "MetadataExpression(" << e.metadata << " " << e.comparison << " " << repr_MetadataValue(e.constantValue) << ")";
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

std::ostream& operator<<(std::ostream &stream, const AttributeExpression &e)
{
    return stream << "AttributeExpression(" << e.kind << "." << e.attribute << " " << e.comparison << " " << repr_Value(e.constantValue) << ")";
}

OrFilter::OrFilter(const std::vector<Filter> &operands_):
    operands(operands_)
{
}

bool operator==(const OrFilter &a, const OrFilter &b)
{
    return a.operands == b.operands;
}

bool operator!=(const OrFilter &a, const OrFilter &b)
{
    return !(a==b);
}

std::ostream& operator<<(std::ostream &stream, const OrFilter &filter)
{
    stream << "OrFilter([";
    BOOST_FOREACH(const Filter& item, filter.operands) {
        stream << item << ", ";
    }
    return stream << "])";
}

AndFilter::AndFilter(const std::vector<Filter> &operands_):
    operands(operands_)
{
}

bool operator==(const AndFilter &a, const AndFilter &b)
{
    return a.operands == b.operands;
}

bool operator!=(const AndFilter &a, const AndFilter &b)
{
    return !(a==b);
}

std::ostream& operator<<(std::ostream &stream, const AndFilter &filter)
{
    stream << "AndFilter([";
    BOOST_FOREACH(const Filter& item, filter.operands) {
        stream << item << ", ";
    }
    return stream << "])";
}


bool operator!=(const Expression &a, const Expression &b)
{
    return !(a==b);
}

/** @short Helper visitor for Deska::Db::Expression's __repr__ */
struct DeskaExpressionToString: public boost::static_visitor<std::string>
{
    template<typename T>
    result_type operator()(const T &v) const
    {
        std::ostringstream ss;
        ss << "Expression(" << v << ")";
        return ss.str();
    }
};

std::string repr_Expression(const Expression &e)
{
    return boost::apply_visitor(DeskaExpressionToString(), e);
}

/** @short Helper visitor for Deska::Db::Filter's __repr__ */
struct DeskaFilterToString: public boost::static_visitor<std::string>
{
    template<typename T>
    result_type operator()(const T &v) const
    {
        std::ostringstream ss;
        ss << "Filter(" << v << ")";
        return ss.str();
    }
};

/** @short __repr__ for a Deska::Db::Filter */
std::string repr_Filter(const Filter &v)
{
    return boost::apply_visitor(DeskaFilterToString(), v);
}

}
}
