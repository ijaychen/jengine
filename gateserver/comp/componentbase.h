#ifndef COMP_COMPONENTBASE_H
#define COMP_COMPONENTBASE_H

#include "../playersession.h"
#include <base/objectmaintainer.h>

namespace gateserver
{
    namespace comp
    {
        // 组件状态
        enum ComponentState
        {
            INIT,
            OK,
            CLEANED,
        };

        class ComponentBase
        {
        public:
            ComponentBase(PlayerSession& ps, const char* comp_name)
                : ps_(ps), comp_name_(comp_name) , state_(INIT) {}
            virtual ~ComponentBase();

            // 会话
            PlayerSession& ps() {
                return ps_;
            }
            // 组件名
            const std::string& comp_name() const {
                return comp_name_;
            }
            // 组件状态
            ComponentState state() const {
                return state_;
            }

            virtual void BeginSetup() = 0;
            virtual void BeginCleanup() = 0;

            // 通知启动完成
            void NotifySetupFinish();
            // 通知退出完成
            void NotifyCleanupFinish();

        protected:
            base::ObjectMaintainer& maintainer() {
                return maintainer_;
            }

        private:
            virtual void OnSetupFinish() {}
            virtual void OnCleanupFinish() {
                maintainer_.ClearAll();
            }

        private:
            PlayerSession& ps_;
            base::ObjectMaintainer maintainer_;
            std::string comp_name_;
            ComponentState state_;
        };
    }
}

#endif // COMP_COMPONENTBASE_H
