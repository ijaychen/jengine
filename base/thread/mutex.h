#ifndef BASE_THREAD_MUTEX_H
#define BASE_THREAD_MUTEX_H

#include "../global.h"
#include <pthread.h>

namespace base
{
    namespace thread
    {
        class Mutex
        {
        public:
            Mutex() {
                pthread_mutex_init(&mutex_, NULL);
            }
            ~Mutex() {
                pthread_mutex_destroy(&mutex_);
            }

            void Lock() {
                pthread_mutex_lock(&mutex_);
            }
            void Unlock() {
                pthread_mutex_unlock(&mutex_);
            }

        private:
            DISABLE_COPY(Mutex)
            pthread_mutex_t mutex_;
        };

        class LockGuard
        {
        public:
            LockGuard(Mutex& mutex)
                : mutex_(mutex) {
                mutex_.Lock();
            }
            ~LockGuard() {
                mutex_.Unlock();
            }

        private:
            DISABLE_COPY(LockGuard)
            Mutex& mutex_;
        };
    }
}
#endif // MUTEX_H
