#include "executor.h"
#include "actionbase.h"
#include "actionmanager.h"
#include <iostream>

namespace base
{
    namespace action
    {
        using namespace std;

        Executor::Executor()
            : registered_(false)
        {
        }

        Executor::~Executor()
        {
            StopAllAction();
        }

        ActionBase* Executor::GetActionByTag(int32_t tag)
        {
            ActionBase* cur = actions_.front();
            while (cur) {
                if (cur->tag() == tag) {
                    return cur;
                }
                cur = cur->list_next();
            }
            return nullptr;
        }

        void Executor::RunAction(ActionBase* act)
        {
            if (!registered_) {
                registered_ = true;
                ActionManager::instance().RegisterNode(this);
            }
            actions_.push_front(act);
            act->Retain();
        }

        void Executor::OnAllActionFinish()
        {
            registered_ = false;
            if (cb_stop_) {
                cb_stop_();
            }
        }

        void Executor::StopAllAction()
        {
            ActionBase* cur = actions_.front();
            while (cur) {
                cur->Stop();
                cur = cur->list_next();
            }
        }

        void Executor::BeginStopAllAction(boost::function< void() > cb)
        {
            if (actions_.empty()) {
                cb();
                return;
            }
            cb_stop_ = cb;
            StopAllAction();
        }

        ActionBase* Executor::EraseAction(ActionBase* act)
        {
            ActionBase* ret = actions_.erase(act);
            act->Release();
            return ret;
        }
    }
}
