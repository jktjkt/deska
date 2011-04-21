/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005 Ciaran McCreesh
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the library nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/// Include guard.
#ifndef HAVE_THE_LIBEBT_LIBEBT_BACKTRACEABLE_HH
#define HAVE_THE_LIBEBT_LIBEBT_BACKTRACEABLE_HH 1

#include <libebt/libebt_util.hh>
#include <libebt/libebt_order.hh>
#include <libebt/libebt_extra_role.hh>
#include <libebt/libebt_context.hh>

#include <list>
#include <string>
#include <algorithm>
#include <iterator>

/** \file
 * Definitions for the libebt::Backtraceable class template, and the
 * deprecated libebt::Backtracable alias.
 */
namespace libebt
{
    /**
     * Exception base class which includes backtrace information.
     *
     * The Tag_ template parameter should be an empty struct. It is used for
     * backtrace grouping.
     *
     * The StringType_ template parameter specifies the type of string used for
     * messages.
     */
    template <typename Tag_, typename StringType_ = std::string>
    class Backtraceable
    {
        private:
            /**
             * Our backtrace list type. Internal use only.
             */
            typedef std::deque<StringType_> ListType;

            ListType _backtrace;

        protected:
            /**
             * Constructor. Grabs backtrace information from the stack and
             * remembers it.
             */
            Backtraceable() throw () :
                _backtrace()
            {
                BacktraceContext<Tag_, StringType_>::copy_backtrace_items_to(
                        std::back_inserter(_backtrace));
            }

            /**
             * Copy constructor.
             */
            Backtraceable(const Backtraceable & other) throw () :
                _backtrace(other._backtrace.begin(), other._backtrace.end())
            {
            }

            /**
             * Constructor, which pulls in items from another Backtraceable
             * (possibly with a different tag) instance, possibly prefixing
             * or suffixing each item or adding a joiner item. Usually used
             * for creating a new exception from a library-thrown exception.
             *
             * \param other The other exception.
             *
             * \param extra The string to be used in the role described by
             * the extra_role parameter. May be blank, in which case no
             * extra_role is ignored and no changes are made.
             *
             * \param extra_role If extra is non-blank, describes how it is
             * used. May be joiner_item, in which case an extra item is created
             * in between the items from other and our current context with the
             * text of extra parameter, or prefix_each_item, in which case
             * each pulled in item is prefixed with the value of extra,
             * or suffix_each_item (idem, but suffix).
             */
            template <typename OtherTag_>
            Backtraceable(const Backtraceable<OtherTag_, StringType_> & other,
                    const StringType_ & extra = StringType_(),
                    const ExtraRole extra_role = joiner_item) throw ();

            /**
             * Destructor.
             */
            virtual ~Backtraceable() throw ()
            {
            }

        public:
            /**
             * Provide access to our Tag_ template parameter type.
             */
            typedef Tag_ Tag;

            /**
             * Provide access to our StringType_ template parameter type.
             */
            typedef StringType_ StringType;

            /**
             * String representation of our backtrace.
             *
             * \param item_terminator A terminator that is appended to every
             * context item. Defaults to a newline.
             *
             * \param order If newest_first, display the top item in the stack
             * first. Defaults to oldest_first.
             */
            StringType_ backtrace(
                    const StringType_ & item_terminator = newline_string<StringType_>(),
                    const Order order = oldest_first) const;

            /**
             * Copy backtrace items to the insert iterator I_.
             */
            template <typename I_>
            void copy_backtrace_items_to(I_ i) const
            {
                std::copy(_backtrace.begin(), _backtrace.end(), i);
            }

            /**
             * Are we empty?
             */
            bool empty() const
            {
                return _backtrace.empty();
            }
    };

    /**
     * Deprecated wrapper for the Backtraceable class, for backwards
     * compatibility.
     *
     * \deprecated Use Backtraceable instead.
     */
    template <typename Tag_, typename StringType_ = std::string>
    class LIBEBT_DEPRECATED Backtracable : public Backtraceable<Tag_, StringType_>
    {
        protected:
            /**
             * Constructor.
             *
             * \deprecated Use Backtraceable::Backtraceable() instead.
             */
            Backtracable() throw () LIBEBT_DEPRECATED ;

            /**
             * Copy constructor.
             *
             * \deprecated Use Backtraceable::Backtraceable(other) instead.
             */
            Backtracable(const Backtracable & other) throw () LIBEBT_DEPRECATED ;

            /**
             * Destructor.
             *
             * \deprecated Use Backtraceable::~Backtraceable() instead.
             */
            virtual ~Backtracable() throw () LIBEBT_DEPRECATED ;
    };

    template <typename Tag_, typename StringType_>
    Backtracable<Tag_, StringType_>::Backtracable() throw () :
        Backtraceable<Tag_, StringType_>()
    {
    }

    template <typename Tag_, typename StringType_>
    Backtracable<Tag_, StringType_>::Backtracable(const Backtracable & other) throw () :
        Backtraceable<Tag_, StringType_>(other)
    {
    }

    template <typename Tag_, typename StringType_>
    Backtracable<Tag_, StringType_>::~Backtracable() throw ()
    {
    }
}

template <typename Tag_, typename StringType_>
template <typename OtherTag_>
libebt::Backtraceable<Tag_, StringType_>::Backtraceable(
        const Backtraceable<OtherTag_, StringType_> & other,
        const StringType_ & extra,
        const ExtraRole extra_role) throw () :
    _backtrace()
{
    other.copy_backtrace_items_to(std::back_inserter(_backtrace));

    do
    {
        if (extra.empty())
            break;

        switch (extra_role)
        {
            case joiner_item:
                _backtrace.push_front(extra);
                break;

            case prefix_each_item:
            case suffix_each_item:
                {
                    typename ListType::iterator p(_backtrace.begin()),
                             end(_backtrace.end());
                    for ( ; p != end ; ++p)
                        *p = (prefix_each_item == extra_role) ?
                            (extra + *p) : (*p + extra);
                }
                break;
        }
    } while (false);

    BacktraceContext<Tag_, StringType_>::copy_backtrace_items_to(
            std::inserter(_backtrace, _backtrace.begin()));
}


template <typename Tag_, typename StringType_>
StringType_
libebt::Backtraceable<Tag_, StringType_>::backtrace(
        const StringType_ & item_terminator,
        const Order order) const
{
    StringType_ result;
    typename ListType::const_iterator p(_backtrace.begin()), end(_backtrace.end());
    for ( ; p != end ; ++p)
    {
        if (newest_first == order)
            result = *p + item_terminator + result;
        else
            result = result + *p + item_terminator;
    }
    return result;
}

#endif
