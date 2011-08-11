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

#ifndef DESKA_DB_FILTER_H
#define DESKA_DB_FILTER_H

#include <vector>

#include "deska/db/Objects.h"
#include "deska/db/Revisions.h"

namespace Deska {
namespace Db {

/** @short What kind of comparison are we doing */
typedef enum {
    /** @short Compare column's value with a constant for equality */
    FILTER_COLUMN_EQ,
    /** @short Compare column's value with a constant for not being equal */
    FILTER_COLUMN_NE,
    /** @short Compare column's value for being greater than a constant */
    FILTER_COLUMN_GT,
    /** @short Compare column's value for being greater-or-equal than a constant */
    FILTER_COLUMN_GE,
    /** @short Compare column's value for being less than a constant */
    FILTER_COLUMN_LT,
    /** @short Compare column's value for being less-or-equal than a constant */
    FILTER_COLUMN_LE,
    /** @short See if a column's value (which must be a set of identifiers) contains the specified item */
    FILTER_COLUMN_CONTAINS,
    /** @short See whether a column's value (a set of identifiers) does not contain the specified item */
    FILTER_COLUMN_NOT_CONTAINS
} ComparisonOperator;

std::ostream& operator<<(std::ostream &stream, const ComparisonOperator o);

/** @short Anything against which we can compare */
typedef boost::variant<Value,RevisionId,TemporaryChangesetId,PendingChangeset::AttachStatus> MetadataValue;

/** @short __repr__ for Deska::Db::MetadataValue */
std::string repr_MetadataValue(const MetadataValue &v);

/** @short __str__ for Deska::Db::MetadataValue */
std::string str_MetadataValue(const MetadataValue &v);

/** @short Compare metadata against a constant using  given comparison operator */
struct MetadataExpression
{
    ComparisonOperator comparison;
    Identifier metadata;
    MetadataValue constantValue;

    MetadataExpression(const ComparisonOperator comparison, const Identifier &metadata, const MetadataValue &constantValue);

    // FIXME: This is here for initialization of the Filter variant for the grammar.
    // Do not know, if there is some other solution now.
    MetadataExpression() {};
};

bool operator==(const MetadataExpression &a, const MetadataExpression &b);
bool operator!=(const MetadataExpression &a, const MetadataExpression &b);
std::ostream& operator<<(std::ostream &stream, const MetadataExpression &m);

/** @short Compare attribute value against a constant using given comparison operator */
struct AttributeExpression
{
    ComparisonOperator comparison;
    Identifier kind;
    Identifier attribute;
    Value constantValue;

    AttributeExpression(const ComparisonOperator comparison, const Identifier &kind, const Identifier &attribute, const Value &constantValue);

    // FIXME: This is here for initialization of the Filter variant for the grammar.
    // Do not know, if there is some other solution now.
    AttributeExpression() {};
};

bool operator==(const AttributeExpression &a, const AttributeExpression &b);
bool operator!=(const AttributeExpression &a, const AttributeExpression &b);
std::ostream& operator<<(std::ostream &stream, const AttributeExpression &a);

/** @short What kind of a special filter expression is it */
typedef enum {
    FILTER_SPECIAL_EMBEDDED_LAST_ONE /**< @short The last-inserted item */
} SpecialFilterType;

std::ostream& operator<<(std::ostream &stream, const SpecialFilterType f);

/** @short Special filter expression: match on magic properties */
struct SpecialExpression
{
    SpecialFilterType type;
    Identifier kind;

    SpecialExpression(const SpecialFilterType type, const Identifier &kind);

    // FIXME: default constructor for the CLI parser
    SpecialExpression() {}
};

bool operator==(const SpecialExpression &a, const SpecialExpression &b);
bool operator!=(const SpecialExpression &a, const SpecialExpression &b);
std::ostream& operator<<(std::ostream &stream, const SpecialExpression &s);

/** @short A generic expression */
typedef boost::variant<MetadataExpression, AttributeExpression, SpecialExpression> Expression;

bool operator!=(const Expression &a, const Expression &b);

std::string repr_Expression(const Expression &e);

struct OrFilter;
struct AndFilter;

/** @short Filter for limiting the result set of an operation */
typedef boost::variant<Expression, boost::recursive_wrapper<OrFilter>, boost::recursive_wrapper<AndFilter> > Filter;

std::string repr_Filter(const Filter &f);

/** @short Perform a logical disjunction of all expression included below */
struct OrFilter
{
    std::vector<Filter> operands;

    OrFilter(const std::vector<Filter> &operands);

    // FIXME: This is here for initialization of the Filter variant for the grammar.
    // Do not know, if there is some other solution now.
    OrFilter() {};
};

bool operator==(const OrFilter &a, const OrFilter &b);
bool operator!=(const OrFilter &a, const OrFilter &b);
std::ostream& operator<<(std::ostream &stream, const OrFilter &o);

/** @short Perform a logical conjunction of all expression included below */
struct AndFilter
{
    std::vector<Filter> operands;

    AndFilter(const std::vector<Filter> &operands);

    // FIXME: This is here for initialization of the Filter variant for the grammar.
    // Do not know, if there is some other solution now.
    AndFilter() {};
};

bool operator==(const AndFilter &a, const AndFilter &b);
bool operator!=(const AndFilter &a, const AndFilter &b);
std::ostream& operator<<(std::ostream &stream, const AndFilter &a);

}
}

#endif // DESKA_DB_FILTER_H
