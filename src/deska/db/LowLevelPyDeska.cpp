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

BOOST_PYTHON_MODULE(libLowLevelPyDeska)
{
    using namespace boost::python;
    using namespace Deska::Db;

    typedef std::vector<std::string> vect_string;
    class_<vect_string>("std_vector_string")
            .def(vector_indexing_suite<vect_string>());

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

    class_<Connection, boost::noncopyable>("Connection")
            .def("kindNames", &Connection::kindNames)
            .def("kindRelations", &Connection::kindRelations);
}
