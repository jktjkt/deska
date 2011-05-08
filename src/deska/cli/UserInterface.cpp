/*
* Copyright (C) 2011 Jan Kundrát <kundratj@fzu.cz>
* Copyright (C) 2011 Tomáż Hubík <hubik.tomas@gmail.com>
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

#include <sstream>

#include "UserInterface.h"


namespace Deska
{
namespace Cli
{



UserInterface::UserInterface(std::ostream &outStream, std::ostream &errStream, std::istream &inStream):
    out(outStream.rdbuf()), err(errStream.rdbuf()), in(inStream.rdbuf())
{
}



void UserInterface::applyCategoryEntered(const std::vector<Db::ObjectDefinition> &context,
                                         const Db::Identifier &kind, const Db::Identifier &object)
{
    std::vector<Db::ObjectDefinition> objects;
    objects = dbInteraction->getAllObjects();
    Db::ObjectDefinition category(kind, object);

    if (std::find(objects.begin(), objects.end(), category) == objects.end()) {
        dbInteraction->createObject(category);
    }
}



void UserInterface::applySetAttribute(const std::vector<Db::ObjectDefinition> &context,
                                      const Db::Identifier &attribute, const Db::Value &value)
{
    dbInteraction->setAttribute(context.back(), Db::AttributeDefinition(attribute, value));
}



void UserInterface::applyFunctionShow(const std::vector<Db::ObjectDefinition> &context)
{
    printAttributes(context.back());
}



void UserInterface::applyFunctionDelete(const std::vector<Db::ObjectDefinition> &context)
{
    dbInteraction->deleteObject(context.back());
}



bool UserInterface::confirmCategoryEntered(const std::vector<Db::ObjectDefinition> &context,
                                           const Db::Identifier &kind, const Db::Identifier &object)
{
    // We're entering into some context, so we should check whether the object in question exists, and if it does not,
    // ask the user whether to create it.
    std::vector<Db::ObjectDefinition> objects;
    objects = dbInteraction->getAllObjects();
    Db::ObjectDefinition category(kind, object);

    if (std::find(objects.begin(), objects.end(), category) != objects.end()) {
        // Object exists
        return true;
    }

    // Object does not exist -> ask the user here
    std::ostringstream ss;
    ss << category << " does not exist. Create?";
    return askForConfirmation(ss.str());    
}



bool UserInterface::confirmSetAttribute(const std::vector<Db::ObjectDefinition> &context,
                                        const Db::Identifier &attribute, const Db::Value &value)
{
    return true;
}



bool UserInterface::confirmFunctionShow(const std::vector<Db::ObjectDefinition> &context)
{
    return true;
}



bool UserInterface::confirmFunctionDelete(const std::vector<Db::ObjectDefinition> &context)
{
    std::ostringstream ss;
    ss << "Are you sure you want to delete object " << context.back() << "?";
    return askForConfirmation(ss.str());
}



void UserInterface::reportError(const std::string &errorMessage)
{
    err << errorMessage;
}



bool UserInterface::askForConfirmation(const std::string &prompt)
{
    out << prompt << " ";
    std::string answer;
    in >> answer;
    out << std::endl;
    boost::algorithm::to_lower(answer);
    return answer == "yes" || answer == "y";
}



void UserInterface::dumpDbContents()
{
    std::vector<Db::ObjectDefinition> objects;
    objects = dbInteraction->getAllObjects();
    for (std::vector<Db::ObjectDefinition>::iterator it = objects.begin(); it != objects.end(); ++it) {
        out << *it << std::endl;
        std::vector<Db::AttributeDefinition> attributes;
        attributes = dbInteraction->getAllAttributes(*it);
        for (std::vector<Db::AttributeDefinition>::iterator ita = attributes.begin(); ita != attributes.end(); ++ita) {
            out << "    " << *ita << std::endl;
        }
        out << "end" << std::endl;
        out << std::endl;
    }
}



void UserInterface::printAttributes(const Db::ObjectDefinition &object)
{
    std::vector<Db::AttributeDefinition> attributes;
    attributes = dbInteraction->getAllAttributes(object);
    for (std::vector<Db::AttributeDefinition>::iterator it = attributes.begin(); it != attributes.end(); ++it) {
        out << *it << std::endl;
    }
}



}
}
