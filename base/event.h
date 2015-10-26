#ifndef BASE_EVENT_H
#define BASE_EVENT_H

#include "global.h"
#include "object.h"
#include "utils/intrusive_list.h"
#include <boost/function.hpp>

namespace base
{
    // 事件链接器，Attach事件回调函数后，首先应持有EventLinker,否则事件无法生效
    // 减持EventLinker则自动移除事件绑定
    class EventLinker : public Object
    {
    };

    template<typename T>
    class Event {};

#ifdef HAS_CXX11
    // 通用事件绑定侦听机制(c++11, 使用可变参数模板)
    template<typename...Args>
    class Event<void(Args...)>
    {
    public:
        typedef boost::function<void(Args...)> callback_t;
    private:
        struct Handler {
            INTRUSIVE_LIST(Handler)
            callback_t cb;
            EventLinker* linker;

            Handler(const callback_t& _cb) : cb(_cb), linker(new EventLinker()) {}
            ~Handler() {
                SAFE_RELEASE(linker);
            }
        };
    public:
        ~Event() {
            Clear();
        }

        EventLinker* Attach(callback_t cb) {
            Handler* hd = new Handler(cb);
            handlers_.push_front(hd);
            return hd->linker;
        }

        void Trigger(Args... args) {
            Handler* cur = handlers_.front();
            while (cur) {
                if (cur->linker->reference_count() >= 2) {
                    cur->cb(args...);
                    cur = cur->list_next();
                } else {
                    Handler* tmp = cur;
                    cur = handlers_.erase(cur);
                    delete tmp;
                }
            }
        }

        void Clear() {
            Handler* cur = handlers_.front();
            while (cur) {
                Handler* tmp = cur;
                cur = handlers_.erase(cur);
                delete tmp;
            }
        }

    private:
        utils::IntrusiveList<Handler> handlers_;
    };
#else
    // 通用事件绑定侦听机制(兼容c++98)
    template<typename T>
    class EventBase
    {
    public:
        typedef boost::function<T> callback_t;
    protected:
        struct Handler {
            INTRUSIVE_LIST(Handler)
            callback_t cb;
            EventLinker* linker;

            Handler(const callback_t& _cb) : cb(_cb), linker(new EventLinker()) {}
            ~Handler() {
                SAFE_RELEASE(linker);
            }
        };
    public:
        virtual ~EventBase() {
            Clear();
        }

        void Clear() {
            Handler* cur = handlers_.front();
            while (cur) {
                Handler* tmp = cur;
                cur = handlers_.erase(cur);
                delete tmp;
            }
        }

        EventLinker* Attach(callback_t cb) {
            Handler* hd = new Handler(cb);
            handlers_.push_front(hd);
            return hd->linker;
        }
        
        // TODO add AttachOnce support

    protected:
        utils::IntrusiveList<Handler> handlers_;
    };

    template<>
    class Event<void()> : public EventBase<void()>
    {
    public:
        void Trigger() {
            Handler* cur = handlers_.front();
            while (cur) {
                if (cur->linker->reference_count() >= 2) {
                    cur->cb();
                    cur = cur->list_next();
                } else {
                    Handler* tmp = cur;
                    cur = handlers_.erase(cur);
                    delete tmp;
                }
            }
        }
    };

    template<typename T1>
    class Event<void(T1)> : public EventBase<void(T1)>
    {
    public:
        typedef typename EventBase<void(T1)>::Handler Handler;
        using EventBase<void(T1)>::handlers_;
        void Trigger(T1 a1) {
            Handler* cur = handlers_.front();
            while (cur) {
                if (cur->linker->reference_count() >= 2) {
                    cur->cb(a1);
                    cur = cur->list_next();
                } else {
                    Handler* tmp = cur;
                    cur = handlers_.erase(cur);
                    delete tmp;
                }
            }
        }
    };

    template<typename T1, typename T2>
    class Event<void(T1, T2)> : public EventBase<void(T1, T2)>
    {
    public:
        typedef typename EventBase<void(T1, T2)>::Handler Handler;
        using EventBase<void(T1, T2)>::handlers_;
        void Trigger(T1 a1, T2 a2) {
            Handler* cur = handlers_.front();
            while (cur) {
                if (cur->linker->reference_count() >= 2) {
                    cur->cb(a1, a2);
                    cur = cur->list_next();
                } else {
                    Handler* tmp = cur;
                    cur = handlers_.erase(cur);
                    delete tmp;
                }
            }
        }
    };

    template<typename T1, typename T2, typename T3>
    class Event<void(T1, T2, T3)> : public EventBase<void(T1, T2, T3)>
    {
    public:
        typedef typename EventBase<void(T1, T2, T3)>::Handler Handler;
        using EventBase<void(T1, T2, T3)>::handlers_;
        void Trigger(T1 a1, T2 a2, T3 a3) {
            Handler* cur = handlers_.front();
            while (cur) {
                if (cur->linker->reference_count() >= 2) {
                    cur->cb(a1, a2, a3);
                    cur = cur->list_next();
                } else {
                    Handler* tmp = cur;
                    cur = handlers_.erase(cur);
                    delete tmp;
                }
            }
        }
    };

    template<typename T1, typename T2, typename T3, typename T4>
    class Event<void(T1, T2, T3, T4)> : public EventBase<void(T1, T2, T3, T4)>
    {
    public:
        typedef typename EventBase<void(T1, T2, T3, T4)>::Handler Handler;
        using EventBase<void(T1, T2, T3, T4)>::handlers_;
        void Trigger(T1 a1, T2 a2, T3 a3, T4 a4) {
            Handler* cur = handlers_.front();
            while (cur) {
                if (cur->linker->reference_count() >= 2) {
                    cur->cb(a1, a2, a3, a4);
                    cur = cur->list_next();
                } else {
                    Handler* tmp = cur;
                    cur = handlers_.erase(cur);
                    delete tmp;
                }
            }
        }
    };

    template<typename T1, typename T2, typename T3, typename T4, typename T5>
    class Event<void(T1, T2, T3, T4, T5)> : public EventBase<void(T1, T2, T3, T4, T5)>
    {
    public:
        typedef typename EventBase<void(T1, T2, T3, T4, T5)>::Handler Handler;
        using EventBase<void(T1, T2, T3, T4, T5)>::handlers_;
        void Trigger(T1 a1, T2 a2, T3 a3, T4 a4, T5 a5) {
            Handler* cur = handlers_.front();
            while (cur) {
                if (cur->linker->reference_count() >= 2) {
                    cur->cb(a1, a2, a3, a4, a5);
                    cur = cur->list_next();
                } else {
                    Handler* tmp = cur;
                    cur = handlers_.erase(cur);
                    delete tmp;
                }
            }
        }
    };

#endif
}

#endif
