#include "dispatcher.h"
#include "eventtimeout.h"
#include "eventio.h"
#include "../autoreleasepool.h"
#include "../logger.h"
#include "../command/runnermgr.h"
#include <unistd.h>
#include <sys/epoll.h>

base::event::Dispatcher* g_dispatcher = nullptr;

namespace base
{
    namespace event
    {
        using namespace std;

        /// EventDispatcher
        Dispatcher::Dispatcher()
            : exit_(false), tick_last_(0), tick_(0), epfd_(-1), wait_(50), frame_no_(0)
        {
            assert(g_dispatcher == nullptr);
            epfd_ = epoll_create(9999);
            errno_assert(epfd_ > 0);
            UpdateTickCache();
            g_dispatcher = this;
        }

        Dispatcher::~Dispatcher()
        {
            close(epfd_);
            g_dispatcher = nullptr;
        }

        void Dispatcher::NormalExit()
        {
            exit_ = true;
        }

        void Dispatcher::Clear()
        {
            // TODO
        }

        void Dispatcher::DebugDump()
        {
            cout << "Dispatcher: is_exit=" << boolalpha << exit_ <<
                 ", io_list_=" << io_list_.size() <<
                 ", timers_=" << timers_.size() <<
                 ", closed_io_list_=" << closed_io_list_.size() << endl;
        }

        void Dispatcher::Dispatch(boost::function<void(int64_t)>* mainloop)
        {
            UpdateTickCache();
            tick_last_ = tick_;
            int32_t tick_span = 0;
            int n = 0;
            while (true) {
                ++frame_no_;
                // update tick
                UpdateTickCache();
                tick_span = tick_ - tick_last_;
                tick_last_ = tick_;

                // execute timer
                while (!timers_.empty()) {
                    timer_map_t::iterator it = timers_.begin();
                    if (it->first > tick_) {
                        break;
                    }
                    it->second->OnTimeout();
                    timers_.erase(it);
                    it->second->Release();
                }

                // execute quick timer
                quicktimer_.Update(tick_);

                // execute command
                command::RunnerMgr::instance().Update(tick_);

                if (mainloop) {
                    (*mainloop)(tick_);
                }

                //action::ActionManager::instance().Update(tick_, tick_span);

                // auto release memory
                PoolManager::instance()->Pop();

                if (exit_) {
                    if (io_list_.empty()
                            && timers_.empty()
                            && closed_io_list_.empty()
                           // && action::ActionManager::instance().nodes().empty()
                            && command::RunnerMgr::instance().empty()) {
                        // there is no pending event, then stop the dispatcher
                        break;
                    } else {
                        ;
                    }
                }

                n = epoll_wait(epfd_, evtbuf_, EVT_BUF_SIZE, wait_);

                if (n <= -1) {
                    // error
                    // TODO error handle
                } else if (n == 0) {
                    // timeout
                } else {
                    // handle io event
                    for (int i = 0; i < n; ++i) {
                        EventIO* evobj = static_cast<EventIO*>(evtbuf_[i].data.ptr);
                        if (!evobj->closed() && ((evtbuf_[i].events & EPOLLIN) | (evtbuf_[i].events & EPOLLERR))) {
                            evobj->OnEventIOReadable();
                        }
                        if (!evobj->closed() && (evtbuf_[i].events & EPOLLOUT)) {
                            evobj->OnEventIOWriteable();
                        }
                    }
                }

                while (!closed_io_list_.empty()) {
                    EventIO* cur = closed_io_list_.front();
                    cur->OnEventIOClose();
                    io_list_.erase(cur);
                    cur->Release();
                    closed_io_list_.pop();
                }
            }
            
            quicktimer_.StopAll();
        }
    }
}
