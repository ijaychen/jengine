#include "memorypool.h"
#include "../logger.h"

namespace base
{
    namespace memory
    {
        using namespace std;
        
        MemoryPool::MemoryPool(std::size_t chunksize, std::size_t poolsize)
            : chunksize_(chunksize), poolsize_(poolsize)
        {
            while (chunks_.size() < poolsize_) {
                chunks_.push(new MemoryChunk(chunksize_));
            }
        }

        MemoryPool::~MemoryPool()
        {
            if (chunks_.size() != poolsize_) {
                LOG_ERROR("memory leak detected, mempool.freesize:%u, mempool.poolsize:%u\n", chunks_.size(), poolsize_);
            }
            while (!chunks_.empty()) {
                delete chunks_.front();
                chunks_.pop();
            }
        }
    }
}
