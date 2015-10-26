#ifndef BASE_AUTORELEASEPOOL_H
#define BASE_AUTORELEASEPOOL_H

#include <vector>
#include <stack>
#include "object.h"

namespace base
{
    class AutoReleasePool
    {
    public:
        AutoReleasePool() {
            objects_.reserve(20);
        }
        ~AutoReleasePool() {
            Clear();
        }

        void Clear() {
            if (!objects_.empty()) {
                for (std::vector<Object*>::iterator it = objects_.begin(); it != objects_.end(); ++it) {
                    (*it)->Release();
                }
                objects_.clear();
            }
        }

        void AddObject(Object* obj) {
            objects_.push_back(obj);
        }

    private:
        std::vector<Object*> objects_;
    };

    class PoolManager
    {
    public:
        static void CreateInstance();
        static void DeleteInstance();
        static PoolManager* instance() {
            return instance_;
        }

        void AddObject(Object* obj) {
            if (pools_.empty()) {
                Push();
            }
            pools_.top()->AddObject(obj);
        }

        void Push() {
            AutoReleasePool* pool = new AutoReleasePool;
            pools_.push(pool);
        }
        void Pop() {
            if (pools_.size() > 1) {
                delete pools_.top();
                pools_.pop();
            } else {
                if (pools_.size() == 1) {
                    pools_.top()->Clear();
                }
            }
        }

    private:
        PoolManager();
        ~PoolManager();
        static PoolManager* instance_;
        std::stack<AutoReleasePool*> pools_;
    };
}

#endif // AUTORELEASEPOOL_H
