#ifndef BASE_ACTION_EXECUTOR_H
#define BASE_ACTION_EXECUTOR_H

#include "../object.h"
#include "../utils/intrusive_list.h"
#include <boost/function.hpp>

namespace base
{
    namespace action
    {
        class ActionBase;

        // Action执行器
        class Executor : public Object
        {
            INTRUSIVE_LIST(Executor)
        public:
            Executor();

            utils::IntrusiveList<ActionBase>& actions() {
                return actions_;
            }

        protected:
            virtual ~Executor();
            
        public:
            ActionBase* GetActionByTag(int32_t tag);
            void RunAction(ActionBase* act);
            void StopAllAction();
            void BeginStopAllAction(boost::function<void()> cb);

        private:
            void OnAllActionFinish();
            ActionBase* EraseAction(ActionBase* act);
            utils::IntrusiveList<ActionBase> actions_;
            boost::function<void()> cb_stop_;
            bool registered_;
            friend class ActionManager;
        };
    }
}
#endif // NODEBASE_H
