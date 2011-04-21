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

#ifndef DESKA_DB_OBJECTMODIFICATION_H
#define DESKA_DB_OBJECTMODIFICATION_H

#include <iosfwd>
#include "deska/db/Objects.h"

namespace Deska {
namespace Db {

struct CreateObjectModification {
    Identifier kindName;
    Identifier objectName;

    CreateObjectModification(const Identifier &kindName, const Identifier &objectName);
};

std::ostream& operator<<(std::ostream &stream, const CreateObjectModification &mod);

struct DeleteObjectModification {
    Identifier kindName;
    Identifier objectName;

    DeleteObjectModification(const Identifier &kindName, const Identifier &objectName);
};

std::ostream& operator<<(std::ostream &stream, const DeleteObjectModification &mod);

struct RenameObjectModification {
    Identifier kindName;
    Identifier objectName;
    Identifier oldObjectName;

    RenameObjectModification(const Identifier &kindName, const Identifier &objectName, const Identifier &oldObjectName);
};

std::ostream& operator<<(std::ostream &stream, const RenameObjectModification &mod);

struct RemoveAttributeModification {
    Identifier kindName;
    Identifier objectName;
    Identifier attributeName;

    RemoveAttributeModification(const Identifier &kindName, const Identifier &objectName, const Identifier &attributeName);
};

std::ostream& operator<<(std::ostream &stream, const RemoveAttributeModification &mod);

struct SetAttributeModification {
    Identifier kindName;
    Identifier objectName;
    Identifier attributeName;
    Value attributeData;
    Value oldAttributeData;

    SetAttributeModification(const Identifier &kindName, const Identifier &objectName, const Identifier &attributeName,
                             const Value &attributeData, const Value &oldAttributeData);
};

std::ostream& operator<<(std::ostream &stream, const SetAttributeModification &mod);

typedef boost::variant<CreateObjectModification, DeleteObjectModification, RenameObjectModification, RemoveAttributeModification,
    SetAttributeModification> ObjectModification;

}
}

#endif // DESKA_DB_OBJECTMODIFICATION_H
