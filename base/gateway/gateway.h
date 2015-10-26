#ifndef BASE_GATEWAY_GATEWAY_H
#define BASE_GATEWAY_GATEWAY_H

#include "../logger.h"
#include "../net/listener.h"
#include "userclient.h"
#include <unistd.h>
#include <boost/function.hpp>
#include <set>

namespace base
{
    namespace gateway
    {
        typedef UserClient* ClientPtr;
        typedef std::set<ClientPtr> client_set_t;

        class Gateway : public UserClient::EventHandler, public net::Listener::EventHandler
        {
        public:
            DISABLE_COPY(Gateway)

            struct EventHandler {
                virtual ~EventHandler() {}
                virtual bool HandleAcceptUserClient(ClientPtr client) = 0;
                virtual void OnUserClientClose(ClientPtr client) = 0;
            };

            Gateway(EventHandler& handler, memory::MemoryPool& mempool)
                : handler_(handler), listener_(nullptr), mempool_(mempool), max_connections_(1000), is_cleanup_(false) {}
            ~Gateway() ;

            // 连接数
            std::size_t clients_size() const {
                return clients_.size();
            }

            // 设置并启动网关
            // ipaddr 绑定的侦听地址
            // port 绑定的端口
            // max_connections 最大连接数，越过最大连接数时，将拒绝连接
            bool Setup(const char* ipaddr, int port, uint32_t max_connections = 3000);
            bool SetupByConfigFile();

            // 开始清理数据，关闭所有连接 (注意，需要等待所有客户端都已经全部清除后，方可释放gateway)
            void BeginCleanup(boost::function<void()> cb) ;
            // 广播
            void Broadcast(PacketOut& pktout) ;
            // 关闭侦听器
            void StopListener();

        private:
            virtual void OnUserClientConnect(ClientPtr client) ;
            virtual void OnUserClientClose(ClientPtr client) ;
            virtual void OnListenerAccept(net::Listener* sender, int clientfd) ;
            void CheckIfCleanupFinish() {
                if (is_cleanup_ && clients_.empty()) {
                    cleanup_finsh_cb_();
                }
            }

            EventHandler& handler_;
            net::Listener* listener_;
            base::memory::MemoryPool& mempool_;
            // 所有客户端列表
            std::set<ClientPtr> clients_;
            uint32_t max_connections_;
            bool is_cleanup_;
            boost::function<void()> cleanup_finsh_cb_;
        };
    }
}

#endif // GATEWAY_H
