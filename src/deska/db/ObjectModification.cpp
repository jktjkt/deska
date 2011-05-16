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

#include <iostream>
#include "ObjectModification.h"

namespace Deska {
namespace Db {

CreateObjectModification::CreateObjectModification(const Identifier &kindName_, const Identifier &objectName_):
    kindName(kindName_), objectName(objectName_)
{
}

bool operator==(const CreateObjectModification &a, const CreateObjectModification &b)
{
    return a.kindName == b.kindName && a.objectName == b.objectName;
}

std::ostream& operator<<(std::ostream &stream, const CreateObjectModification &mod)
{
    return stream << "CreateObjectModification(" << mod.kindName << ", " << mod.objectName << ")";
}

DeleteObjectModification::DeleteObjectModification(const Identifier &kindName_, const Identifier &objectName_):
    kindName(kindName_), objectName(objectName_)
{
}

bool operator==(const DeleteObjectModification &a, const DeleteObjectModification &b)
{
    return a.kindName == b.kindName && a.objectName == b.objectName;
}

std::ostream& operator<<(std::ostream &stream, const DeleteObjectModification &mod)
{
    return stream << "DeleteObjectModification(" << mod.kindName << ", " << mod.objectName << ")";
}

RenameObjectModification::RenameObjectModification(const Identifier &kindName_, const Identifier &oldObjectName_,
                                                   const Identifier &newObjectName_):
    kindName(kindName_), oldObjectName(oldObjectName_), newObjectName(newObjectName_)
{
}

bool operator==(const RenameObjectModification &a, const RenameObjectModification &b)
{
    return a.kindName == b.kindName && a.oldObjectName == b.oldObjectName && a.newObjectName == b.newObjectName;
}

std::ostream& operator<<(std::ostream &stream, const RenameObjectModification &mod)
{
    return stream << "RenameObjectModification(" << mod.kindName << ", " << mod.oldObjectName << ", " << mod.newObjectName << ")";
}

SetAttributeModification::SetAttributeModification(const Identifier &kindName_, const Identifier &objectName_,
    const Identifier &attributeName_, const Value &attributeData_, const Value &oldAttributeData_):
    kindName(kindName_), objectName(objectName_), attributeName(attributeName_), attributeData(attributeData_),
    oldAttributeData(oldAttributeData_)
{
}

bool operator==(const SetAttributeModification &a, const SetAttributeModification &b)
{
    return a.kindName == b.kindName && a.objectName == b.objectName && a.attributeName == b.attributeName &&
            a.attributeData == b.attributeData && a.oldAttributeData == b.oldAttributeData;
}

std::ostream& operator<<(std::ostream &stream, const SetAttributeModification &mod)
{
    return stream << "SetAttributeModification(" << mod.kindName << ", " << mod.objectName << ", " << mod.attributeName <<
        ", "  << mod.attributeData << ", "  << mod.oldAttributeData << ")";
}

bool operator!=(const ObjectModification& a, const ObjectModification& b)
{
    // boost::variant provides operator==(), but not operator!=()
    return !(a==b);
}

}
}
