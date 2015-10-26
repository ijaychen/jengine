#include "objectmaintainer.h"
#include "object.h"

namespace base
{
    using namespace std;

    ObjectMaintainer::ObjectMaintainer()
    {
    }

    ObjectMaintainer::~ObjectMaintainer()
    {
        ClearAll();
    }

    ObjectMaintainer& ObjectMaintainer::Add(Object* obj, int32_t tag)
    {
        objs_.push_back(Item(obj, tag));
        obj->Retain();
        return *this;
    }

    void ObjectMaintainer::ClearByTag(int32_t tag)
    {
        for (vector<Item>::iterator it = objs_.begin(); it != objs_.end();) {
            if ((*it).tag == tag) {
                (*it).obj->Release();
                it = objs_.erase(it);
            } else {
                ++it;
            }
        }
    }

    void ObjectMaintainer::ClearAll()
    {
        for (vector<Item>::iterator it = objs_.begin(); it != objs_.end(); ++it) {
            (*it).obj->Release();
        }
        objs_.clear();
    }
}
