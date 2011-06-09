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
#include <boost/variant.hpp>
#include <boost/noncopyable.hpp>
#include <boost/signals2/trackable.hpp>
#include "deska/db/Objects.h"
#include "deska/db/Api.h"


namespace Deska
{
namespace Cli
{

class Parser;
class ParserException;
class SignalsHandler;
class UserInterface;


/** @short Represents signal categoryEntered() from the parser. */
class ParserSignalCategoryEntered
{
public:

    /** @short Constructor for storing signal categoryEntered().
    *
    *   @param context Current parser context
    *   @param kind Kind name of object being entered
    *   @param object Kind instance of object being entered
    */
    ParserSignalCategoryEntered(const Db::ContextStack &context,
                                const Db::Identifier &kind, const Db::Identifier &object);

    /** @short Performs action, that is the signal connected with.
    *
    *   @param signalsHandler Pointer to the database
    */
    void apply(SignalsHandler *signalsHandler) const;

    /** @short Shows confirmation message for performin actions connected with the signal, when necessary.
    *
    *   @param signalsHandler Pointer to the signals handler for calling actions
    */
    bool confirm(SignalsHandler *signalsHandler) const;

private:

    /** Context stack, that was actual when signal was triggered. */
    Db::ContextStack pastContext;

    //@{
    /** Additional information needed to be stored for particular signals. */
    Db::Identifier kindName;
    Db::Identifier objectName;
    //@}
};



/** @short Represents signal categoryLeft() from the parser. */
class ParserSignalCategoryLeft
{
public:

    /** @short Constructor for storing signal categoryLeft().
    *
    *   @param context Current parser context
    */
    ParserSignalCategoryLeft();

    /** @short Performs action, that is the signal connected with.
    *
    *   @param signalsHandler Pointer to the database
    */
    void apply(SignalsHandler *signalsHandler) const;

    /** @short Shows confirmation message for performin actions connected with the signal, when necessary.
    *
    *   @param signalsHandler Pointer to the signals handler for calling actions
    */
    bool confirm(SignalsHandler *signalsHandler) const;
};



/** @short Represents signal setAttribute() from the parser. */
class ParserSignalSetAttribute
{
public:

    /** @short Constructor for storing signal setAttribute().
    *
    *   @param context Current parser context
    *   @param attribute Name of attribute being changed
    *   @param value Value to be set
    */
    ParserSignalSetAttribute(const Db::ContextStack &context,
                             const Db::Identifier &attribute, const Db::Value &value);

    /** @short Performs action, that is the signal connected with.
    *
    *   @param signalsHandler Pointer to the signals handler for calling actions
    */
    void apply(SignalsHandler *signalsHandler) const;

    /** @short Shows confirmation message for performin actions connected with the signal, when necessary.
    *
    *   @param signalsHandler Pointer to the signals handler for calling actions
    */
    bool confirm(SignalsHandler *signalsHandler) const;

private:

    /** Context stack, that was actual when signal was triggered. */
    Db::ContextStack pastContext;

    //@{
    /** Additional information needed to be stored for particular signals. */
    Db::Identifier attributeName;
    Db::Value setValue;
    //@}
};



/** @short Represents signal setAttribute() from the parser. */
class ParserSignalRemoveAttribute
{
public:

    /** @short Constructor for storing signal setAttribute().
    *
    *   @param context Current parser context
    *   @param attribute Name of attribute being changed
    */
    ParserSignalRemoveAttribute(const Db::ContextStack &context,
                                const Db::Identifier &attribute);

    /** @short Performs action, that is the signal connected with.
    *
    *   @param signalsHandler Pointer to the signals handler for calling actions
    */
    void apply(SignalsHandler *signalsHandler) const;

    /** @short Shows confirmation message for performin actions connected with the signal, when necessary.
    *
    *   @param signalsHandler Pointer to the signals handler for calling actions
    */
    bool confirm(SignalsHandler *signalsHandler) const;

private:

    /** Context stack, that was actual when signal was triggered. */
    Db::ContextStack pastContext;

    //@{
    /** Additional information needed to be stored for particular signals. */
    Db::Identifier attributeName;
    //@}
};



/** @short Represents signal functionShow() from the parser. */
class ParserSignalFunctionShow
{
public:
    
    /** @short Constructor for storing signal functionShow().
    *
    *   @param context Current parser context
    */
    ParserSignalFunctionShow(const Db::ContextStack &context);

    /** @short Performs action, that is the signal connected with.
    *
    *   @param signalsHandler Pointer to the signals handler for calling actions
    */
    void apply(SignalsHandler *signalsHandler) const;

    /** @short Shows confirmation message for performin actions connected with the signal, when necessary.
    *
    *   @param signalsHandler Pointer to the signals handler for calling actions
    */
    bool confirm(SignalsHandler *signalsHandler) const;

private:

    /** Context stack, that was actual when signal was triggered. */
    Db::ContextStack pastContext;
};



/** @short Represents signal functionDelete() from the parser. */
class ParserSignalFunctionDelete
{
public:
    
    /** @short Constructor for storing signal functionDelete().
    *
    *   @param context Current parser context 
    */
    ParserSignalFunctionDelete(const Db::ContextStack &context);

    /** @short Performs action, that is the signal connected with.
    *
    *   @param signalsHandler Pointer to the signals handler for calling actions
    */
    void apply(SignalsHandler *signalsHandler) const;

