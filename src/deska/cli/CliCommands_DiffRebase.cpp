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

#include <fstream>
#include <cstdlib>

#include "CliCommands_DiffRebase.h"
#include "UserInterface.h"
#include "UserInterfaceIOBase.h"
#include "DbInteraction.h"
#include "Exceptions.h"
#include "Parser.h"
#include "deska/db/JsonApi.h"

//#define REBASE_DEBUG

namespace Deska {
namespace Cli {



/** @short Visitor for comparing two modification of the same type */
struct ModificationComparatorLesss: public boost::static_visitor<bool>
{
    //@{
    /** @short Function for comparing two modifications of the same type
    *
    *   @param a Instance of modifications from Db::ObjectModification variant.
    *   @param b Instance of modifications from Db::ObjectModification variant.
    *   @return True if the first modification is "less" than the second. Comparing kinds and object names.
    */
    bool operator()(const Db::CreateObjectModification &a, const Db::CreateObjectModification &b) const;
    bool operator()(const Db::DeleteObjectModification &a, const Db::DeleteObjectModification &b) const;
    bool operator()(const Db::RenameObjectModification &a, const Db::RenameObjectModification &b) const;
    /** When comparing SetAttribute modifications, that are on the same attribute of the same kind, but the
    *   values differ, first modification is always "less". This ensures stable sorting.
    */
    bool operator()(const Db::SetAttributeModification &a, const Db::SetAttributeModification &b) const;
    //@}

    /** @short Function for enabling this comparator work. This function should not be called.
    */
    template <typename MA, typename MB>
    bool operator()(const MA &a, const MB &b) const;
};



bool ModificationComparatorLesss::operator()(const Db::CreateObjectModification &a,
                                             const Db::CreateObjectModification &b) const
{
    if (a.kindName < b.kindName) {
        return true;
    } else if (a.kindName > b.kindName) {
        return false;
    } else {
        return (a.objectName < b.objectName);
    }
}



bool ModificationComparatorLesss::operator()(const Db::DeleteObjectModification &a,
                                             const Db::DeleteObjectModification &b) const
{
    if (a.kindName < b.kindName) {
        return true;
    } else if (a.kindName > b.kindName) {
        return false;
    } else {
        return (a.objectName < b.objectName);
    }
}



bool ModificationComparatorLesss::operator()(const Db::RenameObjectModification &a,
                                             const Db::RenameObjectModification &b) const
{
    if (a.kindName < b.kindName) {
        return true;
    } else if (a.kindName > b.kindName) {
        return false;
    } else if (a.oldObjectName < b.oldObjectName) {
        return true;
    } else if (a.oldObjectName > b.oldObjectName) {
        return false;
    } else {
        return (a.newObjectName < b.newObjectName);
    }
}



bool ModificationComparatorLesss::operator()(const Db::SetAttributeModification &a,
                                             const Db::SetAttributeModification &b) const
{
    if (a.kindName < b.kindName) {
        return true;
    } else if (a.kindName > b.kindName) {
        return false;
    } else if (a.objectName < b.objectName) {
        return true;
    } else if (a.objectName > b.objectName) {
        return false;
    } else {
        return (a.attributeName < b.attributeName);
    }
}



template <typename MA, typename MB>
bool ModificationComparatorLesss::operator()(const MA &a, const MB &b) const
{
    throw std::invalid_argument("Deska::Cli::ModificationComparatorLesss::operator(): Comparator called to two different types of ObjectModificationResult.");
    return false;
}



/** @short Visitor for converting ObjectModification made by un in the changeset to user readable format
*          for purposes of rebase.
*/
struct OurModificationConverter: public boost::static_visitor<std::string>
{
    /** @short Constructor assignes vector of our deletions for proper output
    *
    *   @param ourDeletions Vector of deleted objects in out changeset
    */
    OurModificationConverter(const std::vector<ObjectDefinition> &ourDeletions);

    //@{
    /** @short Function for converting single object modification.
    *
    *   @param  modification Instance of modifications from Db::ObjectModification variant.
    *   @return string representation of each modification saying, that we made it
    */
    std::string operator()(const Db::CreateObjectModification &modification) const;
    std::string operator()(const Db::DeleteObjectModification &modification) const;
    std::string operator()(const Db::RenameObjectModification &modification) const;
    std::string operator()(const Db::SetAttributeModification &modification) const;
    //@}

private:
    /** Vector of deleted objects in out changeset */
    std::vector<ObjectDefinition> m_ourDeletions;
};



/** @short Visitor for converting ObjectModification made by un in the changeset to user readable format
*          for purposes of rebase.
*/
struct OurModificationConverter2: public boost::static_visitor<std::string>
{
     /** @short Constructor assignes vector of our deletions for proper output
    *
    *   @param ourDeletions Vector of deleted objects in out changeset
    */
    OurModificationConverter2(const std::vector<ObjectDefinition> &ourDeletions);

    //@{
    /** @short Function for converting single object modification.
    *
    *   @param  modification Instance of modifications from Db::ObjectModification variant.
    *   @return string representation of each modification saying, that we made it
    */
    template <typename LM>
    std::string operator()(const Db::CreateObjectModification &modification, const LM &lModification) const;
    template <typename LM>
    std::string operator()(const Db::DeleteObjectModification &modification, const LM &lModification) const;
    template <typename LM>
    std::string operator()(const Db::RenameObjectModification &modification, const LM &lModification) const;
    template <typename LM>
    std::string operator()(const Db::SetAttributeModification &modification, const LM &lModification) const;
    std::string operator()(const Db::SetAttributeModification &modification,
                           const Db::SetAttributeModification &lModification) const;
    //@}
    
private:
    /** Vector of deleted objects in out changeset */
    std::vector<ObjectDefinition> m_ourDeletions;
};




/** @short Visitor for converting ObjectModification made by someone else in newer revision to user readable format
*          for purposes of rebase.
*/
struct ExternModificationConverter: public boost::static_visitor<std::string>
{
     /** @short Constructor assignes vector of our deletions for proper output
    *
    *   @param ourDeletions Vector of deleted objects in out changeset
    */
    ExternModificationConverter(const std::vector<ObjectDefinition> &ourDeletions);

