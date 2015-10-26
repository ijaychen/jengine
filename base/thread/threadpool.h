#ifndef BASE_THREAD_THREADPOOL_H
#define BASE_THREAD_THREADPOOL_H

#include <pthread.h>
#include <queue>

namespace base
{
    namespace thread
    {
        class Task;
        class ThreadPool
        {
        public:
            ThreadPool(int min = 2, int max = 8);
            ~ThreadPool();
            
            void Start();
            void Stop();
            
            void SetMinThreads(int min) {
                thread_min_ = min;
            }
            void SetMaxThreads(int max) {
                thread_max_ = max;
            }
            void QueueUserWorkItem(Task* task);
            
        private:
            static void* ThreadFun(void* args);
            void* WorkerThread(void* args);
            void AutoAjustThreadSize();
            void AjustThreadSizeTo(int size);
            
            
            int thread_min_;
            int thread_max_;
            bool run_;
            
            pthread_mutex_t mutex_;
            pthread_cond_t cond_;
            std::queue<Task*> tasks_;
            std::vector<pthread_t> threads_;
        };
    }
}

#endif