    /** @short Shows confirmation message for performin actions connected with the signal, when necessary.
    *
    *   @param signalsHandler Pointer to the signals handler for calling actions
    */
    bool confirm(SignalsHandler *signalsHandler) const;

private:

    /** Context stack, that was actual when signal was triggered. */
    Db::ContextStack pastContext;
};



/** @short Represents signal functionRename() from the parser. */
class ParserSignalFunctionRename
{
public:
    
    /** @short Constructor for storing signal functionDelete().
    *
    *   @param context Current parser context
    *   @param newName New name of the object in the context stack
    */
    ParserSignalFunctionRename(const Db::ContextStack &context, const Db::Identifier &newName);

    /** @short Performs action, that is the signal connected with.
    *
    *   @param signalsHandler Pointer to the signals handler for calling actions
    */
    void apply(SignalsHandler *signalsHandler) const;

    /** @short Shows confirmation message for performin actions connected with the signal, when necessary.
    *
    *   @param signalsHandler Pointer to the signals handler for calling actions
    */
    bool confirm(SignalsHandler *signalsHandler) const;

private:

    /** Context stack, that was actual when signal was triggered. */
    Db::ContextStack pastContext;

    //@{
    /** Additional information needed to be stored for particular signals. */
    Db::Identifier name;
    //@}
};



/** @short Represents one signal from the Parser. */
typedef boost::variant<ParserSignalCategoryEntered, ParserSignalCategoryLeft, ParserSignalSetAttribute,
                       ParserSignalRemoveAttribute, ParserSignalFunctionShow, ParserSignalFunctionDelete,
                       ParserSignalFunctionRename> ParserSignal;



/** @short Variant visitor for performing action for parser signals.
*
*   @see ParserSignalCategoryEntered::apply()
*/
class ApplyParserSignal: public boost::static_visitor<>
{
public:

    /** Constructor only stores pointer to SignalsHandler for calling action functions.
    *
    *   @param _signalsHandler Pointer to SignalsHandler, where the signals are stored
    */
    ApplyParserSignal(SignalsHandler *_signalsHandler);

    /** @short Function for calling apply() function in signals
    *
    *   @param parserSignal Signal, for which the action() function will be called
    */
    template <typename T>
    void operator()(const T &parserSignal) const;

private:

    /** Pointer to the signals handler for performing actions for signals. */
    SignalsHandler *signalsHandler;
};



/** @short Variant visitor for confirmation of each signal.
*
*   @see ParserSignalCategoryEntered::confirm()
*/
class ConfirmParserSignal: public boost::static_visitor<bool>
{
public:

    /** Constructor only stores pointer to SignalsHandler for calling confirm functions.
    *
    *   @param _signalsHandler Pointer to SignalsHandler, where the signals are stored
    */
    ConfirmParserSignal(SignalsHandler *_signalsHandler);

    /** @short Function for calling confirm() function in signals
    *
    *   @param parserSignal Signal, for which the confirm() function will be called
    */
    template <typename T>
    bool operator()(const T &parserSignal) const;

private:

    /** Pointer to the signals handler for performing actions for signals. */
    SignalsHandler *signalsHandler;
};



/** @short Class, that listens to all signals from the Parser and stores them for the purposes of the CLI.
*
*   Each signal is stored and SignalsHandler is waiting for signal parseError() or parsingFinished().
*   When parseError() signal is caught, SignalsHandler clears its signals stack and reports error to
*   the UserInterface. When parsingFinished() signal is caught, SignalsHandler goes through the whole signals
*   stack and calls appropriate actions in UserInterface.
*/
class SignalsHandler: public boost::noncopyable, public boost::signals2::trackable
{
public:

    /** @short Constructor only initializes pointers handler is working with.
    *   
    *   @param _parser Pointer to the Parser for connecting slots to signals
    *   @param _userInterface Pointer to the user interface class for reporting errors and calling actions
    *                         for signals.
    */
    SignalsHandler(Parser *_parser, UserInterface *_userInterface);

private:

    //@{
    /** @short Slots for signals from the parser. */
    void slotCategoryEntered(const Db::Identifier &kind, const Db::Identifier &name);
    void slotCategoryLeft();
    void slotSetAttribute(const Db::Identifier &attribute, const Db::Value &value);
    void slotRemoveAttribute(const Db::Identifier &attribute);
    void slotFunctionShow();
    void slotFunctionDelete();
    void slotFunctionRename(const Db::Identifier &newName);
    void slotParserError(const ParserException &error);
    void slotParsingFinished();
    //@}

    friend class ParserSignalCategoryEntered;
    friend class ParserSignalCategoryLeft;
    friend class ParserSignalSetAttribute;
    friend class ParserSignalRemoveAttribute;
    friend class ParserSignalFunctionShow;
    friend class ParserSignalFunctionDelete;
    friend class ParserSignalFunctionRename;

    /** Here are all signals from the parser stored. */
    std::vector<ParserSignal> signalsStack;

    /** The context is held there. */
    Db::ContextStack contextStack;

    /** Pointer to the parser for listening to the signals. */
    Parser *m_parser;
    /** Pointer to the user interface for reporting errors and and calling actions for signals. */
    UserInterface *userInterface;

    /** Flag that determines whether user will be asked for confirmation of creating new object or not. */
    bool autoCreate;

};



}
}

#endif  // DESKA_PARSER_SIGNALS_H
