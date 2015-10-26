#include "listener.h"

#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "utils.h"
#include "client.h"
#include "../logger.h"

namespace base
{
    namespace net
    {
        using namespace std;

        Listener::~Listener()
        {
        }

        // 绑定并侦听
        bool Listener::Bind(const char* ipaddr, int port)
        {
            sockaddr_in serveraddr;
            convert_sockaddr(&serveraddr, ipaddr, port);

            // 创建侦听套接字
            int listenfd = socket(AF_INET, SOCK_STREAM, 0);

            // 设置socket
            int reuseaddr = 1;
            int rc = 0;
            rc = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int));
            errno_assert(rc == 0);

            int sndbuf = 8192;
            rc = setsockopt(listenfd, SOL_SOCKET, SO_SNDBUF, &sndbuf, sizeof(int));
            errno_assert(rc == 0);

            int rcvbuf = 8192;
            rc = setsockopt(listenfd, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(int));
            errno_assert(rc == 0);

            // 设置为非阻塞模式
            make_fd_nonblocking(listenfd);

            // bind
            rc = ::bind(listenfd, (sockaddr*)&serveraddr, sizeof(struct sockaddr_in));
            errno_assert(rc == 0);

            // listen
            rc = listen(listenfd, 5);
            errno_assert(rc == 0);

            AddToDispatcher(listenfd, event::IO_READABLE);
            return true;
        }

        void Listener::OnEventIOReadable()
        {
            while (true) {
                sockaddr_in clientaddr;
                socklen_t clientaddrlen = sizeof(struct sockaddr_in);
                int clientfd = accept(fd(), (sockaddr*)&clientaddr, &clientaddrlen);
                if (clientfd == -1) {
                    if (errno == EWOULDBLOCK) {
                        break;
                    } else {
                        LOG_ERROR("accept fail, errno:%d\n", errno);
                        return;
                    }
                }
                OnAccept(clientfd);
            }
        }

        void Listener::OnEventIOWriteable()
        {
            release_assert("panic");
        }
    }
}

