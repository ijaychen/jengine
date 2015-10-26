#ifndef BASE_CALLBACK_H
#define BASE_CALLBACK_H

#include "object.h"
#include <boost/function.hpp>

namespace base
{
    template<typename T>
    class CallbackObject;

    template<typename ARGS>
    class CallbackObject<void(ARGS)> : public Object
    {
    public:
        CallbackObject(const boost::function<void(ARGS)>& f) : fun_(f) {}
        virtual ~CallbackObject() {}

        inline void operator()(ARGS args) {
            if (reference_count() >= 2) {
                fun_(args);
            }
        }

    private:
        boost::function<void(ARGS)> fun_;
    };
}

#endif
