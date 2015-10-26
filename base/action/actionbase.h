#ifndef BASE_ACTION_ACTIONBASE_H
#define BASE_ACTION_ACTIONBASE_H

#include "../object.h"
#include "../utils/intrusive_list.h"

namespace base
{
    namespace action
    {
        class Executor;

        class ActionBase : public Object
        {
            INTRUSIVE_LIST(ActionBase)
        public:
            ActionBase();
            virtual ~ActionBase();

            int32_t tag() const {
                return tag_;
            }

            // 设置标记
            void SetTag(int32_t t) {
                tag_ = t;
            }

            Executor* target() {
                return target_;
            }
            bool stopped() const {
                return stopped_;
            }
            // 更新, 每帧执行一次
            virtual void OnUpdate(int64_t tick, int32_t span) = 0;
            // 是否已完成
            virtual bool IsDone() = 0;
            // 在IsDone()返回true之后执行,OnStop之后将执行Release动作
            virtual void OnStop() {}
            // 停止
            void Stop() {
                stopped_ = true;
            }

        private:
            bool stopped_;
            Executor* target_;
            int32_t tag_;
        };
    }
}

#endif // ACTIONBASE_H
