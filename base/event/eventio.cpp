#include "eventio.h"
#include "dispatcher.h"
#include "../logger.h"
#include <unistd.h>

namespace base
{
    namespace event
    {
        using namespace std;

        EventIO::~EventIO()
        {
        }

        void EventIO::AddToDispatcher(int fd, int evt)
        {
            assert(!list_linked());
            ioevt_ = evt;
            if (fd != -1 && fd != fd_) {
                if (fd_ != -1) {
                    close(fd_);
                    // remote evt from epoll ?
                }
                fd_ = fd;
            }
            Dispatcher::instance().io_list_.push_front(this);
            int epfd = Dispatcher::instance().epfd_;
            epoll_event ee;
            ee.events = EPOLLET
                        | (ioevt_ & IO_READABLE ?  EPOLLIN : 0)
                        | (ioevt_ & IO_WRITEABLE ? EPOLLOUT : 0);
            ee.data.ptr = this;
            int r = epoll_ctl(epfd, EPOLL_CTL_ADD, fd_, &ee);
            errno_assert(r == 0);
            Retain();
        }

        void EventIO::ModifyIOEvent(int ioevt)
        {
            assert(list_linked());
            int epfd = Dispatcher::instance().epfd_;
            ioevt_ = ioevt;
            epoll_event ee;
            ee.events = EPOLLET
                        | (ioevt_ & IO_READABLE ?  EPOLLIN : 0)
                        | (ioevt_ & IO_WRITEABLE ? EPOLLOUT : 0);
            ee.data.ptr = this;
            int r = epoll_ctl(epfd, EPOLL_CTL_MOD, fd_, &ee);
            errno_assert(r == 0);
        }

        void EventIO::CloseFD()
        {
            if (fd_ != -1) {
                close(fd_);
                fd_ = -1;
            }
        }

        void EventIO::Close()
        {
            if (!closed_ && list_linked()) {
                int epfd = Dispatcher::instance().epfd_;
                epoll_event ee;
                int r = epoll_ctl(epfd, EPOLL_CTL_DEL, fd_, &ee);
                errno_assert(r == 0);
                Dispatcher::instance().closed_io_list_.push(this);
                closed_ = true;
            }
            CloseFD();
        }
    }
}
