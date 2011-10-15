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
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>

#include "CliCommands_Rebase.h"
#include "UserInterface.h"
#include "UserInterfaceIOBase.h"
#include "DbInteraction.h"
#include "Exceptions.h"
#include "Parser.h"
#include "deska/db/JsonApi.h"

namespace Deska {
namespace Cli {


/** @short Visitor for converting ObjectModification made by un in the changeset to user readable format
*          for purposes of rebase.
*/
struct OurModificationConverter: public boost::static_visitor<std::string>
{
    //@{
    /** @short Function for converting single object modification.
    *
    *   @param modification Instance of modifications from Db::ObjectModification variant.
    */
    std::string operator()(const Db::CreateObjectModification &modification) const;
    std::string operator()(const Db::DeleteObjectModification &modification) const;
    std::string operator()(const Db::RenameObjectModification &modification) const;
    std::string operator()(const Db::SetAttributeModification &modification) const;
    //@}
};



/** @short Visitor for converting ObjectModification made by someone else in newer revision to user readable format
*          for purposes of rebase.
*/
struct ExternModificationConverter: public boost::static_visitor<std::string>
{
    //@{
    /** @short Function for converting single object modification.
    *
    *   @param modification Instance of modifications from Db::ObjectModification variant.
    */
    std::string operator()(const Db::CreateObjectModification &modification) const;
    std::string operator()(const Db::DeleteObjectModification &modification) const;
    std::string operator()(const Db::RenameObjectModification &modification) const;
    std::string operator()(const Db::SetAttributeModification &modification) const;
    //@}
};



/** @short Visitor for converting ObjectModification made in both newer revision and the changeset
*          to user readable format for purposes of rebase.
*/
struct BothModificationConverter: public boost::static_visitor<std::string>
{
    //@{
    /** @short Function for converting single object modification.
    *
    *   @param modification Instance of modifications from Db::ObjectModification variant.
    */
    std::string operator()(const Db::CreateObjectModification &modification) const;
    std::string operator()(const Db::DeleteObjectModification &modification) const;
    std::string operator()(const Db::RenameObjectModification &modification) const;
    std::string operator()(const Db::SetAttributeModification &modification) const;
    //@}
};

std::string OurModificationConverter::operator()(const Db::CreateObjectModification &modification) const
{
    std::ostringstream ostr;
    ostr << "create " << modification.kindName << " " << modification.objectName;
    return ostr.str();
}



std::string OurModificationConverter::operator()(const Db::DeleteObjectModification &modification) const
{
    std::ostringstream ostr;
    ostr << "delete " << modification.kindName << " " << modification.objectName;
    return ostr.str();
}



std::string OurModificationConverter::operator()(const Db::RenameObjectModification &modification) const
{
    std::ostringstream ostr;
    ostr << "rename " << modification.kindName << " " << modification.oldObjectName << " " << modification.newObjectName;
    return ostr.str();
}



std::string OurModificationConverter::operator()(const Db::SetAttributeModification &modification) const
{
    std::ostringstream ostr;
    ostr << "#set attribute " << modification.attributeName <<
            readableAttrPrinter(" from", modification.oldAttributeData) << std::endl <<
            modification.kindName << " " << modification.objectName <<
            readableAttrPrinter(" to", modification.attributeData);
    return ostr.str();
}



std::string ExternModificationConverter::operator()(const Db::CreateObjectModification &modification) const
{
    std::ostringstream ostr;
    ostr << "# created " << modification.kindName << " " << modification.objectName << " in newer revision";
    return ostr.str();
}



std::string ExternModificationConverter::operator()(const Db::DeleteObjectModification &modification) const
{
    std::ostringstream ostr;
    ostr << "# deleted " << modification.kindName << " " << modification.objectName << " in newer revision";
    return ostr.str();
}



std::string ExternModificationConverter::operator()(const Db::RenameObjectModification &modification) const
{
    std::ostringstream ostr;
    ostr << "# renamed " << modification.kindName << " " << modification.oldObjectName << " to "
         << modification.newObjectName << " in newer revision";
    return ostr.str();
}



std::string ExternModificationConverter::operator()(const Db::SetAttributeModification &modification) const
{
    std::ostringstream ostr;
    ostr << "# attribute " << modification.kindName << " " << modification.objectName << " "
         << modification.attributeName << "set"
         << readableAttrPrinter(" from", modification.oldAttributeData)
         << readableAttrPrinter(" to", modification.attributeData) << " in newer revision";
    return ostr.str();
}



std::string BothModificationConverter::operator()(const Db::CreateObjectModification &modification) const
{
    std::ostringstream ostr;
    ostr << "# created " << modification.kindName << " " << modification.objectName
         << " in both newer revision and our changeset";
    return ostr.str();
}



std::string BothModificationConverter::operator()(const Db::DeleteObjectModification &modification) const
{
    std::ostringstream ostr;
    ostr << "# deleted " << modification.kindName << " " << modification.objectName
         << " in both newer revision and our changeset";
    return ostr.str();
}



std::string BothModificationConverter::operator()(const Db::RenameObjectModification &modification) const
{
    std::ostringstream ostr;
    ostr << "# renamed " << modification.kindName << " " << modification.oldObjectName << " to "
         << modification.newObjectName << " in both newer revision and our changeset";
    return ostr.str();
}



std::string BothModificationConverter::operator()(const Db::SetAttributeModification &modification) const
{
    std::ostringstream ostr;
    ostr << "attribute " << modification.kindName << " " << modification.objectName << " "
         << modification.attributeName << "set"
         << readableAttrPrinter(" from", modification.oldAttributeData)
         << readableAttrPrinter(" to", modification.attributeData)
         << " in both newer revision and our changeset";
    return ostr.str();
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



void Rebase::operator()(const std::string &params)
{
    if (!params.empty()) {
        ui->io->reportError("Error: No parameters expected for command " + cmdName + ".");
        return;
    }
    if (!(ui->currentChangeset)) {
        ui->io->reportError("Error: You are not in any changeset!");
        return;
    }
    Db::RevisionId oldParentRevision = ui->m_dbInteraction->changesetParent(*(ui->currentChangeset));
    Db::RevisionId headRevision = ui->m_dbInteraction->allRevisions().back().revision;
    if (headRevision == oldParentRevision) {
        ui->io->printMessage("No rebase needed.");
        return;
    }
    
    try {
        ui->m_dbInteraction->lockCurrentChangeset();
    } catch (Db::ChangesetLockingError &e) {
        ui->io->reportError("Error while locking old changeset for rebase.");
        return;
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
        return;
    }
    Db::RevisionId newParentRevision = ui->m_dbInteraction->changesetParent(newChangeset);

    // Obtain modifications lists for three-way diff
    std::vector<Db::ObjectModificationResult> externModifications = ui->m_dbInteraction->revisionsDifference(
        oldParentRevision, newParentRevision);
    std::vector<Db::ObjectModificationResult> ourModifications = ui->m_dbInteraction->revisionsDifferenceChangeset(
        oldChangeset);

    // Sort modifications for to merge the lists
    using namespace boost::phoenix::arg_names;
    std::sort(externModifications.begin(), externModifications.end(),
        boost::phoenix::bind(&Rebase::objectModificationResultLess, this, arg1, arg2));
    std::sort(ourModifications.begin(), ourModifications.end(),
        boost::phoenix::bind(&Rebase::objectModificationResultLess, this, arg1, arg2));

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
        return;
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
        return;
    }
    while ((ite != externModifications.end()) && (ito != ourModifications.end())) {
        if (objectModificationResultLess(*ite, *ito)) {
            ofs << boost::apply_visitor(ExternModificationConverter(), *ite) << std::endl;
            ++ite;
        } else if (*ite == *ito) {
            ofs << boost::apply_visitor(BothModificationConverter(), *ite) << std::endl;
            ++ite;
            ++ito;
        } else {
            ofs << boost::apply_visitor(OurModificationConverter(), *ito) << std::endl;
            ++ito;
        }
    }
    while (ite != externModifications.end()) {
        ofs << boost::apply_visitor(ExternModificationConverter(), *ite) << std::endl;
        ++ite;
    }
    while (ito != ourModifications.end()) {
        ofs << boost::apply_visitor(OurModificationConverter(), *ito) << std::endl;
        ++ito;
    }

    ofs.close();

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
        return;
    }

