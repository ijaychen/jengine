#ifndef BASE_MEMORY_MEMORYPOOL_H
#define BASE_MEMORY_MEMORYPOOL_H

#include "memorychunk.h"
#include <queue>

namespace base
{
    namespace memory
    {
        // 内存池
        class MemoryPool
        {
        public:
            // 构造一个内存池
            // chunksize 单块内块大小
            // poolsize 池的最大容量
            MemoryPool(std::size_t chunksize, std::size_t poolsize);
            ~MemoryPool();

            // 可用存量
            std::size_t freesize() const {
                return chunks_.size();
            }
            // 最大容量
            std::size_t poolsize() const {
                return poolsize_;
            }
            // 单块内存大小
            std::size_t chunksize() const {
                return chunksize_;
            }
            // 计算需要的内存块数
            uint32_t CalculateChunkCount(uint32_t bytes_count) const {
                std::size_t r = bytes_count / chunksize_;
                return bytes_count % chunksize_ > 0 ? r + 1 : r;
            }

            // 分配 bytes_counts内存至容器container中,
            template<typename C>
            void AquireByByteNeed(C& container, uint32_t bytes_count) {
                uint32_t need = CalculateChunkCount(bytes_count);
                Aquire<C>(container, need);
            }

            template<typename C>
            void Aquire(C& container, uint32_t chunks_count) {
                while (chunks_count > 0) {
                    MemoryChunk* chk = nullptr;
                    if (!chunks_.empty()) {
                        chk = chunks_.front();
                        chunks_.pop();
                    } else {
                        chk = new MemoryChunk(chunksize_);
                    }
                    container.push_back(RefMemoryChunk(this, chk));
                    --chunks_count;
                }
            }

        private:
            // 释放内存块
            void Release(MemoryChunk* chunk) {
                if (chunks_.size() >= poolsize_) {
                    delete chunk;
                } else {
                    chunks_.push(chunk);
                }
            }

            std::size_t chunksize_;
            std::size_t poolsize_;
            std::queue<MemoryChunk*> chunks_;
            friend class RefMemoryChunk;
        };
    }
}

#endif // MEMORYPOOL_H
