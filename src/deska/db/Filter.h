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
    FILTER_COLUMN_LE
} ComparisonOperator;

/** @short Anything against which we can compare */
typedef boost::variant<Value,RevisionId,TemporaryChangesetId,PendingChangeset::AttachStatus> MetadataValue;

/** @short Compare metadata against a constant using  given comparison operator */
struct MetadataExpression
{
    ComparisonOperator comparison;
    Identifier metadata;
    MetadataValue constantValue;

    MetadataExpression(const ComparisonOperator comparison, const Identifier &metadata, const MetadataValue &constantValue);
};

/** @short Compare attribute value against a constant using given comparison operator */
struct AttributeExpression
{
    ComparisonOperator comparison;
    Identifier kind;
    Identifier attribute;
    Value constantValue;

    AttributeExpression(const ComparisonOperator comparison, const Identifier &kind, const Identifier &attribute, const Value &constantValue);
};

/** @short A generic expression */
typedef boost::variant<MetadataExpression, AttributeExpression> Expression;

struct OrFilter;
struct AndFilter;

/** @short Filter for limiting the result set of an operation */
typedef boost::variant<Expression, boost::recursive_wrapper<OrFilter>, boost::recursive_wrapper<AndFilter> > Filter;

/** @short Perform a logical disjunction of all expression included below */
struct OrFilter
{
    std::vector<Filter> operands;

    OrFilter(const std::vector<Filter> operands);
};

/** @short Perform a logical conjunction of all expression included below */
struct AndFilter
{
    std::vector<Filter> operands;

    AndFilter(const std::vector<Filter> operands);
};

}
}

#endif // DESKA_DB_FILTER_H
