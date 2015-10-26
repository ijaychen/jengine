#include "actionmanager.h"
#include "executor.h"
#include "actionbase.h"
#include <cassert>

namespace base
{
    namespace action
    {
        ActionManager::ActionManager()
        {
        }

        ActionManager::~ActionManager()
        {
        }

        void ActionManager::Update(int64_t tick, int32_t span)
        {
            Executor* cur_exe = executors_.front();
            while (cur_exe) {
                ActionBase* cur_act = cur_exe->actions().front();

                // if the node has no more actions to execute, unregister it
                if (!cur_act) {
                    Executor* tmp = cur_exe;
                    cur_exe = executors_.erase(cur_exe);
                    tmp->OnAllActionFinish();
                    tmp->Release();
                    continue;
                }

                while (cur_act) {
                    if (cur_act->stopped()) {
                        ActionBase* tmp = cur_act;
                        cur_act = cur_exe->actions().erase(cur_act);
                        tmp->Release();
                    } else {
                        cur_act->OnUpdate(tick, span);
                        if (cur_act->stopped() || cur_act->IsDone()) {
                            ActionBase* tmp = cur_act;
                            cur_act = cur_exe->actions().erase(cur_act);
                            tmp->Release();
                        } else {
                            cur_act = cur_act->list_next();
                        }
                    }
                }
                cur_exe = cur_exe->list_next();
            }
        }

        void ActionManager::RegisterNode(Executor* exe)
        {
            assert(!exe->list_linked());
            exe->Retain();
            executors_.push_front(exe);
        }

    }
}

