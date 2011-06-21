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

MetadataExpression::MetadataExpression(const ComparisonOperator comparison_, const Identifier &metadata_, const MetadataValue &constantValue_):
    comparison(comparison_), metadata(metadata_), constantValue(constantValue_)
{
}

AttributeExpression::AttributeExpression(const ComparisonOperator comparison_, const Identifier &kind_, const Identifier &attribute_,
                                         const Value &constantValue_):
    comparison(comparison_), kind(kind_), attribute(attribute_), constantValue(constantValue_)
{
}

OrFilter::OrFilter(const std::vector<Filter> operands_):
    operands(operands_)
{
}

AndFilter::AndFilter(const std::vector<Filter> operands_):
    operands(operands_)
{
}

}
}
