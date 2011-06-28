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

/** @short __repr__ for Deska::Db::MetadataValue */
std::string repr_MetadataValue(const MetadataValue &v)
{
    std::ostringstream ss;
    ss << "MetadataValue<" << boost::apply_visitor(DeskaMetadataValueTypeName(), v) << ">(" << boost::apply_visitor(DeskaMetadataValueRepr(), v) << ")";
    return ss.str();
}

/** @short __str__ for Deska::Db::MetadataValue */
std::string str_MetadataValue(const MetadataValue &v)
{
    return boost::apply_visitor(DeskaMetadataValueToString(), v);
}

/** @short __str__ for the Deska::Db::ComparisonOperator */
std::string str_ComparisonOperator(const ComparisonOperator op)
{
    switch (op) {
    case FILTER_COLUMN_EQ:
        return std::string("==");
    case FILTER_COLUMN_NE:
        return std::string("!=");
    case FILTER_COLUMN_GT:
        return std::string(">");
    case FILTER_COLUMN_GE:
        return std::string(">=");
    case FILTER_COLUMN_LT:
        return std::string("<");
    case FILTER_COLUMN_LE:
        return std::string("<=");
    }
    throw std::domain_error("Value of Deska::Db::ExpressionKind is out of bounds");
}

/** @short __repr__ for Deska::Db::MetadataExpression */
std::string repr_MetadataExpression(const MetadataExpression &e)
{
    std::ostringstream ss;
    ss << "MetadataExpression(" << e.metadata << " " << str_ComparisonOperator(e.comparison) << " " << repr_MetadataValue(e.constantValue) << ")";
    return ss.str();
}

/** @short __repr__ for Deska::Db::AttributeExpression */
std::string repr_AttributeExpression(const AttributeExpression &e)
{
    std::ostringstream ss;
    ss << "AttributeExpression(" << e.kind << "." << e.attribute << " " << str_ComparisonOperator(e.comparison) << " " << repr_Value(e.constantValue) << ")";
    return ss.str();
}

/** @short Helper visitor for Deska::Db::Expression's __repr__ */
struct DeskaExpressionToString: public boost::static_visitor<std::string>
{
    result_type operator()(const MetadataExpression &v) const
    {
        std::ostringstream ss;
        ss << "Expression<MetadataExpression>(" << repr_MetadataExpression(v) << ")";
        return ss.str();
    }
    result_type operator()(const AttributeExpression &v) const
    {
        std::ostringstream ss;
        ss << "Expression<AttributeExpression>(" << repr_AttributeExpression(v) << ")";
        return ss.str();
    }
};

/** @short __repr__ for Deska::Db::Expression */
std::string repr_Expression(const Expression &e)
{
    return boost::apply_visitor(DeskaExpressionToString(), e);
}

template <class T>
class no_compare_indexing_suite :
  public vector_indexing_suite<T, false, no_compare_indexing_suite<T> >
{
  public:
    static bool contains(T &container, typename T::value_type const &key)
    {
        PyErr_SetString(PyExc_NotImplementedError, "containment checking not supported on this container");
        throw boost::python::error_already_set();
    }
};

typedef std::vector<Filter> vect_Filter;

std::string repr_Filter(const Filter &v);

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

/** @short __repr__ for a Deska::Db::AndFilter */
std::string repr_AndFilter(const AndFilter& v)
{
    std::ostringstream ss;
    ss << "AndFilter(" << repr_vect_Filter(v.operands) << ")";
    return ss.str();
}

/** @short __repr__ for a Deska::Db::OrFilter */
std::string repr_OrFilter(const OrFilter& v)
{
    std::ostringstream ss;
    ss << "OrFilter(" << repr_vect_Filter(v.operands) << ")";
    return ss.str();
}

/** @short Helper visitor for Deska::Db::Filter's __repr__ */
struct DeskaFilterToString: public boost::static_visitor<std::string>
{
    result_type operator()(const AndFilter &v) const
    {
        return repr_AndFilter(v);
    }
    result_type operator()(const OrFilter &v) const
    {
        return repr_OrFilter(v);
    }
    result_type operator()(const Expression &v) const
    {
        return repr_Expression(v);
    }
};

/** @short __repr__ for a Deska::Db::Filter */
std::string repr_Filter(const Filter &v)
{
    return boost::apply_visitor(DeskaFilterToString(), v);
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
            .value("COLUMN_LE", FILTER_COLUMN_LE);

    enum_<PendingChangeset::AttachStatus>("PendingChangesetAttachStatus")
            .value("DETACHED", PendingChangeset::ATTACH_DETACHED)
            .value("IN_PROGRESS", PendingChangeset::ATTACH_IN_PROGRESS);

    class_<MetadataValue>("MetadataValue", no_init)
            .def(self == other<MetadataValue>())
            .def("__repr__", repr_MetadataValue)
            .def("__str__", str_MetadataValue);

    def("DeskaMetadataValue_2_Py", DeskaMetadataValue_2_Py);
    def("Py_2_DeskaMetadataValue", Py_2_DeskaMetadataValue);

    class_<MetadataExpression>("MetadataExpression", init<ComparisonOperator, Identifier, MetadataValue>())
            .def_readonly("comparison", &MetadataExpression::comparison)
            .def_readonly("metadata", &MetadataExpression::metadata)
            .def_readonly("constantValue", &MetadataExpression::constantValue)
            .def("__repr__", repr_MetadataExpression);

    class_<AttributeExpression>("AttributeExpression", init<ComparisonOperator, Identifier, Identifier, Value>())
            .def_readonly("comparison", &AttributeExpression::comparison)
            .def_readonly("kind", &AttributeExpression::kind)
            .def_readonly("attribute", &AttributeExpression::attribute)
            .def_readonly("constantValue", &AttributeExpression::constantValue)
            .def("__repr__", repr_AttributeExpression);

    class_<Expression>("Expression", no_init)
            .def(init<const MetadataExpression&>())
            .def(init<const AttributeExpression&>())
            .def("__repr__", repr_Expression);

    class_<vect_Filter>("std_vector_Filter")
            .def(no_compare_indexing_suite<vect_Filter>())
            .def("__repr__", repr_vect_Filter);

    class_<OrFilter>("OrFilter", no_init)
            .def(init<const vect_Filter&>())
            .def("__repr__", repr_OrFilter);

    class_<AndFilter>("AndFilter", no_init)
            .def(init<const vect_Filter&>())
            .def("__repr__", repr_AndFilter);

    class_<Filter>("Filter")
            .def(init<const Expression&>())
            .def(init<const OrFilter&>())
            .def(init<const AndFilter&>())
            .def("__repr__", repr_Filter)
            ;
}
