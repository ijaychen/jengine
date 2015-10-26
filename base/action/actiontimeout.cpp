#include "actiontimeout.h"

namespace base
{
    namespace action
    {
        ActionTimeout::~ActionTimeout()
        {
        }

        void ActionTimeout::OnUpdate(int64_t tick, int32_t span)
        {
            acc_ += span;
            if (acc_ >= delay_) {
                OnTimeout(tick);
            }
        }

        void ActionTimeout::OnTimeout(int64_t tick)
        {
            if (cb_) {
                cb_(tick);
            }
        }
    }
}

