#ifndef BASE_OBJECT_H
#define BASE_OBJECT_H

#include "global.h"

namespace base
{
    // 引用计数的对象
    class Object
    {
    public:
        Object();

        // 对象的引用次数
        uint32_t reference_count() const {
            return reference_count_;
        }
        
        // 是否单一引用
        bool IsSingleReference() const {
            return reference_count_ == 1;
        }
        // 释放一次引用
        void Release();
        // 持有一次引用
        void Retain();
        // 自动释放(在下次循环中调用Release)
        void AutoRelease();
        
    protected:
        virtual ~Object();

    private:
        uint32_t reference_count_;
    };
}

#define SAFE_RELEASE(obj) do { if (obj != nullptr) { obj->Release(); obj = nullptr; } } while(0);

#endif // OBJECT_H
