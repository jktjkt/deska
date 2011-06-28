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

#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include "deska/db/Connection.h"
#include "deska/LowLevelPyDeska/Value.h"

using namespace boost::python;
using namespace Deska::Db;

// from the PythonDateTimeConversions.cpp
void bind_datetime();

void exportObjectRelations()
{
    // required for kindRelations
    typedef std::vector<ObjectRelation> vect_ObjectRelation;
    class_<vect_ObjectRelation>("std_vector_Deska_Db_ObjectRelation")
            .def(vector_indexing_suite<vect_ObjectRelation>());

    enum_<ObjectRelationKind>("ObjectRelationKind")
            .value("MERGE_WITH", RELATION_MERGE_WITH)
            .value("EMBED_INTO", RELATION_EMBED_INTO)
            .value("REFERS_TO", RELATION_REFERS_TO)
            .value("IS_TEMPLATE", RELATION_IS_TEMPLATE)
            .value("TEMPLATIZED", RELATION_TEMPLATIZED);

    class_<ObjectRelation>("ObjectRelation", no_init)
            .def_readonly("kind", &ObjectRelation::kind)
            .def_readonly("target", &ObjectRelation::target)
            .def(self_ns::str(self));
}

void exportAttributeTypes()
{
    // required for kindAttributes
    typedef std::vector<KindAttributeDataType> vect_KindAttributeDataType;
    class_<vect_KindAttributeDataType>("std_vector_Deska_Db_KindAttributeDataType")
            .def(vector_indexing_suite<vect_KindAttributeDataType>());
    enum_<Type>("AttributeType")
            .value("IDENTIFIER", TYPE_IDENTIFIER)
            .value("STRING", TYPE_STRING)
            .value("INT", TYPE_INT)
            .value("DOUBLE", TYPE_DOUBLE)
            .value("IPV4_ADDRESS", TYPE_IPV4_ADDRESS)
            .value("IPV6_ADDRESS", TYPE_IPV6_ADDRESS)
            .value("MAC_ADDRESS", TYPE_MAC_ADDRESS)
            .value("DATE", TYPE_DATE)
            .value("TIMESTAMP", TYPE_TIMESTAMP);

    class_<KindAttributeDataType>("KindAttributeDataType", no_init)
            .def_readonly("name", &KindAttributeDataType::name)
            .def_readonly("type", &KindAttributeDataType::type)
            .def(self_ns::str(self));
}

void exportRevisions()
{
    // revisions and changesets (required for filters)
    class_<RevisionId>("RevisionId", init<const unsigned int>())
            .def(self == other<RevisionId>())
            .def(self_ns::str(self));
    class_<TemporaryChangesetId>("TemporaryChangesetId", init<const unsigned int>())
            .def(self == other<TemporaryChangesetId>())
            .def(self_ns::str(self));
}

void exportFilters()
{
    // filters
    enum_<ComparisonOperator>("ComparisonOperator")
            .value("COLUMN_EQ", FILTER_COLUMN_EQ)
            .value("COLUMN_NE", FILTER_COLUMN_NE)
            .value("COLUMN_GT", FILTER_COLUMN_GT)
            .value("COLUMN_GE", FILTER_COLUMN_GE)
            .value("COLUMN_LT", FILTER_COLUMN_LT)
            .value("COLUMN_LE", FILTER_COLUMN_LE);
    //class_<MetadataValue>
}

BOOST_PYTHON_MODULE(libLowLevelPyDeska)
{
    bind_datetime();
    exportObjectRelations();
    exportAttributeTypes();
    exportRevisions();
    exportDeskaValue();
    exportFilters();

    // required for kindNames
    typedef std::vector<std::string> vect_string;
    class_<vect_string>("std_vector_string")
            .def(vector_indexing_suite<vect_string>());

    // DBAPI connection implementation
    class_<Connection, boost::noncopyable>("Connection")
            .def("kindNames", &Connection::kindNames)
            .def("kindRelations", &Connection::kindRelations)
            .def("kindAttributes", &Connection::kindAttributes);
}