    //@{
    /** @short Function for converting single object modification.
    *
    *   @param  modification Instance of modifications from Db::ObjectModification variant.
    *   @return string representation of each modification saying, that somebody else made it
    */
    std::string operator()(const Db::CreateObjectModification &modification) const;
    std::string operator()(const Db::DeleteObjectModification &modification) const;
    std::string operator()(const Db::RenameObjectModification &modification) const;
    std::string operator()(const Db::SetAttributeModification &modification) const;
    //@}
    
private:
    /** Vector of deleted objects in out changeset */
    std::vector<ObjectDefinition> m_ourDeletions;
};



/** @short Visitor for converting ObjectModification made by someone else in newer revision to user readable format
*          for purposes of rebase.
*/
struct ExternModificationConverter2: public boost::static_visitor<std::string>
{
     /** @short Constructor assignes vector of our deletions for proper output
    *
    *   @param ourDeletions Vector of deleted objects in out changeset
    */
    ExternModificationConverter2(const std::vector<ObjectDefinition> &ourDeletions);

    //@{
    /** @short Function for converting single object modification.
    *
    *   @param  modification Instance of modifications from Db::ObjectModification variant.
    *   @return string representation of each modification saying, that somebody else made it
    */
    template <typename LM>
    std::string operator()(const Db::CreateObjectModification &modification, const LM &lModification) const;
    template <typename LM>
    std::string operator()(const Db::DeleteObjectModification &modification, const LM &lModification) const;
    template <typename LM>
    std::string operator()(const Db::RenameObjectModification &modification, const LM &lModification) const;
    template <typename LM>
    std::string operator()(const Db::SetAttributeModification &modification, const LM &lModification) const;
    std::string operator()(const Db::SetAttributeModification &modification,
                           const Db::SetAttributeModification &lModification) const;
    //@}
    
private:
    /** Vector of deleted objects in out changeset */
    std::vector<ObjectDefinition> m_ourDeletions;
};



/** @short Visitor for converting ObjectModification made in both newer revision and the changeset
*          to user readable format for purposes of rebase.
*/
struct BothModificationConverter: public boost::static_visitor<std::string>
{
     /** @short Constructor assignes vector of our deletions for proper output
    *
    *   @param ourDeletions Vector of deleted objects in out changeset
    */
    BothModificationConverter(const std::vector<ObjectDefinition> &ourDeletions);

    //@{
    /** @short Function for converting single object modification.
    *
    *   @param  modification Instance of modifications from Db::ObjectModification variant.
    *   @return string representation of each modification saying, that it was made in both our and
    *           somebody else
    */
    std::string operator()(const Db::CreateObjectModification &modification) const;
    std::string operator()(const Db::DeleteObjectModification &modification) const;
    std::string operator()(const Db::RenameObjectModification &modification) const;
    std::string operator()(const Db::SetAttributeModification &modification) const;
    //@}
    
private:
    /** Vector of deleted objects in out changeset */
    std::vector<ObjectDefinition> m_ourDeletions;
};



/** @short Visitor for converting ObjectModification made in both newer revision and the changeset
*          to user readable format for purposes of rebase.
*/
struct BothModificationConverter2: public boost::static_visitor<std::string>
{
     /** @short Constructor assignes vector of our deletions for proper output
    *
    *   @param ourDeletions Vector of deleted objects in out changeset
    */
    BothModificationConverter2(const std::vector<ObjectDefinition> &ourDeletions);

