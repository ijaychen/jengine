#include "autoreleasepool.h"
#include <cassert>
#include <cstddef>

namespace base
{
    using namespace std;

    /// AutoReleasePool
    PoolManager* PoolManager::instance_ = NULL;

    void PoolManager::CreateInstance()
    {
        assert(instance_ == NULL);
        instance_ = new PoolManager();
    }

    void PoolManager::DeleteInstance()
    {
        assert(instance_ != NULL);
        delete instance_;
        instance_ = NULL;
    }

    PoolManager::PoolManager()
    {
    }

    PoolManager::~PoolManager()
    {
        while (!pools_.empty()) {
            delete pools_.top();
            pools_.pop();
        }
    }
}
