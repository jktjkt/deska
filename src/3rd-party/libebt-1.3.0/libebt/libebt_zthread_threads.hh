/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/// Include guards.
#ifndef HAVE_THE_LIBEBT_LIBEBT_ZTHREAD_THREADS_HH
#define HAVE_THE_LIBEBT_LIBEBT_ZTHREAD_THREADS_HH 1

#include <deque>
#include <string>

#include <zthread/ThreadLocal.h>

/** \file
 * Declarations for libebt::ZThreadBacktraceContextHolder template
 * class, a helper struct for setups using ZThread.
 */

namespace libebt
{
    namespace internal_only
    {

        /**
         * Really basic reference counted pointer class, necessary because ZThread
         * thread local storage is really really dumb. Do not use this in your own
         * code -- get boost::shared_ptr instead.
         */
        template <typename T_>
        class RefCountedPtr
        {
#ifndef DOXYGEN
            private:
                struct Data
                {
                    T_ * value;
                    unsigned count;
                };

                Data * _data;

            public:
                RefCountedPtr(T_ * const value) :
                    _data(new Data)
                {
                    _data->value = value;
                    _data->count = 1;
                }

                RefCountedPtr(const RefCountedPtr & other) :
                    _data(other._data)
                {
                    ++(_data->count);
                }

                ~RefCountedPtr()
                {
                    if (0 == --(_data->count))
                    {
                        delete _data->value;
                        delete _data;
                    }
                }

                T_ & operator* ()
                {
                    return *(_data->value);
                }

                T_ * operator-> ()
                {
                    return _data->value;
                }
#endif
        };
    }

    /**
     * Helper struct for libebt::BacktraceContextHolder when using
     * ZThread.
     */
    template <typename Tag_, typename StringType_ = std::string>
    struct ZThreadBacktraceContextHolder
    {
        /**
         * Our list type.
         */
        typedef std::deque<std::string> ListType;

        /**
         * A pointer to our list type.
         */
        typedef internal_only::RefCountedPtr<ListType> ListPtrType;

        /**
         * Create a new ListPtrType instance.
         */
        struct MakeListPtr
        {
#ifndef DOXYGEN
            ListPtrType operator() () const
            {
                return ListPtrType(new ListType);
            }
#endif
        };

        /**
         * Fetch the list for the current thread.
         */
        static ListPtrType get_list()
        {
            static ZThread::ThreadLocal<ListPtrType, MakeListPtr> the_list_ptr;
            return the_list_ptr.get();
        }
    };
}

#endif
