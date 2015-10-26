#ifndef BASE_OBJECTMAINTAINER_H
#define BASE_OBJECTMAINTAINER_H

#include "global.h"
#include <vector>

namespace base
{
    class Object;
    class ObjectMaintainer
    {
    public:
        DISABLE_COPY(ObjectMaintainer)
        ObjectMaintainer();
        ~ObjectMaintainer();

        ObjectMaintainer& Add(Object* obj, int32_t tag = 0);
        void ClearAll();
        void ClearByTag(int32_t tag);

    private:
        struct Item {
            Item(Object* o, int32_t t) : obj(o), tag(t) {}
            Object* obj;
            int32_t tag;
        };
        std::vector<Item> objs_;
    };
}

#endif // OBJECTMAINTAINER_H
