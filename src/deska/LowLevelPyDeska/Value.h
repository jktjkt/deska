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

#ifndef DESKA_LOWLEVELPYDESKA_H
#define DESKA_LOWLEVELPYDESKA_H

#include <boost/python.hpp>
#include "deska/db/Objects.h"

/** @short Convert a Deska::Db::Value into Deska::Db::NonOptionalValue or throw an exception */
Deska::Db::NonOptionalValue deoptionalify(const Deska::Db::Value &v);

/** @short Convert a Deska::Db::Value to a python object */
boost::python::api::object pythonify(const Deska::Db::Value &v);

/** @short Convert a python object into the Deska::Db::Value */
Deska::Db::Value valueify(const boost::python::api::object &o);

void exportDeskaValue();

#endif // DESKA_LOWLEVELPYDESKA_H
