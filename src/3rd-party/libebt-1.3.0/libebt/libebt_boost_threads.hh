/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/// Include guards.
#ifndef HAVE_THE_LIBEBT_LIBEBT_BOOST_THREADS_HH
#define HAVE_THE_LIBEBT_LIBEBT_BOOST_THREADS_HH 1

#include <deque>
#include <string>

#include <boost/thread.hpp>

/** \file
 * Declarations for libebt::BoostThreadsBacktraceContextHolder template
 * class, a helper struct for setups using Boost.Threads.
 */

namespace libebt
{
    /**
     * Helper struct for libebt::BacktraceContextHolder when using
     * Boost.Threads.
     */
    template <typename Tag_, typename StringType_ = std::string>
    struct BoostThreadsBacktraceContextHolder
    {
        /**
         * Our list type.
         */
        typedef std::deque<StringType_> ListType;

        /**
         * A pointer to our list type.
         */
        typedef ListType * ListPtrType;

        /**
         * Get the thread-specific list.
         */
        static ListPtrType get_list()
        {
            static boost::thread_specific_ptr<ListType> the_list_ptr;

            ListPtrType result(the_list_ptr.get());
            if (0 == result)
            {
                the_list_ptr.reset(new ListType);
                result = the_list_ptr.get();
            }
            return result;
        }
    };
}

#endif
