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

#ifndef DESKA_PARSER_SIGNALS_H
#define DESKA_PARSER_SIGNALS_H


#include <vector>
#include "deska/cli/Parser.h"
#include "deska/cli/Exceptions.h"
#include "deska/db/Objects.h"


namespace Deska
{
namespace Cli
{


typedef enum {
    /** @short Signal type for categoryEntered() */
    SIGNAL_TYPE_CATEGORY_ENTERED,
    /** @short Signal type for categoryLeft() */
    SIGNAL_TYPE_CATEGORY_LEFT,
    /** @short Signal type for setAttribute() */
    SIGNAL_TYPE_SET_ATTRIBUTE,
    /** @short Signal type for functionShow() */
    SIGNAL_TYPE_FUNCTION_SHOW,
    /** @short Signal type for functionDelete() */
    SIGNAL_TYPE_FUNCTION_DELETE,
    /** @short Signal type for parseError() */
    SIGNAL_TYPE_PARSE_ERROR,
} SignalType;



/** @short represents one signal from the Parser. */
class ParserSignal
{
public:

    /** @short Constructor for storing signal categoryEntered().
    *
    *   @param type Signal type
    *   @param context Current parser context
    *   @param kind Kind name of object being entered
    *   @param object Kind instance of object being entered
    */
    ParserSignal(SignalType type, const std::vector<ContextStackItem> &context,
                 const Db::Identifier &kind, const Db::Identifier &object);
    /** @short Constructor for storing signal categoryLeft() or functionShow() or functionDelete().
    *
    *   @param type Signal type
    *   @param context Current parser context
    */
    ParserSignal(SignalType type, const std::vector<ContextStackItem> &context);
    /** @short Constructor for storing signal setAttribute().
    *
    *   @param type Signal type
    *   @param context Current parser context
    *   @param attribute Name of attribute being changed
    *   @param value Value to be set
    */
    ParserSignal(SignalType type, const std::vector<ContextStackItem> &context,
                 const Db::Identifier &attribute, const Db::Value &value);
    /** @short Constructor for storing signal parseError().
    *
    *   @param type Signal type
    *   @param context Current parser context
    *   @param error String dump of parse error
    */
    ParserSignal(SignalType type, const std::vector<ContextStackItem> &context,
                 const std::string &error);

private:

    /** Signal type. @see SignalType */
    SignalType signalType;

    /** Context stack, that was actual when signal was triggered. */
    std::vector<ContextStackItem> contextStack;

    std::string parseError;

    //@{
    /** Additional information needed to be stored for particular signals.
    *   Which members will be initialised depends on signal type.
    */
    Db::Identifier kindName;
    Db::Identifier objectName;
    Db::Identifier attributeName;
    Db::Value setValue;
    //@}
};



/** @short Class, that listens to all signals from the Parser and stores them for the purposes of the CLI. */
class SignalsHandler
{
public:

    SignalsHandler(Parser *parser);

private:

    friend class ParserSignal;

    void slotCategoryEntered(const Db::Identifier &kind, const Db::Identifier &name);
    void slotCategoryLeft();
    void slotSetAttribute(const Db::Identifier &attribute, const Db::Value &value);
    void slotFunctionShow();
    void slotFunctionDelete();
    void slotParserError(const ParserException &error);
    void slotParsingFinished();

    std::vector<ParserSignal> signalsStack;

    Parser *m_parser;

};


}
}

#endif  // DESKA_PARSER_SIGNALS_H
