/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/// Include guards.
#ifndef HAVE_THE_LIBEBT_LIBEBT_PTHREAD_THREADS_HH
#define HAVE_THE_LIBEBT_LIBEBT_PTHREAD_THREADS_HH 1

#include <pthread.h>
#include <deque>
#include <string>
#include <exception>

/** \file
 * Declarations for libebt::PthreadBacktraceContextHolder template
 * class, a helper struct for setups using Boost.Threads.
 */

namespace libebt
{
    /**
     * Thrown if pthread is unhappy.
     */
    class PthreadException : public std::exception
    {
        public:
            PthreadException() throw () :
                std::exception()
            {
            }

            virtual ~PthreadException() throw ()
            {
            }
    };

    /**
     * Helper struct for libebt::BacktraceContextHolder when using
     * pthread.
     */
    template <typename Tag_, typename StringType_ = std::string>
    class PthreadBacktraceContextHolder
    {
        public:
            /**
             * Our list type.
             */
            typedef std::deque<StringType_> ListType;

            /**
             * A pointer to our list type.
             */
            typedef ListType * ListPtrType;

        protected:
            /**
             * Tidy up our allocated TLS data.
             */
            static void delete_tls_data(void * p)
            {
                delete (reinterpret_cast<ListType *>(p));
            }

        public:
            /**
             * Get the thread-specific list.
             */
            static ListPtrType get_list()
            {
                static bool first_time(true);
                static pthread_key_t key;

                if (first_time)
                {
                    first_time = false;
                    if (0 != pthread_key_create(&key, &PthreadBacktraceContextHolder::delete_tls_data))
                        throw PthreadException();
                }
                if (0 == pthread_getspecific(key))
                    if (0 != pthread_setspecific(key, new ListType))
                        throw PthreadException();
                return reinterpret_cast<ListType *>(pthread_getspecific(key));
            }
    };
}


#endif
