#ifndef BASE_ACTION_ACTIONNEXTCALL_H
#define BASE_ACTION_ACTIONNEXTCALL_H

#include "actionbase.h"
#include <boost/function.hpp>

namespace base
{
    namespace action
    {
        // 将在下一帧执行的动作
        class ActionNextCall : public ActionBase
        {
        public:
            ActionNextCall();
            ActionNextCall(boost::function<void()> cb);
            virtual ~ActionNextCall();

        private:
            virtual void OnExecute();
            virtual void OnUpdate(int64_t tick, int32_t span);
            virtual bool IsDone();

            bool executed_;
            boost::function<void()> cb_;
        };
    }
}

#endif // ACTIONNEXTCALL_H
