#ifndef BASE_THREAD_TASK_H
#define BASE_THREAD_TASK_H

#include "../global.h"

namespace base
{
    namespace thread
    {
        class Task
        {
        public:
            Task() {}
            virtual ~Task();

        private:
            DISABLE_COPY(Task)
            virtual void OnTaskExecute() = 0;
            friend class ThreadPool;
        };
    }
}

#endif
