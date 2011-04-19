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
#ifndef HAVE_THE_LIBEBT_LIBEBT_CONTEXT_HH
#define HAVE_THE_LIBEBT_LIBEBT_CONTEXT_HH 1

#include <libebt/libebt_util.hh>
#include <libebt/libebt_order.hh>

#include <deque>
#include <string>
#include <algorithm>
#include <iterator>

/** \file
 * Definitions for the libebt::BacktraceContextHolder and
 * libebt::BacktraceContext class templates.
 */

namespace libebt
{
    /**
     * Holds the backtrace context for a given tag. This can be overridden
     * for a given tag type (for example, if thread safety is needed) by
     * specialising the template. See \link Threads Threads \endlink for
     * details.
     */
    template <typename Tag_, typename StringType_ = std::string>
    struct BacktraceContextHolder
    {
        /**
         * Our list type.
         */
        typedef std::deque<StringType_> ListType;

        /**
         * A pointer to our list type.
         */
        typedef ListType * const ListPtrType;

        /**
         * Return the backtrace context list.
         */
        static inline ListPtrType get_list()
        {
            static ListType the_list;
            return &the_list;
        }
    };

    /**
     * Represents an item on the backtrace context stack. When a
     * BacktraceContext is instantiated, a context stack entry is acquired. When
     * the BacktraceContext is destroyed, the stack entry is removed.
     *
     * The Tag_ template parameter should be an empty struct. It is used for
     * backtrace grouping.
     *
     * The StringType_ template parameter specifies the type of string used for
     * messages.
     */
    template <typename Tag_, typename StringType_ = std::string>
    class BacktraceContext
    {
        private:
            typedef BacktraceContextHolder<Tag_, StringType_> HolderType;

            /**
             * Do not copy.
             */
            BacktraceContext(const BacktraceContext & other)
            {
            }

            /**
             * Our item in the backtrace stack.
             */
            typename HolderType::ListType::iterator _our_item;

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
             * Constructor.
             *
             * \param context The context message.
             */
            inline BacktraceContext(const StringType_ & context) :
                _our_item(HolderType::get_list()->insert(HolderType::get_list()->end(), context))
            {
            }

            /**
             * Destructor.
             */
            inline ~BacktraceContext()
            {
                HolderType::get_list()->pop_back();
            }

            /**
             * Change context. Note that this will work even if our context
             * item isn't top of the context stack (for example, if there are
             * two context objects in scope).
             *
             * \param context The new context message.
             */
            inline void change_context(const StringType_ & context)
            {
                *_our_item = context;
            }

            /**
             * Are we empty?
             */
            static bool empty()
            {
                return HolderType::get_list()->empty();
            }

            /**
             * Create and return a backtrace string by joining together context
             * information.
             *
             * \param item_terminator A terminator which is appended to every
             * context item. Defaults to a newline.
             *
             * \param order If newest_first, start with the top of the context
             * stack, rather than the bottom. Defaults to oldest_first.
             */
            static StringType_ backtrace(
                    const StringType_ & item_terminator = newline_string<StringType_>(),
                    const Order order = oldest_first);

            /**
             * Copy backtrace items to the insert iterator I_.
             */
            template <typename I_>
            static void copy_backtrace_items_to(I_ i)
            {
                std::copy(HolderType::get_list()->begin(), HolderType::get_list()->end(), i);
            }
    };
}


template <typename Tag_, typename StringType_>
StringType_
libebt::BacktraceContext<Tag_, StringType_>::backtrace(
        const StringType_ & item_terminator,
        const Order order)
{
    StringType_ result;
    typename HolderType::ListType::const_iterator p(HolderType::get_list()->begin()),
             end(HolderType::get_list()->end());
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
