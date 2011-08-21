/*
* Copyright (C) 2011 Jan Kundrát <kundratj@fzu.cz>
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

#ifndef DESKA_CLI_CHILDPROCESS_H
#define DESKA_CLI_CHILDPROCESS_H

#include "boost/process.hpp"

namespace Deska
{
namespace Cli
{


class Editor
{
public:
    Editor(const std::string &fileName);
    ~Editor();

private:
    /** @short Identification of the launched child process

    We've got to use boost::optional here because the boost::process::child has no default constructor,
    see http://lists.boost.org/boost-users/2011/03/67265.php for details
    */
    boost::optional<boost::process::child> childProcess;
};



class Pager
{
public:
    Pager();
    ~Pager();

    /** @short Obtains a stream for writing */
    std::ostream *writeStream();

private:
    /** @short Identification of the launched child process

    We've got to use boost::optional here because the boost::process::child has no default constructor,
    see http://lists.boost.org/boost-users/2011/03/67265.php for details
    */
    boost::optional<boost::process::child> childProcess;
};

}
}

#endif  // DESKA_CLI_CHILDPROCESS_H
