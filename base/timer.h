#ifndef BASE_TIMER_H
#define BASE_TIMER_H

#include "global.h"
#include "utils/time.h"
#include "object.h"
#include <boost/function.hpp>
#include <boost/container/map.hpp>
#include <vector>

namespace base
{
    struct NowTickProvider {
        int64_t operator()() {
            return utils::nowtick();
        }
    };

    class TimeoutLinker : public Object {};

    template<typename T = NowTickProvider>
    class Timer
    {
    private:
        class DelayAction
        {
        public:
            DISABLE_COPY(DelayAction)
            DelayAction(const boost::function<void()>& fun, int64_t tick, TimeoutLinker* linker = nullptr, int64_t interval = 0)
                : fun_(fun), tick_(tick), linker_(linker), interval_(interval) {}
            ~DelayAction() {
                SAFE_RELEASE(linker_);
            }
            int64_t tick() const {
                return tick_;
            }
            int64_t interval() const {
                return interval_;
            }
            inline void operator()() {
                if (linker_ == nullptr || linker_->reference_count() == 2) {
                    fun_();
                }
            }
            void ResetTimeout(int64_t timeout) {
                tick_ = timeout;
            }
        private:
            boost::function<void()> fun_;
            int64_t tick_;
            TimeoutLinker* linker_;
            int64_t interval_;
        };
    public:
        DISABLE_COPY(Timer)

        typedef boost::container::multimap<int64_t, DelayAction*> timer_map_t;

        Timer(T tickprovider = T()) : tickprovider_(tickprovider) {}
        ~Timer() {
            StopAll();
        }

        std::size_t size() const {
            return delayqueue_.size();
        }

        void Update(int64_t tick) {
            if (delayqueue_.empty()) {
                return;
            }
            for (typename timer_map_t::iterator it = delayqueue_.begin(); it != delayqueue_.end();) {
                if (it->first > tick) {
                    break;;
                } else {
                    (*it->second)();
                    if (it->second->interval() > 0) {
                        intervals_.push_back(it->second);
                    } else {
                        delete it->second;
                    }
                    it = delayqueue_.erase(it);
                }
            }
            if (!intervals_.empty()) {
                for (typename std::vector<DelayAction*>::iterator it = intervals_.begin(); it != intervals_.end(); ++it) {
                    (*it)->ResetTimeout(tick + (*it)->interval());
                    delayqueue_.emplace((*it)->tick(), *it);
                }
                intervals_.clear();
            }
        }

        void SetTimeout(const boost::function<void()>& fun, int64_t timeout) {
            timeout += tickprovider_();
            delayqueue_.emplace(timeout, new DelayAction(fun, timeout));
        }

        TimeoutLinker* SetTimeoutWithLinker(const boost::function<void()>& fun, int64_t timeout) {
            TimeoutLinker* linker = new TimeoutLinker;
            timeout += tickprovider_();
            delayqueue_.emplace(timeout, new DelayAction(fun, timeout, linker));
            return linker;
        }

        TimeoutLinker* SetIntervalWithLinker(const boost::function<void()>& fun, int64_t interval) {
            TimeoutLinker* linker = new TimeoutLinker;
            int64_t timeout = interval + tickprovider_();
            delayqueue_.emplace(timeout, new DelayAction(fun, timeout, linker, interval));
            return linker;
        }

        void StopAll() {
            for (typename timer_map_t::iterator it = delayqueue_.begin(); it != delayqueue_.end(); ++it) {
                delete it->second;
            }
            delayqueue_.clear();
            for (typename std::vector<DelayAction*>::iterator it = intervals_.begin(); it != intervals_.end(); ++it) {
                delete *it;
            }
            intervals_.clear();
        }

    private:
        T tickprovider_;
        timer_map_t delayqueue_;
        std::vector<DelayAction*> intervals_;
    };
}

#endif // TIMER_H

