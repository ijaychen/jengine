#ifndef BASE_MEMORY_MEMORYCHUNK_H
#define BASE_MEMORY_MEMORYCHUNK_H

#include "../global.h"
#include <cassert>
#include <string>
#include <string.h>

namespace base
{
    namespace memory
    {
        // 内存块
        struct MemoryChunk {
            MemoryChunk(uint32_t length)
                : data(new char[length]), length(length), ref_(0) {}
            ~MemoryChunk() {
                delete[] data;
                data = NULL;
            }

            char* data;
            uint32_t length;

            uint32_t ref() const {
                return ref_;
            }
            void IncRef() {
                ++ref_;
            }
            void DecRef() {
                --ref_;
            }
        private:
            uint32_t ref_;               // 引用计数
        };

        class MemoryPool;

        // 引用计数的内存映射块，可对同一块MemoryChunk建立多个映射，每个持有一个引用计数
        class RefMemoryChunk
        {
        public:
            RefMemoryChunk()
                : mempool_(nullptr), chunk_(nullptr), data_(nullptr), pos_(0), count_(0) {}
            RefMemoryChunk(MemoryPool* mempool, MemoryChunk* chunk)
                : mempool_(mempool), chunk_(chunk), data_(chunk_->data), pos_(0), count_(chunk_->length) {
                RetainChunk();
            }
            ~RefMemoryChunk() {
                ReleaseChunk();
            }

            RefMemoryChunk(const RefMemoryChunk& rhs)
                : mempool_(rhs.mempool_), chunk_(rhs.chunk_), data_(rhs.data_), pos_(rhs.pos_), count_(rhs.count_) {
                RetainChunk();
            }
            RefMemoryChunk& operator=(const RefMemoryChunk& rhs) {
                ReleaseChunk();
                mempool_ = rhs.mempool_;
                chunk_ = rhs.chunk_;
                data_ = rhs.data_;
                pos_ = rhs.pos_;
                count_ = rhs.count_;
                RetainChunk();
                return *this;
            }

            const MemoryChunk* chunk() const {
                return chunk_;
            }
            uint32_t pos() const {
                return pos_;
            }
            char* data() {
                return data_;
            }
            const char* data() const {
                return data_;
            }
            uint32_t count() const {
                return count_;
            }

            // 重置映射偏移
            void ResetOffset(uint32_t offset, uint32_t count) {
                assert(offset <= chunk_->length && count <= chunk_->length - offset);
                data_ = chunk_->data + offset;
                count_ = count;
                Reset();
            }

            // 收缩映射数据
            void ShrinkCount(uint32_t shrink) {
                assert(count_ >= shrink);
                count_ -= shrink;
                Reset();
            }

            // 向前移动映射偏移
            void ForwardOffset(uint32_t forward) {
                assert(forward <= count_);
                data_ += forward;
                count_ -= forward;
                Reset();
            }

            // 重围可读写光标
            void Reset() {
                pos_ = 0;
            }

            // 向前移动可读写光标
            void Skip(uint32_t step) {
                assert(pos_ + step <= count_);
                pos_ += step;
            }

            // 移动读写光标至指定位置
            void SkipTo(uint32_t pos) {
                assert(pos <= count_);
                pos_ = pos;
            }

            // 可读写余量
            uint32_t FreeCount() const {
                return count_ - pos_;
            }

            // 读取count字节至dst中，返回实际读取的字节数
            uint32_t Read(char* dst, uint32_t count) {
                uint32_t free = FreeCount();
                if (free >= count) {
                    memcpy(dst, data_ + pos_, count);
                    pos_ += count;
                    return count;
                } else if (free > 0) {
                    memcpy(dst, data_ + pos_, free);
                    pos_ += free;
                    return free;
                } else {
                    return 0;
                }
            }

            // 写入count字节，返回实际写入的字节数
            uint32_t Write(const char* src, uint32_t count) {
                uint32_t free = FreeCount();
                if (free >= count) {
                    memcpy(data_ + pos_, src, count);
                    pos_ += count;
                    return count;
                } else if (free > 0) {
                    memcpy(data_ + pos_, src, free);
                    pos_ += free;
                    return free;
                } else {
                    return 0;
                }
            }
            
            std::string DebugDump();

        private:
            void RetainChunk() {
                if (chunk_ != nullptr) {
                    chunk_->IncRef();
                }
            }
            void ReleaseChunk();
            MemoryPool* mempool_;
            MemoryChunk* chunk_;
            char* data_;
            uint32_t pos_;
            uint32_t count_;
        };
    }
}

#endif // MEMORYCHUNK_H
