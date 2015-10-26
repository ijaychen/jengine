#include "memorychunk.h"
#include "memorypool.h"
#include "../utils/string.h"

namespace base
{
    namespace memory
    {
        void RefMemoryChunk::ReleaseChunk()
        {
            if (chunk_ != nullptr) {
                chunk_->DecRef();
                if (chunk_->ref() == 0) {
                    mempool_->Release(chunk_);
                }
                mempool_ = nullptr;
                chunk_ = nullptr;
                data_ = nullptr;
                pos_ = 0;
                count_ = 0;
            }
        }
        
        std::string RefMemoryChunk::DebugDump()
        {
            std::string ret;
            base::utils::string_append_format(ret, "rawchunk.length=%u, offset=%u, pos=%u, count=%u, data=", chunk_->length, data_ - chunk_->data, pos_, count_);
            for (uint32_t i = 0; i < count_; ++i) {
                base::utils::string_append_format(ret, "%u, ", (uint8_t)(*(data_ + i)));
            }
            return ret;
        }
    }
}
