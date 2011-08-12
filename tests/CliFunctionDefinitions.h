/*
* Copyright (C) 2011 Tomáš Hubík <hubik.tomas@gmail.com>
* Copyright (C) 2011 Jan Kundrát <kundratj@fzu.cz>
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

/** @file
 * @short Definitions of all the methods that are needed for CLI testing
 * */

typedef std::map<std::string, std::string> map_string_string;
typedef std::vector<std::pair<std::string, std::string> > vect_pair_str_str;
typedef std::vector<std::pair<Deska::Cli::AttributeDefinition, Deska::Db::Identifier> > vect_pair_attrdef_ident;
typedef std::pair<std::string, bool> pair_string_bool;

FORWARD_1(reportError, ReportError, std::string);
FORWARD_1(printMessage, PrintMessage, std::string);
FORWARD_2(printHelp, PrintHelp, map_string_string, map_string_string);
FORWARD_2(printHelpCommand, PrintHelpCommand, std::string, std::string);
FORWARD_2(printHelpKeyword, PrintHelpKeyword, std::string, std::string);
FORWARD_1(printHelpShowKinds, PrintHelpShowKinds, std::vector<std::string>);
FORWARD_1_RETURN(confirmDeletion, ConfirmDeletion, bool, boolean, Deska::Cli::ObjectDefinition);
FORWARD_1_RETURN(confirmCreation, ConfirmCreation, bool, boolean, Deska::Cli::ObjectDefinition);
FORWARD_1_RETURN(confirmRestoration, ConfirmRestoration, bool, boolean, Deska::Cli::ObjectDefinition);
FORWARD_1_RETURN(chooseChangeset, ChooseChangeset, int, integer, std::vector<Deska::Db::PendingChangeset>);
FORWARD_1_RETURN(readLine, ReadLine, pair_string_bool, strbool, std::string);
FORWARD_3(printHelpKind, PrintHelpKind, std::string, vect_pair_str_str, std::vector<std::string>);
FORWARD_0_RETURN(askForCommitMessage, AskForCommitMessage, std::string, str1);
FORWARD_0_RETURN(askForDetachMessage, AskForDetachMessage, std::string, str1);
FORWARD_3_OSTREAM(printAttributes, PrintAttributes, std::vector<Deska::Cli::AttributeDefinition>, int);
FORWARD_3_OSTREAM(printAttributesWithOrigin, PrintAttributesWithOrigin, vect_pair_attrdef_ident, int);
FORWARD_4_OSTREAM(printObjects, PrintObjects, std::vector<Deska::Cli::ObjectDefinition>, int, bool);
FORWARD_3_OSTREAM(printAttribute, PrintAttribute, Deska::Cli::AttributeDefinition, int);
FORWARD_4_OSTREAM(printAttributeWithOrigin, PrintAttributeWithOrigin, Deska::Cli::AttributeDefinition, Deska::Db::Identifier, int);
FORWARD_4_OSTREAM(printObject, PrintObject, Deska::Cli::ObjectDefinition, int, bool);
FORWARD_2_RAW_ARGS(printEnd, PrintEnd, int, std::ostream &);
FORWARD_1(printRevisions, PrintRevisions, std::vector<Deska::Db::RevisionMetadata>);
FORWARD_1(printDiff, PrintDiff, std::vector<Deska::Db::ObjectModificationResult>);
FORWARD_1(addCommandCompletion, AddCommandCompletion, std::string);

#undef FORWARD_0_RETURN
#undef FORWARD_1
#undef FORWARD_1_RETURN
#undef FORWARD_2
#undef FORWARD_2_RAW_ARGS
#undef FORWARD_3
#undef FORWARD_3_OSTREAM
#undef FORWARD_4_OSTREAM
