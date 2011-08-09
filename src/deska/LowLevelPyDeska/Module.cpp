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
#include <boost/python/suite/indexing/map_indexing_suite.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include "deska/db/Connection.h"
#include "deska/LowLevelPyDeska/Filter.h"
#include "deska/LowLevelPyDeska/Value.h"

using namespace boost::python;
using namespace Deska::Db;

// from the PythonDateTimeConversions.cpp
void bind_datetime();

template<typename T>
std::string repr_vect(const std::vector<T> &v)
{
    std::ostringstream ss;
    ss << "[";
    BOOST_FOREACH(const T &item, v) {
        ss << item << ", ";
    }
    ss << "]";
    return ss.str();
}

std::string repr_ObjectRelation(const ObjectRelation &r)
{
    std::ostringstream ss;
    ss << r;
    return ss.str();
}

void exportObjectRelations()
{
    // required for kindRelations
    typedef std::vector<ObjectRelation> vect_ObjectRelation;
    class_<vect_ObjectRelation>("std_vector_Deska_Db_ObjectRelation")
            .def(vector_indexing_suite<vect_ObjectRelation>())
            .def("__repr__", repr_vect<ObjectRelation>);

    enum_<ObjectRelationKind>("ObjectRelationKind")
            .value("MERGE_WITH", RELATION_MERGE_WITH)
            .value("EMBED_INTO", RELATION_EMBED_INTO)
            .value("REFERS_TO", RELATION_REFERS_TO)
            .value("TEMPLATIZED", RELATION_TEMPLATIZED);

    class_<ObjectRelation>("ObjectRelation", no_init)
            .def_readonly("kind", &ObjectRelation::kind)
            .def_readonly("target", &ObjectRelation::target)
            .def("__repr__", repr_ObjectRelation)
            .def(self_ns::str(self))
            .def(self < other<ObjectRelation>());
}

void exportAttributeTypes()
{
    // required for kindAttributes
    typedef std::vector<KindAttributeDataType> vect_KindAttributeDataType;
    class_<vect_KindAttributeDataType>("std_vector_Deska_Db_KindAttributeDataType")
            .def(vector_indexing_suite<vect_KindAttributeDataType>())
            .def("__repr__", repr_vect<KindAttributeDataType>);
    enum_<Type>("AttributeType")
            .value("IDENTIFIER", TYPE_IDENTIFIER)
            .value("IDENTIFIER_SET", TYPE_IDENTIFIER_SET)
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
            .def(self_ns::str(self))
            .def(self_ns::repr(self))
            .def(self < other<KindAttributeDataType>());
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

template<class T1, class T2>
struct PairToTupleConverter {
    static PyObject* convert(const std::pair<T1, T2>& pair) {
        return incref(boost::python::make_tuple(pair.first, pair.second).ptr());
    }
};

/** @short Add Python definitions for various convoluted Deska map structures */
void exportNastyMaps()
{
    typedef std::map<Identifier, Value> map_Identifier_Value;
    class_<map_Identifier_Value>("std_map_Identifier_Value")
            .def(map_indexing_suite<map_Identifier_Value>());

    // this one is the inner type inside the multipleResolvedObjectDataWithOrigin return type
    typedef std::pair<Identifier, Value> pair_Identifier_Value;
    to_python_converter<pair_Identifier_Value, PairToTupleConverter<Identifier, Value> >();

    // ...the middle one...
    typedef std::map<Identifier, pair_Identifier_Value> map_Identifier_pair_Identifier_Value;
    class_<map_Identifier_pair_Identifier_Value>("std_map_Identifier_pair_Identifier_Value")
            .def(map_indexing_suite<map_Identifier_pair_Identifier_Value>());

    // ...and even the outer one :)
    typedef std::map<Identifier, map_Identifier_pair_Identifier_Value> map_Identifier_map_Identifier_pair_Identifier_Value;
    class_<map_Identifier_map_Identifier_pair_Identifier_Value>("std_map_Identifier_std_map_Identifier_pair_Identifier_Value")
            .def(map_indexing_suite<map_Identifier_map_Identifier_pair_Identifier_Value>());

    // also define a proper type for the multipleObjectData
    typedef std::map<Identifier, std::map<Identifier, Value> > map_Identifier_map_Identifier_Value;
    class_<map_Identifier_map_Identifier_Value>("std_map_Identifier_map_Identifier_Value")
            .def(map_indexing_suite<map_Identifier_map_Identifier_Value>());
}

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(Connection_kindInstances_overloads, kindInstances, 1, 3);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(Connection_objectData_overloads, objectData, 2, 3);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(Connection_resolvedObjectData_overloads, resolvedObjectData, 2, 3);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(Connection_multipleObjectData, multipleObjectData, 2, 3);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(Connection_multipleResolvedObjectData, multipleResolvedObjectData, 2, 3);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(Connection_resolvedObjectDataWithOrigin, resolvedObjectDataWithOrigin, 2, 3);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(Connection_multipleResolvedObjectDataWithOrigin, multipleResolvedObjectDataWithOrigin, 2, 3);

BOOST_PYTHON_MODULE(libLowLevelPyDeska)
{
    bind_datetime();
    exportObjectRelations();
    exportAttributeTypes();
    exportRevisions();
    exportDeskaValue();
    exportDeskaFilter();
    exportNastyMaps();

    // required for kindNames
    typedef std::vector<std::string> vect_string;
    class_<vect_string>("std_vector_string")
            .def(vector_indexing_suite<vect_string>())
            .def("__repr__", repr_vect<std::string>);

    // DBAPI connection implementation
    class_<Connection, boost::noncopyable>("Connection")
            .def("kindNames", &Connection::kindNames)
            .def("kindRelations", &Connection::kindRelations)
            .def("kindAttributes", &Connection::kindAttributes)
            .def("kindInstances", &Connection::kindInstances, Connection_kindInstances_overloads())
            .def("objectData", &Connection::objectData, Connection_objectData_overloads())
            .def("resolvedObjectData", &Connection::resolvedObjectData, Connection_resolvedObjectData_overloads())
            .def("multipleObjectData", &Connection::multipleObjectData, Connection_multipleObjectData())
            .def("multipleResolvedObjectData", &Connection::multipleResolvedObjectData, Connection_multipleResolvedObjectData())
            .def("resolvedObjectDataWithOrigin", &Connection::resolvedObjectDataWithOrigin, Connection_resolvedObjectDataWithOrigin())
            .def("multipleResolvedObjectDataWithOrigin", &Connection::multipleResolvedObjectDataWithOrigin, Connection_multipleResolvedObjectDataWithOrigin())
            .def("resumeChangeset", &Connection::resumeChangeset)
            .def("detachFromCurrentChangeset", &Connection::detachFromCurrentChangeset)
            .def("freezeView", &Connection::freezeView)
            .def("unFreezeView", &Connection::unFreezeView);
}
