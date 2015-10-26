#include "eventtimeout.h"
#include "dispatcher.h"
#include <cassert>

namespace base
{
    namespace event
    {
        using namespace std;

        void EventTimeout::AddToDispatcher()
        {
            int64_t expire = Dispatcher::instance().GetTickCache() + delay();
            Dispatcher::instance().timers_.insert(make_pair(expire, this));
            Retain();
        }

        void EventTimeout::RemoveFromDispatcher()
        {
            timer_map_t& timers = Dispatcher::instance().timers_;
            for (timer_map_t::iterator it = timers.begin(); it != timers.end(); ++it) {
                if (it->second == this) {
                    timers.erase(it);
                    Release();
                    return;
                }
            }
            assert("not exit timeout event");
        }
    }
}
