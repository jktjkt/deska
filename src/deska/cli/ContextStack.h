/*
* Copyright (C) 2011 Tomáš Hubík <hubik.tomas@gmail.com>
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


#ifndef DESKA_CONTEXTSTACK_H
#define DESKA_CONTEXTSTACK_H

#include <string>
#include <vector>
#include <boost/variant.hpp>
#include "deska/db/Objects.h"
#include "deska/db/Filter.h"


namespace Deska {
namespace Cli {


/** @short Typedef for one particular context stack item. */
// FIXME: Allow Filter here
//typedef boost::variant<Db::ObjectDefinition, Db::Filter> ContextStackItem;
typedef Db::ObjectDefinition ContextStackItem;

/** @short Typedef for context stack. */
typedef std::vector<ContextStackItem> ContextStack;

/** @short Function for converting context stack into name path to the object on the top.
*
*   This function is for obtaining full name from the context stack, so we can pass it to the DB.
*   
*   Example: For context stack [host hpv2, interface eth0] will the result of the function be hpv2->eth0
*
*   @param contextStack Context stack, where the top object is the one for which we want to get the path.
*   @return Identifier of a object composed from single identifiers from the context stack
*/
Db::Identifier contextStackToPath(const ContextStack &contextStack);

/** @short Function for converting context stack into string representation.
*
*   @param contextStack Context stack to convert
*   @return String representation of the context stack composed from single object definitions
*/
std::string contextStackToString(const ContextStack &contextStack);

/** @short Function for converting object path into vector of identifiers.
*
*   @param contextStack Context stack to convert
*   @return Vector of identifiers extracted from the path
*/
std::vector<Db::Identifier> pathToVector(const std::string &path);

}
}

#endif // DESKA_CONTEXTSTACK_H