    ui->nonInteractiveMode = true;
    std::string line;
    std::string parserLine;
    ui->m_parser->clearContextStack();
    unsigned int lineNumber = 0;
    while (!getline(ifs, line).eof()) {
        ++lineNumber;
        if (!parserLine.empty() && parserLine[0] == '#')
            continue;
        ui->m_parser->parseLine(parserLine);
        ui->m_parser->clearContextStack();
        if (ui->parsingFailed)
            break;
    }
    ui->nonInteractiveMode = false;
    if (ui->parsingFailed) {
        std::ostringstream ostr;
        ostr << "Parsing of resolved conflicts file failed on line " << lineNumber << ".";
        ui->io->reportError(ostr.str());
        ifs.close();
        remove(tempFile);
        ui->m_dbInteraction->abortChangeset();
        ui->m_dbInteraction->resumeChangeset(oldChangeset);
        try {
            ui->m_dbInteraction->unlockCurrentChangeset();
        } catch (Db::ChangesetLockingError &e) {
            ui->io->reportError("Error while unlocking old changeset after rebase failure.");
        }
        return;
    }
    ifs.close();
    remove(tempFile);

    // Delete old obsolete changeset and connect to the new one
    ui->m_dbInteraction->detachFromChangeset("Rebase in progress");
    ui->m_dbInteraction->resumeChangeset(oldChangeset);
    ui->m_dbInteraction->abortChangeset();
    ui->m_dbInteraction->resumeChangeset(newChangeset);
    try {
        ui->m_dbInteraction->unlockCurrentChangeset();
    } catch (Db::ChangesetLockingError &e) {
        ui->io->reportError("Error while unlocking new changeset after rebase.");
    }
    ui->io->printMessage("Rebase successful.");
}



bool Rebase::objectModificationResultLess(const Db::ObjectModificationResult &a, const Db::ObjectModificationResult &b)
{
    // TODO
    return true;
}

}
}
