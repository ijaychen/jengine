#include "actionnextcall.h"
#include <iostream>

namespace base
{
    namespace action
    {
        using namespace std;
        
        ActionNextCall::ActionNextCall()
            : ActionBase(), executed_(false)
        {
        }

        ActionNextCall::ActionNextCall(boost::function<void()> cb)
            : ActionBase(), executed_(false), cb_(cb)
        {
        }

        ActionNextCall::~ActionNextCall()
        {
        }

        bool ActionNextCall::IsDone()
        {
            return executed_;
        }

        void ActionNextCall::OnUpdate(int64_t tick, int32_t span)
        {
            if (!executed_) {
                OnExecute();
                executed_ = true;
            }
        }

        void ActionNextCall::OnExecute()
        {
            if (cb_) {
                cb_();
            }
        }
    }
}
