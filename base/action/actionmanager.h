#ifndef BASE_ACTION_ACTIONMANAGER_H
#define BASE_ACTION_ACTIONMANAGER_H

#include "../global.h"
#include "../utils/intrusive_list.h"

namespace base
{
    namespace action
    {
        class ActionBase;
        class Executor;

        // 管理并调度Action的执行
        // 管理Node与Action的引用关系与生命周期  Node -> ActionManager <- Action
        class ActionManager
        {
        public:
            DISABLE_COPY(ActionManager)
            static ActionManager& instance() {
                static ActionManager ins;
                return ins;
            }
            ~ActionManager();

            std::size_t nodes_size() const {
                return executors_.size();
            }
            const utils::IntrusiveList<Executor>& nodes() const {
                return executors_;
            }

            // 更新执行所有节点上的action
            void Update(int64_t tick, int32_t span);

        private:
            void RegisterNode(Executor* node);

        private:
            ActionManager();
            utils::IntrusiveList<Executor> executors_;
            friend class Executor;
        };
    }
}
#endif // ACTIONMANAGER_H
