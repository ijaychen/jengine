#ifndef BASE_ACTION_ACTIONTIMEOUT_H
#define BASE_ACTION_ACTIONTIMEOUT_H

#include "actionbase.h"
#include <boost/function.hpp>

namespace base
{
    namespace action
    {
        class ActionTimeout : public ActionBase
        {
        public:
            // delay 延时的毫秒数
            ActionTimeout(int32_t delay)
                : ActionBase(), delay_(delay), acc_(0) {}
            ActionTimeout(int32_t delay, boost::function<void(int64_t)> cb)
                : ActionBase(), delay_(delay), acc_(0), cb_(cb) {}
            virtual ~ActionTimeout();

            virtual bool IsDone() {
                return acc_ >= delay_;
            }

        private:
            virtual void OnUpdate(int64_t tick, int32_t span);
            virtual void OnTimeout(int64_t tick);
            int32_t delay_;
            int32_t acc_;
            boost::function<void(int64_t)> cb_;
        };
    }
}

#endif // ACTIONTIMEOUT_H
