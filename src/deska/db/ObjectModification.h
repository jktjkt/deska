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

bool operator==(const CreateObjectModification &a, const CreateObjectModification &b);
std::ostream& operator<<(std::ostream &stream, const CreateObjectModification &mod);

struct DeleteObjectModification {
    Identifier kindName;
    Identifier objectName;

    DeleteObjectModification(const Identifier &kindName, const Identifier &objectName);
};

bool operator==(const DeleteObjectModification &a, const DeleteObjectModification &b);
std::ostream& operator<<(std::ostream &stream, const DeleteObjectModification &mod);

struct RenameObjectModification {
    Identifier kindName;
    Identifier oldObjectName;
    Identifier newObjectName;

    RenameObjectModification(const Identifier &kindName, const Identifier &oldObjectName, const Identifier &newObjectName);
};

bool operator==(const RenameObjectModification &a, const RenameObjectModification &b);
std::ostream& operator<<(std::ostream &stream, const RenameObjectModification &mod);

struct SetAttributeModification {
    Identifier kindName;
    Identifier objectName;
    Identifier attributeName;
    Value attributeData;
    Value oldAttributeData;

    SetAttributeModification(const Identifier &kindName, const Identifier &objectName, const Identifier &attributeName,
                             const Value &attributeData, const Value &oldAttributeData);
    SetAttributeModification(const Identifier &kindName, const Identifier &objectName, const Identifier &attributeName,
                             const Value &attributeData);
};

bool operator==(const SetAttributeModification &a, const SetAttributeModification &b);
std::ostream& operator<<(std::ostream &stream, const SetAttributeModification &mod);

struct SetAttributeInsertModification {
    Identifier kindName;
    Identifier objectName;
    Identifier attributeName;
    Identifier attributeData;

    SetAttributeInsertModification(const Identifier &kindName, const Identifier &objectName, const Identifier &attributeName, const Identifier &attributeData);
};
bool operator==(const SetAttributeInsertModification &a, const SetAttributeInsertModification &b);
bool operator<<(std::ostream &stream, const SetAttributeInsertModification &mod);

struct SetAttributeRemoveModification {
    Identifier kindName;
    Identifier objectName;
    Identifier attributeName;
    Identifier attributeData;

    SetAttributeRemoveModification(const Identifier &kindName, const Identifier &objectName, const Identifier &attributeName, const Identifier &attributeData);
};
bool operator==(const SetAttributeRemoveModification &a, const SetAttributeRemoveModification &b);
bool operator<<(std::ostream &stream, const SetAttributeRemoveModification &mod);


/** @short Variant of all possible object modifications which can be encountered in the context of a diff */
typedef boost::variant<CreateObjectModification, DeleteObjectModification, RenameObjectModification,
    SetAttributeModification> ObjectModificationResult;

/** @short Variant of all possible object modifications which are usable as a part of a batched command */
typedef boost::variant<CreateObjectModification, DeleteObjectModification, RenameObjectModification,
SetAttributeModification, SetAttributeInsertModification, SetAttributeRemoveModification> ObjectModificationCommand;

/** @short operator!= is not provided by boost::variant */
bool operator!=(const ObjectModificationResult& a, const ObjectModificationResult& b);
bool operator!=(const ObjectModificationCommand& a, const ObjectModificationCommand& b);

}
}

#endif // DESKA_DB_OBJECTMODIFICATION_H