    //@{
    /** @short Function for converting single object modification.
    *
    *   @param  modification Instance of modifications from Db::ObjectModification variant.
    *   @return string representation of each modification saying, that it was made in both our and
    *           somebody else
    */
    template <typename LM>
    std::string operator()(const Db::CreateObjectModification &modification, const LM &lModification) const;
    template <typename LM>
    std::string operator()(const Db::DeleteObjectModification &modification, const LM &lModification) const;
    template <typename LM>
    std::string operator()(const Db::RenameObjectModification &modification, const LM &lModification) const;
    template <typename LM>
    std::string operator()(const Db::SetAttributeModification &modification, const LM &lModification) const;
    std::string operator()(const Db::SetAttributeModification &modification,
                           const Db::SetAttributeModification &lModification) const;
    //@}
    
private:
    /** Vector of deleted objects in out changeset */
    std::vector<ObjectDefinition> m_ourDeletions;
};



OurModificationConverter::OurModificationConverter(const std::vector<ObjectDefinition> &ourDeletions):
    m_ourDeletions(ourDeletions)
{
};



std::string OurModificationConverter::operator()(const Db::CreateObjectModification &modification) const
{
    std::ostringstream ostr;
    ostr << "create " << modification.kindName << " " << modification.objectName << std::endl;
    return ostr.str();
}



std::string OurModificationConverter::operator()(const Db::DeleteObjectModification &modification) const
{
    std::ostringstream ostr;
    ostr << "delete " << modification.kindName << " " << modification.objectName << std::endl;
    return ostr.str();
}



std::string OurModificationConverter::operator()(const Db::RenameObjectModification &modification) const
{
    std::ostringstream ostr;
    ostr << "rename " << modification.kindName << " " << modification.oldObjectName << " "
         << modification.newObjectName << std::endl;
    return ostr.str();
}



std::string OurModificationConverter::operator()(const Db::SetAttributeModification &modification) const
{
    std::ostringstream ostr;
    ostr << modification.kindName << " " << modification.objectName << std::endl;
    ostr << "#    " << modification.attributeName << " set"
         << readableAttrPrinter(" from", modification.oldAttributeData) << std::endl;
    if (modification.attributeData) {
        ostr << "     " << modification.attributeName << " " << boost::apply_visitor(NonOptionalValuePrettyPrint(), *(modification.attributeData)) << std::endl;
    } else {
        ostr << "     no " << modification.attributeName << std::endl;
    }
    return ostr.str();
}



OurModificationConverter2::OurModificationConverter2(const std::vector<ObjectDefinition> &ourDeletions):
    m_ourDeletions(ourDeletions)
{
};



template <typename LM>
std::string OurModificationConverter2::operator()(const Db::CreateObjectModification &modification, const LM &lModification) const
{
    OurModificationConverter conv(m_ourDeletions);
    return conv(modification);
}



template <typename LM>
std::string OurModificationConverter2::operator()(const Db::DeleteObjectModification &modification, const LM &lModification) const
{
    OurModificationConverter conv(m_ourDeletions);
    return conv(modification);
}



template <typename LM>
std::string OurModificationConverter2::operator()(const Db::RenameObjectModification &modification, const LM &lModification) const
{
    OurModificationConverter conv(m_ourDeletions);
    return conv(modification);
}



template <typename LM>
std::string OurModificationConverter2::operator()(const Db::SetAttributeModification &modification, const LM &lModification) const
{
    OurModificationConverter conv(m_ourDeletions);
    return conv(modification);
}



std::string OurModificationConverter2::operator()(const Db::SetAttributeModification &modification,
                                                  const Db::SetAttributeModification &lModification) const
{
    if ((modification.kindName == lModification.kindName) && (modification.objectName == lModification.objectName)) {
        std::ostringstream ostr;
        ostr << "#    " << modification.attributeName << " set"
             << readableAttrPrinter(" from", modification.oldAttributeData) << std::endl;
        if (modification.attributeData) {
            ostr << "     " << modification.attributeName << " " << boost::apply_visitor(NonOptionalValuePrettyPrint(), *(modification.attributeData)) << std::endl;
        } else {
            ostr << "     no " << modification.attributeName << std::endl;
        }
        return ostr.str();
    } else {
        std::ostringstream ostr;
        // Commenting end if it was from some deleted object. Its definition was also commented in this case.
        std::vector<ObjectDefinition>::const_iterator it = std::find(m_ourDeletions.begin(), m_ourDeletions.end(),
            ObjectDefinition(lModification.kindName, lModification.objectName));
        if (it != m_ourDeletions.end())
            ostr << "# ";
        ostr << "end" << std::endl;
        ostr << modification.kindName << " " << modification.objectName << std::endl;
        ostr << "#    " << modification.attributeName << " set"
             << readableAttrPrinter(" from", modification.oldAttributeData) << std::endl;
        if (modification.attributeData) {
            ostr << "     " << modification.attributeName << " " << boost::apply_visitor(NonOptionalValuePrettyPrint(), *(modification.attributeData)) << std::endl;
        } else {
            ostr << "     no " << modification.attributeName << std::endl;
        }
        return ostr.str();
    }
}



ExternModificationConverter::ExternModificationConverter(const std::vector<ObjectDefinition> &ourDeletions):
    m_ourDeletions(ourDeletions)
{
};



std::string ExternModificationConverter::operator()(const Db::CreateObjectModification &modification) const
{
    std::ostringstream ostr;
    ostr << "# created " << modification.kindName << " " << modification.objectName << " in newer revision" << std::endl;
    return ostr.str();
}



std::string ExternModificationConverter::operator()(const Db::DeleteObjectModification &modification) const
{
    std::ostringstream ostr;
    ostr << "# deleted " << modification.kindName << " " << modification.objectName << " in newer revision" << std::endl;
    return ostr.str();
}



std::string ExternModificationConverter::operator()(const Db::RenameObjectModification &modification) const
{
    std::ostringstream ostr;
    ostr << "# renamed " << modification.kindName << " " << modification.oldObjectName << " to "
         << modification.newObjectName << " in newer revision" << std::endl;
    return ostr.str();
}



std::string ExternModificationConverter::operator()(const Db::SetAttributeModification &modification) const
{
    std::ostringstream ostr;
    std::vector<ObjectDefinition>::const_iterator it = std::find(m_ourDeletions.begin(), m_ourDeletions.end(),
        ObjectDefinition(modification.kindName, modification.objectName));
    // Commenting object definition if the object was deleted by us. If we did not comment it, it would be created.
    if (it != m_ourDeletions.end())
        ostr << "# ";
    ostr << modification.kindName << " " << modification.objectName << std::endl;
    ostr << "#    " << modification.attributeName << " set"
         << readableAttrPrinter(" from", modification.oldAttributeData)
         << readableAttrPrinter(" to", modification.attributeData) << " in newer revision" << std::endl;
    return ostr.str();
}



ExternModificationConverter2::ExternModificationConverter2(const std::vector<ObjectDefinition> &ourDeletions):
    m_ourDeletions(ourDeletions)
{
};



template <typename LM>
std::string ExternModificationConverter2::operator()(const Db::CreateObjectModification &modification, const LM &lModification) const
{
    ExternModificationConverter conv(m_ourDeletions);
    return conv(modification);
}



template <typename LM>
std::string ExternModificationConverter2::operator()(const Db::DeleteObjectModification &modification, const LM &lModification) const
{
    ExternModificationConverter conv(m_ourDeletions);
    return conv(modification);
}



template <typename LM>
std::string ExternModificationConverter2::operator()(const Db::RenameObjectModification &modification, const LM &lModification) const
{
    ExternModificationConverter conv(m_ourDeletions);
    return conv(modification);
}



template <typename LM>
std::string ExternModificationConverter2::operator()(const Db::SetAttributeModification &modification, const LM &lModification) const
{
    ExternModificationConverter conv(m_ourDeletions);
    return conv(modification);
}



std::string ExternModificationConverter2::operator()(const Db::SetAttributeModification &modification,
                                                     const Db::SetAttributeModification &lModification) const
{
    if ((modification.kindName == lModification.kindName) && (modification.objectName == lModification.objectName)) {
        std::ostringstream ostr;
        ostr << "#    " << modification.attributeName << " set"
             << readableAttrPrinter(" from", modification.oldAttributeData)
             << readableAttrPrinter(" to", modification.attributeData) << " in newer revision" << std::endl;
        return ostr.str();
    } else {
        std::ostringstream ostr;
        std::vector<ObjectDefinition>::const_iterator it = std::find(m_ourDeletions.begin(), m_ourDeletions.end(),
            ObjectDefinition(lModification.kindName, lModification.objectName));
        // Commenting end if it was from some deleted object. Its definition was also commented in this case.
        if (it != m_ourDeletions.end())
            ostr << "# ";
        ostr << "end" << std::endl;
        it = std::find(m_ourDeletions.begin(), m_ourDeletions.end(),
            ObjectDefinition(modification.kindName, modification.objectName));
        // Commenting object definition if the object was deleted by us. If we did not comment it, it would be created.
        if (it != m_ourDeletions.end())
            ostr << "# ";
        ostr << modification.kindName << " " << modification.objectName << std::endl;
        ostr << "#    " << modification.attributeName << " set"
             << readableAttrPrinter(" from", modification.oldAttributeData)
             << readableAttrPrinter(" to", modification.attributeData) << " in newer revision" << std::endl;
        return ostr.str();
    }
}



BothModificationConverter::BothModificationConverter(const std::vector<ObjectDefinition> &ourDeletions):
    m_ourDeletions(ourDeletions)
{
};



std::string BothModificationConverter::operator()(const Db::CreateObjectModification &modification) const
{
    std::ostringstream ostr;
    ostr << "# created " << modification.kindName << " " << modification.objectName
         << " in both newer revision and our changeset" << std::endl;
    return ostr.str();
}



std::string BothModificationConverter::operator()(const Db::DeleteObjectModification &modification) const
{
    std::ostringstream ostr;
    ostr << "# deleted " << modification.kindName << " " << modification.objectName
         << " in both newer revision and our changeset" << std::endl;
    return ostr.str();
}



std::string BothModificationConverter::operator()(const Db::RenameObjectModification &modification) const
{
    std::ostringstream ostr;
    ostr << "# renamed " << modification.kindName << " " << modification.oldObjectName << " to "
        << modification.newObjectName << " in both newer revision and our changeset" << std::endl;
    return ostr.str();
}



std::string BothModificationConverter::operator()(const Db::SetAttributeModification &modification) const
{
    std::ostringstream ostr;
    ostr << modification.kindName << " " << modification.objectName << std::endl;
    ostr << "#    " << modification.attributeName << " set"
         << readableAttrPrinter(" from", modification.oldAttributeData)
         << readableAttrPrinter(" to", modification.attributeData)
         << " in both newer revision and our changeset" << std::endl;
    return ostr.str();
}



BothModificationConverter2::BothModificationConverter2(const std::vector<ObjectDefinition> &ourDeletions):
    m_ourDeletions(ourDeletions)
{
};



template <typename LM>
std::string BothModificationConverter2::operator()(const Db::CreateObjectModification &modification, const LM &lModification) const
{
    BothModificationConverter conv(m_ourDeletions);
    return conv(modification);
}



template <typename LM>
std::string BothModificationConverter2::operator()(const Db::DeleteObjectModification &modification, const LM &lModification) const
{
    BothModificationConverter conv(m_ourDeletions);
    return conv(modification);
}



template <typename LM>
std::string BothModificationConverter2::operator()(const Db::RenameObjectModification &modification, const LM &lModification) const
{
    BothModificationConverter conv(m_ourDeletions);
    return conv(modification);
}



template <typename LM>
std::string BothModificationConverter2::operator()(const Db::SetAttributeModification &modification, const LM &lModification) const
{
    BothModificationConverter conv(m_ourDeletions);
    return conv(modification);
}



std::string BothModificationConverter2::operator()(const Db::SetAttributeModification &modification,
                                                   const Db::SetAttributeModification &lModification) const
{
    if ((modification.kindName == lModification.kindName) && (modification.objectName == lModification.objectName)) {
        std::ostringstream ostr;
        ostr << "#    " << modification.attributeName << " set"
             << readableAttrPrinter(" from", modification.oldAttributeData)
             << readableAttrPrinter(" to", modification.attributeData)
             << " in both newer revision and our changeset" << std::endl;
    return ostr.str();
    } else {
        std::ostringstream ostr;
        std::vector<ObjectDefinition>::const_iterator it = std::find(m_ourDeletions.begin(), m_ourDeletions.end(),
            ObjectDefinition(lModification.kindName, lModification.objectName));
        // Commenting end if it was from some deleted object. Its definition was also commented in this case.
        if (it != m_ourDeletions.end())
            ostr << "# ";
        ostr << "end" << std::endl;
        ostr << modification.kindName << " " << modification.objectName << std::endl;
        ostr << "#    " << modification.attributeName << " set"
             << readableAttrPrinter(" from", modification.oldAttributeData)
             << readableAttrPrinter(" to", modification.attributeData)
             << " in both newer revision and our changeset" << std::endl;
        return ostr.str();
    }
}



/** @short Visitor for checking if one modification is a delete of parent of another */
struct ModificationDeleteNested: public boost::static_visitor<bool>
{
    /** @short Constructor assignes pointer to the DbInteraction for obtaining nesting parents
    *
    *   @param db pointer to the DbInteraction
    */
    ModificationDeleteNested(const DbInteraction *db);

