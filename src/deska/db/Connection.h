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

#ifndef DESKA_DB_CONNECTION_H
#define DESKA_DB_CONNECTION_H

#include "CachingJsonApi.h"

namespace Deska {
namespace Db {

class ProcessIO;

// FIXME: change this to derive directly from the abstract Api to prevent cluttering up the namespace with implementation details
class Connection: public CachingJsonApi
{
public:
    Connection();
    virtual ~Connection();
private:
    ProcessIO *io;
};

}
}

#endif // DESKA_DB_CONNECTION_H