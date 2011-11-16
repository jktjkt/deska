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
#include <boost/algorithm/string/trim.hpp>
#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/phoenix_core.hpp>

#include "CliCommands_Log.h"
#include "CliCommands_Rebase.h"
#include "UserInterface.h"
#include "UserInterfaceIOBase.h"
#include "DbInteraction.h"
#include "Exceptions.h"
#include "Parser.h"
#include "deska/db/JsonApi.h"

namespace Deska {
namespace Cli {



/** @short Print a well-formatted representation of an attribute value */
std::string readableAttrPrinter(const std::string &prefixMessage, const Db::Value &v)
{
    std::ostringstream ss;
    if (v) {
        ss << prefixMessage << " " << boost::apply_visitor(NonOptionalValuePrettyPrint(), *v);
    } else {
        ss << " (none)";
    }
    return ss.str();
}



ModificationType ModificationTypeGetter::operator()(const Db::CreateObjectModification &modification) const
{
    return OBJECT_MODIFICATION_TYPE_CREATE;
}



ModificationType ModificationTypeGetter::operator()(const Db::DeleteObjectModification &modification) const
{
    return OBJECT_MODIFICATION_TYPE_DELETE;
}



ModificationType ModificationTypeGetter::operator()(const Db::RenameObjectModification &modification) const
{
    return OBJECT_MODIFICATION_TYPE_RENAME;
}



ModificationType ModificationTypeGetter::operator()(const Db::SetAttributeModification &modification) const
{
    return OBJECT_MODIFICATION_TYPE_SETATTR;
}



bool ModificationComparatorLesss::operator()(const Db::CreateObjectModification &a,
                                             const Db::CreateObjectModification &b) const
{
    if (a.kindName < b.kindName) {
        return true;
    } else if (a.kindName > b.kindName) {
        return false;
    } else {
        return (a.objectName <= b.objectName);
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
        return (a.objectName <= b.objectName);
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
        return (a.newObjectName != b.newObjectName);
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
    } else if (a.attributeName < b.attributeName) {
        return true;
    } else if (a.attributeName > b.attributeName) {
        return false;
    } else {
        return (a.attributeData != b.attributeData);
    }
}



template <typename MA, typename MB>
bool ModificationComparatorLesss::operator()(const MA &a, const MB &b) const
{
    throw std::invalid_argument("Deska::Cli::ModificationComparatorLesss::operator(): Comparator called to two different types of ObjectModificationResult.");
    return false;
}



/** @short Visitor for printing object modifications. */
struct ModificationBackuper: public boost::static_visitor<std::string> {
    //@{
    /** @short Function for converting single object modification to string for purposes of backup.
    *
    *   @param modification Instance of modifications from Db::ObjectModification variant.
    *   @return Parser readable string representation of the modification.
    */
    std::string operator()(const Db::CreateObjectModification &modification) const;
    std::string operator()(const Db::DeleteObjectModification &modification) const;
    std::string operator()(const Db::RenameObjectModification &modification) const;
    std::string operator()(const Db::SetAttributeModification &modification) const;
    //@}
};



std::string ModificationBackuper::operator()(const Db::CreateObjectModification &modification) const
{
    std::ostringstream ostr;
    ostr << "create " << modification.kindName << " " << modification.objectName;
    return ostr.str();
}



std::string ModificationBackuper::operator()(const Db::DeleteObjectModification &modification) const
{
    std::ostringstream ostr;
    ostr << "delete " << modification.kindName << " " << modification.objectName;
    return ostr.str();
}



std::string ModificationBackuper::operator()(const Db::RenameObjectModification &modification) const
{
    std::ostringstream ostr;
    ostr << "rename " << modification.kindName << " " << modification.oldObjectName << " " << modification.newObjectName;
    return ostr.str();
}



std::string ModificationBackuper::operator()(const Db::SetAttributeModification &modification) const
{
    std::ostringstream ostr;
    ostr << modification.kindName << " " << modification.objectName << " ";
    if (modification.attributeData)
        ostr << modification.attributeName << " " << boost::apply_visitor(NonOptionalValuePrettyPrint(), *(modification.attributeData));
    else
        ostr << "no " << modification.attributeName;
    return ostr.str();
}



Command::Command(UserInterface *userInterface): ui(userInterface)
{
}



Command::~Command()
{
}



std::vector<std::string> Command::completionPatterns()
{
    return complPatterns;
}



std::string Command::name()
{
    return cmdName;
}



std::string Command::usage()
{
    return cmdUsage;
}



std::vector<std::string> Command::extractParams(const std::string &params)
{
    std::vector<std::string> paramsList;
    boost::char_separator<char> separators(" \t");
    boost::tokenizer<boost::char_separator<char> > tokenizer(params, separators);
    std::string token;
    
    for (boost::tokenizer<boost::char_separator<char> >::const_iterator it = tokenizer.begin();
         it != tokenizer.end(); ++it) {
        token = *it;
        boost::algorithm::trim(token);
        paramsList.push_back(token);
    }

    return paramsList;
}



Start::Start(UserInterface *userInterface): Command(userInterface)
{
    cmdName = "start";
    cmdUsage = "Starts new changeset.";
    complPatterns.push_back("start");
}



Start::~Start()
{
}



void Start::operator()(const std::string &params)
{
    if (!params.empty()) {
        ui->io->reportError("Error: No parameters expected for command " + cmdName + ".");
        return;
    }
    if (ui->currentChangeset) {
        std::ostringstream ostr;
        ostr << "Error: You are already in the changeset " << *(ui->currentChangeset) << "!";
        ui->io->reportError(ostr.str());
        return;
    }
    ui->currentChangeset = ui->m_dbInteraction->createNewChangeset();
    std::ostringstream ostr;
    ostr << "Changeset " << *(ui->currentChangeset) << " started.";
    ui->io->printMessage(ostr.str());
}



Resume::Resume(UserInterface *userInterface): Command(userInterface)
{
    cmdName = "resume";
    cmdUsage = "Displays list of pending changesets with ability to connect to one.";
    complPatterns.push_back("resume");
}



Resume::~Resume()
{
}



void Resume::operator()(const std::string &params)
{
    if (ui->currentChangeset) {
        std::ostringstream ostr;
        ostr << "Error: You are already in the changeset " << *(ui->currentChangeset) << "!";
        ui->io->reportError(ostr.str());
        return;
    }

    if (!params.empty()) {
        
        std::vector<std::string> paramsList = extractParams(params);
        if (paramsList.size() != 1) {
            ui->io->reportError("Invalid number of parameters entered!");
            return;
        }

        boost::optional<Db::TemporaryChangesetId> tmpId;
        try {
            tmpId = Db::TemporaryChangesetId::fromString(paramsList[0]);
        } catch (std::runtime_error &e) {
            std::ostringstream ss;
            ss << "Invalid parameters: " << e.what();
            ui->io->reportError(ss.str());
            return;
        }
        try {
            ui->m_dbInteraction->resumeChangeset(*tmpId);
            ui->currentChangeset = *tmpId;
            std::ostringstream ostr;
            ostr << "Changeset " << *(ui->currentChangeset) << " resumed.";
            ui->io->printMessage(ostr.str());
        } catch (Db::ServerError &e) {
            std::ostringstream ostr;
            ostr << "Error while resuming changeset: " << e.what();
            ui->io->reportError(ostr.str());
        }
        return;
    }
    
    // Print list of pending changesets, so user can choose one
    std::vector<Db::PendingChangeset> pendingChangesets = ui->m_dbInteraction->allPendingChangesets();
    int choice = ui->io->chooseChangeset(pendingChangesets);

    if (choice >= 0) {
        // Some changeset was choosen
        ui->m_dbInteraction->resumeChangeset(pendingChangesets[choice].revision);
        ui->currentChangeset = pendingChangesets[choice].revision;
        std::ostringstream ostr;
        ostr << "Changeset " << *(ui->currentChangeset) << " resumed.";
        ui->io->printMessage(ostr.str());
    }
}



Commit::Commit(UserInterface *userInterface): Command(userInterface)
{
    cmdName = "commit";
    cmdUsage = "Displays promt for commit message and commits current changeset.";
    complPatterns.push_back("commit");
}



Commit::~Commit()
{
}



void Commit::operator()(const std::string &params)
{
    if (!(ui->currentChangeset)) {
        ui->io->reportError("Error: You are not in any changeset!");
        return;
    }
    std::string commitMessage;
    if (!params.empty()) {
        commitMessage = params;
    } else {
        commitMessage = ui->io->askForCommitMessage();
    }
    try {
        ui->m_dbInteraction->commitChangeset(commitMessage);
        std::ostringstream ostr;
        ostr << "Changeset " << *(ui->currentChangeset) << " commited.";
        ui->io->printMessage(ostr.str());
        ui->currentChangeset = boost::optional<Db::TemporaryChangesetId>();
        ui->m_parser->clearContextStack();
    } catch (Db::ConstraintError &e) {
        std::ostringstream ostr;
        ostr << "Commit failed due to constraint violation: " << e.what();
        ui->io->reportError(ostr.str());
    } catch (Db::ObsoleteParentError &e) {
        ui->io->reportError("Changeset parent obsolete. Use \"rebase\" and try again.");
    }
}



Detach::Detach(UserInterface *userInterface): Command(userInterface)
{
    cmdName = "detach";
    cmdUsage = "Displays promt for detach message and detaches from current changeset.";
    complPatterns.push_back("detach");
}



Detach::~Detach()
{
}



void Detach::operator()(const std::string &params)
{
    if (!(ui->currentChangeset)) {
        ui->io->reportError("Error: You are not in any changeset!");
        return;
    }
    std::string detachMessage;
    if (!params.empty()) {
        detachMessage = params;
    } else {
        detachMessage = ui->io->askForDetachMessage();
    }
    ui->m_dbInteraction->detachFromChangeset(detachMessage);
    std::ostringstream ostr;
    ostr << "Changeset " << *(ui->currentChangeset) << " detached.";
    ui->io->printMessage(ostr.str());
    ui->currentChangeset = boost::optional<Db::TemporaryChangesetId>();
    ui->m_parser->clearContextStack();
}



Abort::Abort(UserInterface *userInterface): Command(userInterface)
{
    cmdName = "abort";
    cmdUsage = "Aborts current changeset.";
    complPatterns.push_back("abort");
}



Abort::~Abort()
{
}



void Abort::operator()(const std::string &params)
{
    if (!params.empty()) {
        ui->io->reportError("Error: No parameters expected for command " + cmdName + ".");
        return;
    }
    if (!(ui->currentChangeset)) {
        ui->io->reportError("Error: You are not in any changeset!");
        return;
    }
    ui->m_dbInteraction->abortChangeset();
    std::ostringstream ostr;
    ostr << "Changeset " << *(ui->currentChangeset) << " aborted.";
    ui->io->printMessage(ostr.str());
    ui->currentChangeset = boost::optional<Db::TemporaryChangesetId>();
    ui->m_parser->clearContextStack();
}



Status::Status(UserInterface *userInterface): Command(userInterface)
{
    cmdName = "status";
    cmdUsage = "Shows if you are connected to any changeset or not.";
    complPatterns.push_back("status");
}



Status::~Status()
{
}



void Status::operator()(const std::string &params)
{
    if (!params.empty()) {
        ui->io->reportError("Error: No parameters expected for command " + cmdName + ".");
        return;
    }
    if (ui->currentChangeset) {
        std::ostringstream ostr;
        ostr << "You are connected to changeset " << *(ui->currentChangeset) << ".";
        ui->io->printMessage(ostr.str());
    } else {
        ui->io->printMessage("You are not connected to any changeset.");
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



void Diff::operator()(const std::string &params)
{
    if (params.empty()) {
        if (!ui->currentChangeset) {
            ui->io->reportError("Error: Wou have to be connected to a changeset to perform diff with its parent. Use commands \"start\" or \"resume\". Use \"help\" for more info.");
            return;
        }
        // Show diff with parent
        std::vector<Db::ObjectModificationResult> modifications = ui->m_dbInteraction->revisionsDifferenceChangeset(
            *(ui->currentChangeset));
        ui->io->printDiff(modifications);
        return;
    }

    std::vector<std::string> paramsList = extractParams(params);
    if (paramsList.size() > 3) {
        ui->io->reportError("Invalid number of parameters entered!");
        return;
    }

    if (paramsList.size() == 1) {
        // Create patch to current changeset
        if (!ui->currentChangeset) {
            ui->io->reportError("Error: Wou have to be connected to a changeset to perform diff with its parent. Use commands \"start\" or \"resume\". Use \"help\" for more info.");
            return;
        }
        std::ofstream ofs(paramsList[0].c_str());
        if (!ofs) {
            ui->io->reportError("Error while creating patch to file \"" + params + "\".");
            return;
        }
        std::vector<Db::ObjectModificationResult> modifications = ui->m_dbInteraction->revisionsDifferenceChangeset(
            *(ui->currentChangeset));
        using namespace boost::phoenix::arg_names;
        std::sort(modifications.begin(), modifications.end(),
            boost::phoenix::bind(&Diff::objectModificationResultLess, this, arg1, arg2));
        ModificationBackuper modificationBackuper;
        for (std::vector<Db::ObjectModificationResult>::iterator itm = modifications.begin(); itm != modifications.end(); ++itm) {
            ofs << boost::apply_visitor(modificationBackuper, *itm) << std::endl;
        }
        ofs.close();
        ui->io->printMessage("Patch successfully created into file \"" + paramsList[0] + "\".");

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
            return;
        }
        try {
            modifications = ui->m_dbInteraction->revisionsDifference(*revA, *revB);
        } catch (Db::RevisionRangeError &e) {
            ui->io->reportError("Revision range does not make a sense.");
            return;
        }
        if (paramsList.size() == 2) {
            // Print diff
            ui->io->printDiff(modifications);
        } else {
            // Create patch
            using namespace boost::phoenix::arg_names;
            std::sort(modifications.begin(), modifications.end(),
                boost::phoenix::bind(&Diff::objectModificationResultLess, this, arg1, arg2));
            std::ofstream ofs(paramsList[0].c_str());
            if (!ofs) {
                ui->io->reportError("Error while creating patch to file \"" + params + "\".");
                return;
            }
            ModificationBackuper modificationBackuper;
            for (std::vector<Db::ObjectModificationResult>::iterator itm = modifications.begin(); itm != modifications.end(); ++itm) {
                ofs << boost::apply_visitor(modificationBackuper, *itm) << std::endl;
            }
            ofs.close();
            ui->io->printMessage("Patch successfully created into file \"" + paramsList[0] + "\".");
        }
    }
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



Configdiff::Configdiff(UserInterface *userInterface): Command(userInterface)
{
    cmdName = "configdiff";
    cmdUsage = "Command for showing diff of current configuration and configuration generated with changes in current changeset using configuration generators. With parameter \"regenerate\" forces regeneration of the configuration.";
    complPatterns.push_back("configdiff regenerate");
}



Configdiff::~Configdiff()
{
}



void Configdiff::operator()(const std::string &params)
{
    if (!ui->currentChangeset) {
        ui->io->reportError("Error: Wou have to be connected to a changeset to perform diff of configuration. Use commands \"start\" or \"resume\". Use \"help\" for more info.");
        return;
    }

    Db::Api::ConfigGeneratingMode forceRegen = Db::Api::MAYBE_REGENERATE;
    if (params.empty()) {
        // nothing special
    } else if (params == "regenerate") {
        forceRegen = Db::Api::FORCE_REGENERATE;
    } else {
        ui->io->reportError("Error: Invalid parameter entered. Use \"help\" for more info.");
        return;
    }

    std::string diff = ui->m_dbInteraction->configDiff(forceRegen);
    if (diff.empty())
        ui->io->printMessage("No difference.");
    else
        ui->io->displayInPager(diff);
    return;
}



Exit::Exit(UserInterface *userInterface): Command(userInterface)
{
    cmdName = "exit";
    cmdUsage = "Exits the CLI.";
    complPatterns.push_back("exit");
    complPatterns.push_back("quit");
}



Exit::~Exit()
{
}



void Exit::operator()(const std::string &params)
{
    if (!params.empty()) {
        ui->io->reportError("Error: No parameters expected for command " + cmdName + ".");
        return;
    }
    ui->exitLoop = true;
}



Dump::Dump(UserInterface *userInterface): Command(userInterface)
{
    cmdName = "dump";
    cmdUsage = "Prints everything in the DB. Optional parameter is filename, where to save the dump.";
    complPatterns.push_back("dump %file");
}



Dump::~Dump()
{
}



void Dump::operator()(const std::string &params)
{
    try {
        ui->m_dbInteraction->freezeView();
        if (params.empty()) {
            BOOST_FOREACH(const Deska::Db::Identifier &kindName, ui->m_dbInteraction->topLevelKinds()) {
                BOOST_FOREACH(const Deska::Cli::ObjectDefinition &object, ui->m_dbInteraction->kindInstances(kindName)) {
                    ui->io->printObject(object, 0, false);
                    dumpObjectRecursive(object, 1);
                }
            }
            ui->m_dbInteraction->unFreezeView();
        } else {
            std::ofstream ofs(params.c_str());
            if (!ofs) {
                ui->io->reportError("Error while dumping DB to file \"" + params + "\".");
                ui->m_dbInteraction->unFreezeView();
                return;
            }
            BOOST_FOREACH(const Deska::Db::Identifier &kindName, ui->m_dbInteraction->topLevelKinds()) {
                BOOST_FOREACH(const Deska::Cli::ObjectDefinition &object, ui->m_dbInteraction->kindInstances(kindName)) {
                    ui->io->printObject(object, 0, false, ofs);
                    dumpObjectRecursive(object, 1, ofs);
                }
            }
            ofs.close();
            ui->m_dbInteraction->unFreezeView();
            ui->io->printMessage("DB successfully dumped into file \"" + params + "\".");
        }
    } catch (Db::FreezingError &e) {
        ui->io->reportError("Error while freezeing DB view for dump. Dumping failed.");
    }
}



void Dump::dumpObjectRecursive(const ObjectDefinition &object, unsigned int depth, std::ostream &out)
{
    std::vector<AttributeDefinition> attributes = ui->m_dbInteraction->allAttributes(object);
    ui->io->printAttributes(attributes, depth, out);
    std::vector<ObjectDefinition> nestedObjs = ui->m_dbInteraction->allNestedObjects(object);
    for (std::vector<ObjectDefinition>::iterator it = nestedObjs.begin(); it != nestedObjs.end(); ++it) {
        ui->io->printObject(*it, depth, false, out);
        dumpObjectRecursive(*it, depth + 1, out);
    }
    if (depth > 0)
        ui->io->printEnd(depth - 1, out);
}



Context::Context(UserInterface *userInterface): Command(userInterface)
{
    cmdName = "context";
    cmdUsage = "Shows current context with detail of filters. With parameter \"objects\" prints list of objects matched by current context with its filters.";
    complPatterns.push_back("context objects");
}



Context::~Context()
{
}



void Context::operator()(const std::string &params)
{
    if (params.empty()) {
        std::string contextDump = dumpContextStack(ui->m_parser->currentContextStack());
        if (contextDump.empty())
            ui->io->printMessage("No context.");
        else
            ui->io->printMessage(contextDump);
        return;
    }

    if (params != "objects") {
        ui->io->reportError("Error: Unknown parameter. Function context supports parameter \"objects\". Try \"help context\".");
        return;
    }
    
    std::vector<ObjectDefinition> objects =
        ui->m_dbInteraction->expandContextStack(ui->m_parser->currentContextStack());
    if (objects.empty())
        ui->io->printMessage("No objects matched by current context stack.");
    else
        ui->io->printObjects(objects, 0, true);
}



Batch::Batch(UserInterface *userInterface): Command(userInterface)
{
    cmdName = "batch";
    cmdUsage = "Executes commands from a file. Requires file name with commands as a parameter. Lines with # at the beginning are comments and will not be parsed.";
    complPatterns.push_back("batch %file");
}



Batch::~Batch()
{
}



void Batch::operator()(const std::string &params)
{
    if (params.empty()) {
        ui->io->reportError("Error: This command requires file name as a parameter.");
        return;
    }
    if (!ui->currentChangeset) {
        ui->io->reportError("Error: Wou have to be connected to a changeset to perform batched operations. Use commands \"start\" or \"resume\". Use \"help\" for more info.");
        return;
    }
    std::ifstream ifs(params.c_str());
    if (!ifs) {
        ui->io->reportError("Error while opening commands file \"" + params + "\".");
        return;
    }
    
    ui->nonInteractiveMode = true;
    std::string line;
    ui->m_parser->clearContextStack();
    unsigned int lineNumber = 0;
    try {
        ui->m_dbInteraction->lockCurrentChangeset();
        while (!getline(ifs, line).eof()) {
            ++lineNumber;
            if (!line.empty() && line[0] == '#')
                continue;
            ui->m_parser->parseLine(line);
            if (ui->parsingFailed)
                break;
        }
        ui->nonInteractiveMode = false;
        ui->m_dbInteraction->unlockCurrentChangeset();
        if (ui->parsingFailed) {
            std::ostringstream ostr;
            ostr << "Parsing of commands file failed on line " << lineNumber << ".";
            ui->io->reportError(ostr.str());
        } else {
            ui->io->printMessage("All commands successfully executed.");
        }
    } catch (Db::ChangesetLockingError &e) {
        ui->io->reportError("Error while locking changeset for batched oparations.");
    }
    ifs.close();
    ui->m_parser->clearContextStack();
}



Backup::Backup(UserInterface *userInterface): Command(userInterface)
{
    cmdName = "backup";
    cmdUsage = "Creates backup of whole DB to a file. Does not backup changesets. Requires file name where to store the backup as a parameter.";
    complPatterns.push_back("backup %file");
}



Backup::~Backup()
{
}



void Backup::operator()(const std::string &params)
{
    if (params.empty()) {
        ui->io->reportError("Error: This command requires file name as a parameter.");
        return;
    }

    std::ofstream ofs(params.c_str());
    if (!ofs) {
        ui->io->reportError("Error while backing up the DB to file \"" + params + "\".");
        ui->m_dbInteraction->unFreezeView();
        return;
    }

    if (ui->currentChangeset) {
        ui->io->printMessage("Notice: Backup function creates backup only for revisions, not changesets.");
    }
    
    std::vector<Db::RevisionMetadata> revisions = ui->m_dbInteraction->allRevisions();
    if (revisions.size() < 2)
        ui->io->reportError("Database empty. Nothing to back up.");
    // First revision is not a real revision, but head of the list, that is always present even with empty DB
    ModificationBackuper modificationBackuper;
    for (std::vector<Db::RevisionMetadata>::iterator it = revisions.begin() + 1; it != revisions.end(); ++it) {
        std::vector<Db::ObjectModificationResult> modifications = ui->m_dbInteraction->revisionsDifference((it - 1)->revision, it->revision);
        using namespace boost::phoenix::arg_names;
        std::sort(modifications.begin(), modifications.end(),
            boost::phoenix::bind(&Backup::objectModificationResultLess, this, arg1, arg2));
        for (std::vector<Db::ObjectModificationResult>::iterator itm = modifications.begin(); itm != modifications.end(); ++itm) {
            ofs << boost::apply_visitor(modificationBackuper, *itm) << std::endl;
        }
        ofs << "@commit to " << it->revision << std::endl;
        ofs << it->author << std::endl;
        ofs << it->commitMessage << std::endl;
        ofs << it->timestamp << std::endl;
        ofs << "#commit end" << std::endl;
    }

    ofs.close();
    ui->m_dbInteraction->unFreezeView();
    ui->io->printMessage("DB successfully backed up into file \"" + params + "\".");
}



bool Backup::objectModificationResultLess(const Db::ObjectModificationResult &a, const Db::ObjectModificationResult &b)
{
    ModificationTypeGetter modificationTypeGetter;
    if (boost::apply_visitor(modificationTypeGetter, a) > boost::apply_visitor(modificationTypeGetter, b)) {
        return false;
    } else {
        return true;
    }
}



Restore::Restore(UserInterface *userInterface): Command(userInterface)
{
    cmdName = "restore";
    cmdUsage = "Restores the DB from backup created by command \"backup\". Requires file name with backup as a parameter.";
    complPatterns.push_back("restore %file");
}



Restore::~Restore()
{
}



void Restore::operator()(const std::string &params)
{
    if (params.empty()) {
        ui->io->reportError("Error: This command requires file name as a parameter.");
        return;
    }
    if (ui->currentChangeset) {
        ui->io->reportError("Error: Wou must not be connected to a changeset to perform restore. Use \"help\" for more info.");
        return;
    }
    std::ifstream ifs(params.c_str());
    if (!ifs) {
        ui->io->reportError("Error while opening backup file \"" + params + "\".");
        return;
    }
    
    ui->nonInteractiveMode = true;
    std::string line;
    ui->m_parser->clearContextStack();
    unsigned int lineNumber = 0;
    bool restoreError = false;

    // FIXME: Batched operations for restore
    ui->currentChangeset = ui->m_dbInteraction->createNewChangeset();
    while (!getline(ifs, line).eof()) {
        ++lineNumber;
        // Comment
        if (line.empty() || line[0] == '#')
            continue;
        // Commit info
        if (!line.empty() && line[0] == '@') {
            std::string author;
            std::string message;
            std::string timestamp;
            getline(ifs, author);
            if (ifs.fail()) {
                restoreError = true;
                break;
            }
            ++lineNumber;
            getline(ifs, message);
            if (ifs.fail()) {
                restoreError = true;
                break;
            }
            ++lineNumber;
            getline(ifs, timestamp);
            if (ifs.fail()) {
                restoreError = true;
                break;
            }
            ++lineNumber;
            ui->m_dbInteraction->restoringCommit(message, author, boost::posix_time::time_from_string(timestamp));
        // Normal command
        } else {
            if (!ui->currentChangeset)
                ui->currentChangeset = ui->m_dbInteraction->createNewChangeset();
            ui->m_parser->parseLine(line);
        }
        if (ui->parsingFailed)
            break;
    }

    ui->nonInteractiveMode = false;
    if (ui->parsingFailed || restoreError) {
        std::ostringstream ostr;
        ostr << "Parsing of backup file failed on line " << lineNumber << ".";
        ui->io->reportError(ostr.str());
    } else {
        ui->io->printMessage("DB successfully restored.");
    }

    ifs.close();
    ui->m_parser->clearContextStack();
}



Help::Help(UserInterface *userInterface): Command(userInterface)
{
    cmdName = "help";
    cmdUsage = "Displays this list of commands with usages. Accepts parameter. For command name or parser keyword as parametr, usage is displayed, for kind name is displayed content of the kind and for word \"kinds\" are all defined kind names printed.";
    complPatterns.push_back("help kinds");
    // Help is not in commands map, because we are constructing it now.
    complPatterns.push_back("help help");
    for (UserInterface::CommandMap::iterator it = ui->commandsMap.begin(); it != ui->commandsMap.end(); ++it) {
        complPatterns.push_back("help " + it->first);
    }
    std::vector<std::string> kinds = ui->m_dbInteraction->kindNames();
    for (std::vector<std::string>::iterator it = kinds.begin(); it != kinds.end(); ++it) {
        complPatterns.push_back("help " + *it);
    }
    std::map<std::string, std::string> keywords = ui->m_parser->parserKeywordsUsage();
    for (std::map<std::string, std::string>::iterator it = keywords.begin(); it != keywords.end(); ++it) {
        complPatterns.push_back("help " + it->first);
    }
}



Help::~Help()
{
}



void Help::operator()(const std::string &params)
{
    if (!params.empty()) {
        if (params == "kinds") {
            ui->io->printHelpShowKinds(ui->m_dbInteraction->kindNames());
            return;
        }
        UserInterface::CommandMap::iterator itc = ui->commandsMap.find(params);
        if ( itc != ui->commandsMap.end()) {
            ui->io->printHelpCommand(params, itc->second->usage());
            return;
        }
        std::map<std::string, std::string> keywords = ui->m_parser->parserKeywordsUsage();
        std::map<std::string, std::string>::iterator itke = keywords.find(params);
        if ( itke != keywords.end()) {
            ui->io->printHelpKeyword(params, itke->second);
            return;
        }
        std::vector<std::string> kinds = ui->m_dbInteraction->kindNames();
        std::vector<std::string>::iterator itki = std::find(kinds.begin(), kinds.end(), params);
        if ( itki != kinds.end()) {
            ui->io->printHelpKind(params, ui->m_parser->parserKindsAttributes(params),
                                  ui->m_parser->parserKindsEmbeds(params));
            return;
        }
        ui->io->reportError("Error: No help entry for \"" + params + "\".");
        return;
    }
    std::map<std::string, std::string> cliCommands;
    for (UserInterface::CommandMap::iterator it = ui->commandsMap.begin(); it != ui->commandsMap.end(); ++it) {
        cliCommands[it->first] = it->second->usage();
    }
    ui->io->printHelp(cliCommands, ui->m_parser->parserKeywordsUsage());
}

}
}
