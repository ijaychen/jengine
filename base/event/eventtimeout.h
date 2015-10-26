#ifndef BASE_EVENT_EVENTTIMEOUT_H
#define BASE_EVENT_EVENTTIMEOUT_H

#include "../object.h"

namespace base
{
    namespace event
    {
        class Dispatcher;

        // 定时器事件
        class EventTimeout : public Object
        {
        public:
            // 构造函数
            // delay 延时(ms)
            EventTimeout(int delay) : delay_(delay) {}
            virtual ~EventTimeout() {}

            int delay() const {
                return delay_;
            }

            // 添加到事件派发器中
            void AddToDispatcher();
            // 从事件派发器中移除
            void RemoveFromDispatcher();

        private:
            // 定时器触发
            virtual void OnTimeout() = 0;
            int delay_;
            friend class Dispatcher;
        };
    }
}

#endif