    //@{
    /** @short Function for checking two delete modifications if one modification is a delete of parent of another
    *
    *   @param a Instance of Db::DeleteObjectModification
    *   @param b Instance of Db::DeleteObjectModification
    *   @return True if the object in first modification is parent of the object from the second one
    */
    bool operator()(const Db::DeleteObjectModification &a, const Db::DeleteObjectModification &b) const;

    /** @short We do not care about other types of pairs. This function is always returning false.
    */
    template <typename MA, typename MB>
    bool operator()(const MA &a, const MB &b) const;

private:
    /** pointer to the DbInteraction for obtaining nesting parents */
    const DbInteraction *m_db;
};



ModificationDeleteNested::ModificationDeleteNested(const DbInteraction *db): m_db(db)
{
}



bool ModificationDeleteNested::operator()(const Db::DeleteObjectModification &a,
                                          const Db::DeleteObjectModification &b) const
{
    Db::Identifier parent = m_db->embeddedIntoKind(b.kindName);
    if (!parent.empty() && (parent == a.kindName)) {
        std::vector<Db::Identifier> path = pathToVector(b.objectName);
        BOOST_ASSERT(path.size() > 1);
        Db::Identifier expParentName = vectorToPath(std::vector<Db::Identifier>(path.begin(), path.end() - 1));
        if (expParentName == a.objectName)
            return true;
    }
    return false;
}



template <typename MA, typename MB>
bool ModificationDeleteNested::operator()(const MA &a, const MB &b) const
{
    return false;
}



/** @short Visitor for extracting object definitions from Db::DeleteObjectModification */
struct ModificationDeleteExtractor: public boost::static_visitor<boost::optional<ObjectDefinition> >
{
    //@{
    /** @short Function for extracting object definition from Db::DeleteObjectModification
    *
    *   @param m Instance of Db::DeleteObjectModification
    *   @return ObjectDefinition of deleted object
    */
    boost::optional<ObjectDefinition> operator()(const Db::DeleteObjectModification &m) const;

