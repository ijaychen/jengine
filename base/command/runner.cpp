#include "runner.h"
#include "runnermgr.h"

namespace base
{
    namespace command
    {
        Runner::Runner()
            : queue_(new CommandQueue)
        {
        }

        Runner::~Runner()
        {
            SAFE_RELEASE(queue_);
        }

        void Runner::PushCommand(base::command::Command* cmd)
        {
            queue_->Push(cmd);
            RunnerMgr::instance().Register(queue_);
        }
    }
}

