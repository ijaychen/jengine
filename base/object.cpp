#include "object.h"
#include "autoreleasepool.h"
#include <iostream>
#include <typeinfo>

namespace base
{
    using namespace std;

    uint32_t g_object_count = 0;

    Object::Object()
        : reference_count_(1)
    {
        ++g_object_count;
    }

    Object::~Object()
    {
        --g_object_count;
    }

    void Object::Retain()
    {
        ++reference_count_;
    }

    void Object::Release()
    {
        --reference_count_;
        if (reference_count_ == 0) {
            delete this;
        }
    }

    void Object::AutoRelease()
    {
        PoolManager::instance()->AddObject(this);
    }
}
