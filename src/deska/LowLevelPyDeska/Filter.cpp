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

#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include "deska/LowLevelPyDeska/Filter.h"
#include "deska/LowLevelPyDeska/Value.h"
#include "3rd-party/scitbx/set_wrapper.h"

using namespace boost::python;
using namespace Deska::Db;

/** @short Variant visitor for converting a Deska::Db::MetadataValue to a Python object */
struct DeskaMetadataValueToPythonObjectVisitor: public boost::static_visitor<api::object>
{
    result_type operator()(const Value &v) const
    {
        return DeskaDbValue_2_Py(v);
    }

    template <typename T>
    result_type operator()(const T &v) const
    {
        return result_type(v);
    }
};


/** @short Convert a Deska::Db::MetadataValue to a python object */
api::object DeskaMetadataValue_2_Py(const MetadataValue &v)
{
    return boost::apply_visitor(DeskaMetadataValueToPythonObjectVisitor(), v);
}

/** @short Convert a python object into the Deska::Db::Value */
MetadataValue Py_2_DeskaMetadataValue(const api::object &o)
{
    try {
        Value val = Py_2_DeskaDbValue(o);
        return val;
    } catch (std::runtime_error &e) {
        // failed conversion, do nothing now
    }
    // Deska::Db::RevisionId
    extract<RevisionId> get_revision(o);
    if (get_revision.check())
        return MetadataValue(get_revision());

    throw std::runtime_error("Unsupported type of a python object");
}

typedef std::vector<Filter> vect_Filter;

/** @short __repr__ for a std::vector<Deska::Db::Filter> */
std::string repr_vect_Filter(const vect_Filter &v)
{
    std::ostringstream ss;
    ss << "[";
    BOOST_FOREACH(const Filter& f, v) {
        ss << repr_Filter(f) << ", ";
    }
    ss << "]";
    return ss.str();
}

/** @short __repr__ for the boost::optional<Deska::Db::Filter> */
std::string repr_optionalFilter(const boost::optional<Filter> &v)
{
    return v ? repr_Filter(*v) : std::string("None");
}

/** @short __repr__ for the boost::optional<Deska::Db::RevisionId> */
std::string repr_optionalRevisionId(const boost::optional<RevisionId> &v)
{
    if (v) {
        std::ostringstream ss;
        ss << *v;
        return ss.str();
    } else {
        return "None";
    }
}

void exportDeskaFilter()
{
    // filters
    enum_<ComparisonOperator>("ComparisonOperator")
            .value("COLUMN_EQ", FILTER_COLUMN_EQ)
            .value("COLUMN_NE", FILTER_COLUMN_NE)
            .value("COLUMN_GT", FILTER_COLUMN_GT)
            .value("COLUMN_GE", FILTER_COLUMN_GE)
            .value("COLUMN_LT", FILTER_COLUMN_LT)
            .value("COLUMN_LE", FILTER_COLUMN_LE)
            .value("COLUMN_CONTAINS", FILTER_COLUMN_CONTAINS)
            .value("COLUMN_NOT_CONTAINS", FILTER_COLUMN_NOT_CONTAINS);

    enum_<SpecialFilterType>("SpecialFilterType")
            .value("SPECIAL_EMBEDDED_LAST_ONE", FILTER_SPECIAL_EMBEDDED_LAST_ONE);

    enum_<PendingChangeset::AttachStatus>("PendingChangesetAttachStatus")
            .value("DETACHED", PendingChangeset::ATTACH_DETACHED)
            .value("IN_PROGRESS", PendingChangeset::ATTACH_IN_PROGRESS);

    class_<MetadataValue>("MetadataValue", no_init)
            .def(self == other<MetadataValue>())
            .def("__repr__", Deska::Db::repr_MetadataValue)
            .def("__str__", Deska::Db::str_MetadataValue);

    def("DeskaMetadataValue_2_Py", DeskaMetadataValue_2_Py);
    def("Py_2_DeskaMetadataValue", Py_2_DeskaMetadataValue);

    class_<MetadataExpression>("MetadataExpression", init<ComparisonOperator, Identifier, MetadataValue>())
            .def_readonly("comparison", &MetadataExpression::comparison)
            .def_readonly("metadata", &MetadataExpression::metadata)
            .def_readonly("constantValue", &MetadataExpression::constantValue)
            .def(self_ns::str(self))
            .def(self_ns::repr(self));

    class_<AttributeExpression>("AttributeExpression", init<ComparisonOperator, Identifier, Identifier, Value>())
            .def_readonly("comparison", &AttributeExpression::comparison)
            .def_readonly("kind", &AttributeExpression::kind)
            .def_readonly("attribute", &AttributeExpression::attribute)
            .def_readonly("constantValue", &AttributeExpression::constantValue)
            .def(self_ns::str(self))
            .def(self_ns::repr(self));

    class_<SpecialExpression>("SpecialExpression", init<SpecialFilterType, Identifier>())
            .def_readonly("type", &SpecialExpression::type)
            .def_readonly("kind", &SpecialExpression::kind)
            .def(self_ns::str(self))
            .def(self_ns::repr(self));

    class_<Expression>("Expression", no_init)
            .def(init<const MetadataExpression&>())
            .def(init<const AttributeExpression&>())
            .def(init<const SpecialExpression&>())
            .def(self_ns::str(self))
            .def("__repr__", Deska::Db::repr_Expression);

    class_<vect_Filter>("std_vector_Filter")
            .def(vector_indexing_suite<vect_Filter>())
            .def("__repr__", repr_vect_Filter);

    class_<OrFilter>("OrFilter", no_init)
            .def(init<const vect_Filter&>())
            .def(self_ns::str(self))
            .def(self_ns::repr(self))
            .def_readonly("operands", &OrFilter::operands);

    class_<AndFilter>("AndFilter", no_init)
            .def(init<const vect_Filter&>())
            .def(self_ns::str(self))
            .def(self_ns::repr(self))
            .def_readonly("operands", &AndFilter::operands);

    class_<Filter>("Filter")
            .def(init<const Expression&>())
            .def(init<const OrFilter&>())
            .def(init<const AndFilter&>())
            .def(self_ns::str(self))
            .def(self_ns::repr(self))
            .def("__repr__", Deska::Db::repr_Filter);
            ;

    class_<boost::optional<Filter> >("OptionalFilter")
            .def(init<const Expression&>())
            .def(init<const AndFilter&>())
            .def(init<const OrFilter&>())
            .def("__repr__", repr_optionalFilter);

    class_<boost::optional<RevisionId> >("OptionalRevisionId")
            .def(init<const RevisionId>())
            .def("__repr__", repr_optionalRevisionId);

    scitbx::stl::boost_python::set_wrapper<std::string>::wrap("std_set_std_string");
}
