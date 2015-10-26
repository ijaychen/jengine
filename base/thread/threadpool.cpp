#include "threadpool.h"
#include "task.h"
#include <iostream>

namespace base
{
    namespace thread
    {
        using namespace std;

        ThreadPool::ThreadPool(int min, int max)
            : thread_min_(min), thread_max_(max), run_(false)
        {
            pthread_mutex_init(&mutex_, NULL);
            pthread_cond_init(&cond_, NULL);
        }

        ThreadPool::~ThreadPool()
        {
            if (run_) {
                Stop();
            }
            pthread_mutex_destroy(&mutex_);
            pthread_cond_destroy(&cond_);

            while (!tasks_.empty()) {
                delete tasks_.front();
                tasks_.pop();
            }
        }

        void ThreadPool::AutoAjustThreadSize()
        {
            // TODO 根据负载自动调整线程池大小
        }

        void ThreadPool::AjustThreadSizeTo(int size)
        {
            // TODO
        }

        void* ThreadPool::ThreadFun(void* args)
        {
            ThreadPool* tp = static_cast<ThreadPool*>(args);
            return tp->WorkerThread(NULL);
        }

        void ThreadPool::Start()
        {
            run_ = true;
            for (int i = 0; i < thread_min_; ++i) {
                pthread_t pid;
                pthread_create(&pid, NULL, &ThreadPool::ThreadFun, this);
                threads_.push_back(pid);
            }
        }

        void ThreadPool::Stop()
        {
            run_ = false;
            pthread_cond_broadcast(&cond_);

            for (vector<pthread_t>::iterator it = threads_.begin(); it != threads_.end(); ++it) {
                pthread_join(*it, NULL);
            }
        }

        void ThreadPool::QueueUserWorkItem(Task* task)
        {
            pthread_mutex_lock(&mutex_);
            tasks_.push(task);
            pthread_mutex_unlock(&mutex_);
            pthread_cond_signal(&cond_);
        }

        void* ThreadPool::WorkerThread(void* args)
        {
            Task* job = NULL;
            while (run_) {
                pthread_mutex_lock(&mutex_);
                if (tasks_.empty()) {
                    pthread_cond_wait(&cond_, &mutex_);
                } else {
                    if (run_) {
                        job = tasks_.front();
                        tasks_.pop();
                    }
                }
                pthread_mutex_unlock(&mutex_);

                if (job) {
                    job->OnTaskExecute();
                    delete job;
                    job = NULL;
                }
            }

            return NULL;
        }
    }
}
