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

#ifndef DESKA_CLI_CLICOMMANDS_REBASE_H
#define DESKA_CLI_CLICOMMANDS_REBASE_H

#include <string>
#include <vector>

#include "CliCommands.h"
#include "deska/db/ObjectModification.h"

namespace Deska {
namespace Cli {


class UserInterface;


/** @short Cli command.
*
*   Rebases current changeset.
*
*   @see Command
*/
class Rebase: public Command
{
public:
    /** @short Constructor sets command name and completion pattern.
    *
    *   @param userInterface Pointer to the UserInterface
    */
    Rebase(UserInterface *userInterface);

    virtual ~Rebase();

    /** @short Rebases current changeset.
    *
    *   @param params Unused here.
    */
    virtual bool operator()(const std::string &params);

private:
    /** @short Function for sorting object modifications.
    *
    *   Sorting at first by modification type, then by kind, by name, and lastly by attribute name.
    *
    *   @param a First object modification
    *   @param b Second object modification
    *   @return True if b is greater than a, else false
    */
    bool objectModificationResultLess(const Db::ObjectModificationResult &a, const Db::ObjectModificationResult &b);
};

/** @short Cli command.
*
*   Command for showing differences between revisions.
*
*   @see Command
*/
class Diff: public Command
{
public:
    /** @short Constructor sets command name and completion pattern.
    *
    *   @param userInterface Pointer to the UserInterface
    */
    Diff(UserInterface *userInterface);

    virtual ~Diff();

    /** @short Function for showing diffs between revisions.
    *
    *   @param params Unused here.
    */
    virtual bool operator()(const std::string &params);

private:
    /** @short Function for sorting object modifications.
    *
    *   Sorting at first by modification type, then by kind, by name, and lastly by attribute name.
    *
    *   @param a First object modification
    *   @param b Second object modification
    *   @return True if b is greater than a, else false
    */
    bool objectModificationResultLess(const Db::ObjectModificationResult &a, const Db::ObjectModificationResult &b);
};



}
}

#endif // DESKA_CLI_CLICOMMANDS_REBASE_H