    /** @short We do not care about other types of modifications.
    */
    template <typename M>
    boost::optional<ObjectDefinition> operator()(const M &m) const;
};



boost::optional<ObjectDefinition> ModificationDeleteExtractor::operator()(const Db::DeleteObjectModification &m) const
{
    return ObjectDefinition(m.kindName, m.objectName);
}



template <typename M>
boost::optional<ObjectDefinition> ModificationDeleteExtractor::operator()(const M &m) const
{
    return boost::optional<ObjectDefinition>();
}



/** @short Visitor for extracting object definitions from Db::SetAttributeModification */
struct ModificationSetAttrExtractor: public boost::static_visitor<ObjectDefinition>
{
    //@{
    /** @short Function for extracting object definition from Db::SetAttributeModification
    *
    *   @param m Instance of Db::SetAttributeModification
    *   @return ObjectDefinition of deleted object
    */
    ObjectDefinition operator()(const Db::SetAttributeModification &m) const;

    /** @short We do not care about other types of modifications.
    */
    template <typename M>
    ObjectDefinition operator()(const M &m) const;
};



ObjectDefinition ModificationSetAttrExtractor::operator()(const Db::SetAttributeModification &m) const
{
    return ObjectDefinition(m.kindName, m.objectName);
}



template <typename M>
ObjectDefinition ModificationSetAttrExtractor::operator()(const M &m) const
{
    throw std::invalid_argument("Deska::Cli::ModificationSetAttrExtractor::operator(): Comparator called to unsupported ObjectModificationResult.");
}



void clearChildDeletions(std::vector<Db::ObjectModificationResult> &modifications, const DbInteraction* db)
{
    ModificationDeleteNested modificationDeleteNested(db);
    for(;;) {
        std::vector<Db::ObjectModificationResult>::iterator it = modifications.begin();
        std::vector<Db::ObjectModificationResult>::iterator itn = modifications.begin();
        bool breakLoop = false;
        for (; it != modifications.end(); ++it) {
            for (; itn != modifications.end(); ++itn) {
                if (boost::apply_visitor(modificationDeleteNested, *it, *itn)) {
                    modifications.erase(itn);
                    breakLoop = true;
                    break;
                }
            }
            if (breakLoop)
                break;
        }
        if ((it == modifications.end()) && (itn == modifications.end()))
            break;
    }
}



Rebase::Rebase(UserInterface *userInterface): Command(userInterface)
{
    cmdName = "rebase";
    cmdUsage = "Rebases current changeset.";
    complPatterns.push_back("rebase");
}



Rebase::~Rebase()
{
}



bool Rebase::operator()(const std::string &params)
{
    if (!params.empty()) {
        ui->io->reportError("Error: No parameters expected for command " + cmdName + ".");
        return false;
    }
    if (!(ui->currentChangeset)) {
        ui->io->reportError("Error: You are not in any changeset!");
        return false;
    }
    Db::RevisionId oldParentRevision = ui->m_dbInteraction->changesetParent(*(ui->currentChangeset));
    Db::RevisionId headRevision = ui->m_dbInteraction->allRevisions().back().revision;
    if (headRevision == oldParentRevision) {
        ui->io->printMessage("No rebase needed.");
        return true;
    }

    if (ui->nonInteractiveMode || ui->forceNonInteractive) {
        ui->io->reportError("You can not perform rebase in non-interactive mode.");
        return false;
    }
    
    try {
        ui->m_dbInteraction->lockCurrentChangeset();
    } catch (Db::ChangesetLockingError &e) {
        ui->io->reportError("Error while locking old changeset for rebase.");
        return false;
    }

    // Start new changeset for rebase
    Db::TemporaryChangesetId oldChangeset = *(ui->currentChangeset);
    ui->m_dbInteraction->detachFromChangeset("Rebase in progress");
    Db::TemporaryChangesetId newChangeset = ui->m_dbInteraction->createNewChangeset();
    try {
        ui->m_dbInteraction->lockCurrentChangeset();
    } catch (Db::ChangesetLockingError &e) {
        ui->io->reportError("Error while locking new changeset for rebase.");
        ui->m_dbInteraction->abortChangeset();
        ui->m_dbInteraction->resumeChangeset(oldChangeset);
        try {
            ui->m_dbInteraction->unlockCurrentChangeset();
        } catch (Db::ChangesetLockingError &e) {
            ui->io->reportError("Error while unlocking old changeset after rebase failure.");
        }
        return false;
    }
    Db::RevisionId newParentRevision = ui->m_dbInteraction->changesetParent(newChangeset);

    // Obtain modifications lists for three-way diff
    std::vector<Db::ObjectModificationResult> externModifications = ui->m_dbInteraction->revisionsDifference(
        oldParentRevision, newParentRevision);
    std::vector<Db::ObjectModificationResult> ourModifications = ui->m_dbInteraction->revisionsDifferenceChangeset(
        oldChangeset);

#ifdef REBASE_DEBUG
    std::ofstream odlfs("deska_rebase.log");
    if (!odlfs)
        ui->io->reportError("Error while creating log file \"deska_rebase.log\" for rebase.");
    odlfs << "DEBUG: External modifications:" << std::endl;
    OurModificationConverter modificationDebugConverter;
    for (std::vector<Db::ObjectModificationResult>::iterator it = externModifications.begin();
        it != externModifications.end(); ++it)
        odlfs << boost::apply_visitor(modificationDebugConverter, *it) << std::endl;
    odlfs << "DEBUG: Our modifications:" << std::endl;
    for (std::vector<Db::ObjectModificationResult>::iterator it = ourModifications.begin();
        it != ourModifications.end(); ++it)
        odlfs << boost::apply_visitor(modificationDebugConverter, *it) << std::endl;
    odlfs.close();
#endif

    std::vector<ObjectDefinition> ourDeletions;
    ModificationDeleteExtractor modificationDeleteExtractor;
    for (std::vector<Db::ObjectModificationResult>::iterator it = ourModifications.begin();
         it != ourModifications.end(); ++it) {
             boost::optional<ObjectDefinition> deletion = boost::apply_visitor(modificationDeleteExtractor, *it);
             if (deletion)
                 ourDeletions.push_back(*deletion);
    }

    clearChildDeletions(externModifications, ui->m_dbInteraction);
    clearChildDeletions(ourModifications, ui->m_dbInteraction);

    // Erasing modifications, that should not be stored (eg. read-only attributes)
    ModificationBackupChecker modificationBackupChecker(ui->m_dbInteraction);
    for (std::vector<Db::ObjectModificationResult>::iterator it = externModifications.begin();
         it != externModifications.end();) {
        if (!boost::apply_visitor(modificationBackupChecker, *it))
            externModifications.erase(it);
        else
            ++it;
    }
    for (std::vector<Db::ObjectModificationResult>::iterator it = ourModifications.begin();
         it != ourModifications.end();) {
        if (!boost::apply_visitor(modificationBackupChecker, *it))
            ourModifications.erase(it);
        else
            ++it;
    }

    // Sort modifications for to merge the lists
    std::sort(externModifications.begin(), externModifications.end(), Rebase::objectModificationResultLess);
    std::sort(ourModifications.begin(), ourModifications.end(),Rebase::objectModificationResultLess);

    // Merge the lists in the temporary file in user readable and edittable format
    std::vector<Db::ObjectModificationResult>::iterator ite = externModifications.begin();
    std::vector<Db::ObjectModificationResult>::iterator ito = ourModifications.begin();
    char tempFile[] = "/tmp/DeskaRebaseXXXXXXXX";
    if (mkstemp(tempFile) == -1) {
        ui->io->reportError("Error while creating rebase temp file \"" + std::string(tempFile) + "\".");
        ui->m_dbInteraction->abortChangeset();
        ui->m_dbInteraction->resumeChangeset(oldChangeset);
        try {
            ui->m_dbInteraction->unlockCurrentChangeset();
        } catch (Db::ChangesetLockingError &e) {
            ui->io->reportError("Error while unlocking old changeset after rebase failure.");
        }
        return false;
    }
    std::ofstream ofs(tempFile);
    if (!ofs) {
        ui->io->reportError("Error while opening rebase temp file \"" + std::string(tempFile) + "\".");
        remove(tempFile);
        ui->m_dbInteraction->abortChangeset();
        ui->m_dbInteraction->resumeChangeset(oldChangeset);
        try {
            ui->m_dbInteraction->unlockCurrentChangeset();
        } catch (Db::ChangesetLockingError &e) {
            ui->io->reportError("Error while unlocking old changeset after rebase failure.");
        }
        return false;
    }
    ExternModificationConverter externModificationConverter(ourDeletions);
    BothModificationConverter bothModificationConverter(ourDeletions);
    OurModificationConverter ourModificationConverter(ourDeletions);
    ExternModificationConverter2 externModificationConverter2(ourDeletions);
    BothModificationConverter2 bothModificationConverter2(ourDeletions);
    OurModificationConverter2 ourModificationConverter2(ourDeletions);
    std::vector<Db::ObjectModificationResult>::iterator lastModif;
    while ((ite != externModifications.end()) && (ito != ourModifications.end())) {
        if (objectModificationResultLess(*ite, *ito)) {
            if ((ite != externModifications.begin()) || (ito != ourModifications.begin()))
                ofs << boost::apply_visitor(externModificationConverter2, *ite, *lastModif);
            else
                ofs << boost::apply_visitor(externModificationConverter, *ite);
            lastModif = ite;
            ++ite;
        } else if (*ite == *ito) {
            if ((ite != externModifications.begin()) || (ito != ourModifications.begin()))
                ofs << boost::apply_visitor(bothModificationConverter2, *ite, *lastModif);
            else
                ofs << boost::apply_visitor(bothModificationConverter, *ite);
            lastModif = ite;
            ++ite;
            ++ito;
        } else {
            if ((ite != externModifications.begin()) || (ito != ourModifications.begin()))
                ofs << boost::apply_visitor(ourModificationConverter2, *ito, *lastModif);
            else
                ofs << boost::apply_visitor(ourModificationConverter, *ito);
            lastModif = ito;
            ++ito;
        }
    }
    while (ite != externModifications.end()) {
        if ((ite != externModifications.begin()) || (ito != ourModifications.begin()))
            ofs << boost::apply_visitor(externModificationConverter2, *ite, *lastModif);
        else
            ofs << boost::apply_visitor(externModificationConverter, *ite);
        lastModif = ite;
        ++ite;
    }
    while (ito != ourModifications.end()) {
        if ((ite != externModifications.begin()) || (ito != ourModifications.begin()))
            ofs << boost::apply_visitor(ourModificationConverter2, *ito, *lastModif);
        else
            ofs << boost::apply_visitor(ourModificationConverter, *ito);
        lastModif = ito;
        ++ito;
    }

    // We have to add last end if necessary as modification converters print end only for previous object and do not
    // look to the next one
    ModificationTypeGetter modificationTypeGetter;
    ModificationSetAttrExtractor modificationSetAttrExtractor;
    if (!ourModifications.empty() || !externModifications.empty()) {
        if (!ourModifications.empty() && (lastModif == (ourModifications.end() - 1))) {
            if (boost::apply_visitor(modificationTypeGetter, *lastModif) == OBJECT_MODIFICATION_TYPE_SETATTR)
                ofs << "end" << std::endl;
        } else if (!externModifications.empty() && (lastModif == (externModifications.end() - 1))) {
            if (boost::apply_visitor(modificationTypeGetter, *lastModif) == OBJECT_MODIFICATION_TYPE_SETATTR) {
                std::vector<ObjectDefinition>::const_iterator it = std::find(ourDeletions.begin(), ourDeletions.end(),
                    boost::apply_visitor(modificationSetAttrExtractor, *lastModif));
                if (it != ourDeletions.end())
                    ofs << "# ";
                ofs << "end" << std::endl;
            }
        } else {
            throw std::logic_error("Deska::CLI::Rebase::operator(): Last modification not found.");
        }
    }

    ofs.close();

    bool conflictResolved = false;

    while (!conflictResolved) {
        // User resolves conflicts using text editor
        ui->io->editFile(tempFile);

        // Open the file after the user actions and apply changes
        std::ifstream ifs(tempFile);
        if (!ifs) {
            ui->io->reportError("Error while opening resolved conflicts file \"" + std::string(tempFile) + "\".");
            remove(tempFile);
            ui->m_dbInteraction->abortChangeset();
            ui->m_dbInteraction->resumeChangeset(oldChangeset);
            try {
                ui->m_dbInteraction->unlockCurrentChangeset();
            } catch (Db::ChangesetLockingError &e) {
                ui->io->reportError("Error while unlocking old changeset after rebase failure.");
            }
            return false;
        }

        ui->nonInteractiveMode = true;
        std::string line;
        ui->m_parser->clearContextStack();
        unsigned int lineNumber = 0;
        while (!getline(ifs, line).eof()) {
            ++lineNumber;
            if (line.empty() || line[0] == '#')
                continue;
            try {
                ui->m_parser->parseLine(line);
            } catch (Db::ConstraintError &e) {
                ui->parsingFailed = true;
                std::ostringstream ostr;
                ostr << "DB constraint violation:\n " << e.what() << std::endl;
                ui->io->reportError(ostr.str());
            } catch (Db::NotFoundError &e) {
                ui->parsingFailed = true;
                std::ostringstream ostr;
                ostr << "DB reference constraint violation:\n " << e.what() << std::endl;
                ui->io->reportError(ostr.str());
            } catch (Db::RemoteDbError &e) {
                ui->parsingFailed = true;
                std::ostringstream ostr;
                ostr << "Unexpected server error:\n " << e.whatWithBacktrace() << std::endl;
                ui->io->reportError(ostr.str());
            } catch (Db::JsonParseError &e) {
                ui->parsingFailed = true;
                std::ostringstream ostr;
                ostr << "Unexpected JSON error:\n " << e.whatWithBacktrace() << std::endl;
                ui->io->reportError(ostr.str());
            }
            if (ui->parsingFailed)
                break;
        }
        ui->m_parser->clearContextStack();
        ui->nonInteractiveMode = false;
        ifs.close();
        if (ui->parsingFailed) {
            std::ostringstream ostr;
            ostr << "Parsing of resolved conflicts file failed on line " << lineNumber << ".";
            ui->io->reportError(ostr.str());
            // FIXME: Make all changes batched in order to be atomical and new attemp could be done
            //if (!ui->io->askForConfirmation("Try to resolve conflict again?")) {
                remove(tempFile);
                ui->m_dbInteraction->abortChangeset();
                ui->m_dbInteraction->resumeChangeset(oldChangeset);
                try {
                    ui->m_dbInteraction->unlockCurrentChangeset();
                } catch (Db::ChangesetLockingError &e) {
                    ui->io->reportError("Error while unlocking old changeset after rebase failure.");
                }
                return false;
            //}
        } else {
            conflictResolved = true;
        }
    }
    remove(tempFile);

    // Delete old obsolete changeset and connect to the new one
    ui->m_dbInteraction->detachFromChangeset("Rebase in progress");
    ui->m_dbInteraction->resumeChangeset(oldChangeset);
    ui->m_dbInteraction->abortChangeset();
    ui->m_dbInteraction->resumeChangeset(newChangeset);
    ui->currentChangeset = newChangeset;

    try {
        ui->m_dbInteraction->unlockCurrentChangeset();
    } catch (Db::ChangesetLockingError &e) {
        ui->io->reportError("Error while unlocking new changeset after rebase.");
        return false;
    }
    ui->io->printMessage("Rebase successful.");
    return true;
}



bool Rebase::objectModificationResultLess(const Db::ObjectModificationResult &a, const Db::ObjectModificationResult &b)
{
    ModificationTypeGetter modificationTypeGetter;
    ModificationComparatorLesss modificationComparatorLesss;
    if (boost::apply_visitor(modificationTypeGetter, a) < boost::apply_visitor(modificationTypeGetter, b)) {
        return true;
    } else if (boost::apply_visitor(modificationTypeGetter, a) > boost::apply_visitor(modificationTypeGetter, b)) {
        return false;
    } else {
        return (boost::apply_visitor(modificationComparatorLesss, a, b));
    }
}



Diff::Diff(UserInterface *userInterface): Command(userInterface)
{
    cmdName = "diff";
    cmdUsage = "Command for showing difference between revisions. Without parameters shows diff of current changeset with its parent. With two parameters, revisions IDs shows diff between these revisions. You can give a filename as a first parameter in both cases to produce patch between two revisions or backup of current changeset work.";
    complPatterns.push_back("diff %file");
}



Diff::~Diff()
{
}



bool Diff::operator()(const std::string &params)
{
    if (params.empty()) {
        if (!ui->currentChangeset) {
            ui->io->reportError("Error: You have to be connected to a changeset to perform diff with its parent. Use commands \"start\" or \"resume\". Use \"help\" for more info.");
            return false;
        }
        // Show diff with parent
        std::vector<Db::ObjectModificationResult> modifications = ui->m_dbInteraction->revisionsDifferenceChangeset(
            *(ui->currentChangeset));
        ui->io->printDiff(modifications);
        return true;
    }

    std::vector<std::string> paramsList = extractParams(params);
    if (paramsList.size() > 3) {
        ui->io->reportError("Invalid number of parameters entered!");
        return false;
    }

    if (paramsList.size() == 1) {
        // Create patch to current changeset
        if (!ui->currentChangeset) {
            ui->io->reportError("Error: You have to be connected to a changeset to perform diff with its parent. Use commands \"start\" or \"resume\". Use \"help\" for more info.");
            return false;
        }
        std::ofstream ofs(paramsList[0].c_str());
        if (!ofs) {
            ui->io->reportError("Error while creating patch to file \"" + params + "\".");
            return false;
        }
        std::vector<Db::ObjectModificationResult> modifications = ui->m_dbInteraction->revisionsDifferenceChangeset(
            *(ui->currentChangeset));
        std::sort(modifications.begin(), modifications.end(), Diff::objectModificationResultLess);
        clearChildDeletions(modifications, ui->m_dbInteraction);
        ModificationBackuper modificationBackuper;
        ModificationBackupChecker modificationBackupChecker(ui->m_dbInteraction);
        for (std::vector<Db::ObjectModificationResult>::iterator itm = modifications.begin(); itm != modifications.end(); ++itm) {
            if (boost::apply_visitor(modificationBackupChecker, *itm))
                ofs << boost::apply_visitor(modificationBackuper, *itm) << std::endl;
        }
        ofs.close();
        ui->io->printMessage("Patch successfully created into file \"" + paramsList[0] + "\".");
        return true;
    } else if (paramsList.size() >= 2) {
        // Diff between two revisions
        boost::optional<Db::RevisionId> revA, revB;
        std::vector<Db::ObjectModificationResult> modifications;
        try {
            if (paramsList.size() == 2) {
                revA = Deska::Db::RevisionId::fromString(paramsList[0]);
                revB = Deska::Db::RevisionId::fromString(paramsList[1]);
            } else {
                revA = Deska::Db::RevisionId::fromString(paramsList[1]);
                revB = Deska::Db::RevisionId::fromString(paramsList[2]);
            }
        } catch (std::runtime_error &e) {
            std::ostringstream ss;
            ss << "Invalid parameters: " << e.what();
            ui->io->reportError(ss.str());
            return false;
        }
        try {
            modifications = ui->m_dbInteraction->revisionsDifference(*revA, *revB);
        } catch (Db::RevisionRangeError &e) {
            ui->io->reportError("Revision range does not make a sense or revision does not exist.");
            return false;
        }
        if (paramsList.size() == 2) {
            // Print diff
            ui->io->printDiff(modifications);
            return true;
        } else {
            // Create patch
            std::sort(modifications.begin(), modifications.end(), Diff::objectModificationResultLess);
            std::ofstream ofs(paramsList[0].c_str());
            if (!ofs) {
                ui->io->reportError("Error while creating patch to file \"" + params + "\".");
                return false;
            }
            clearChildDeletions(modifications, ui->m_dbInteraction);
            ModificationBackuper modificationBackuper;
            ModificationBackupChecker modificationBackupChecker(ui->m_dbInteraction);
            for (std::vector<Db::ObjectModificationResult>::iterator itm = modifications.begin(); itm != modifications.end(); ++itm) {
                if (boost::apply_visitor(modificationBackupChecker, *itm))
                    ofs << boost::apply_visitor(modificationBackuper, *itm) << std::endl;
            }
            ofs.close();
            ui->io->printMessage("Patch successfully created into file \"" + paramsList[0] + "\".");
            return true;
        }
    }
    return true;
}



bool Diff::objectModificationResultLess(const Db::ObjectModificationResult &a, const Db::ObjectModificationResult &b)
{
    ModificationTypeGetter modificationTypeGetter;
    ModificationComparatorLesss modificationComparatorLesss;
    if (boost::apply_visitor(modificationTypeGetter, a) < boost::apply_visitor(modificationTypeGetter, b)) {
        return true;
    } else if (boost::apply_visitor(modificationTypeGetter, a) > boost::apply_visitor(modificationTypeGetter, b)) {
        return false;
    } else {
        return (boost::apply_visitor(modificationComparatorLesss, a, b));
    }
}

}
}
