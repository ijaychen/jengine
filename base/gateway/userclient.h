#ifndef BASE_GATEWAY_USERCLIENT_H
#define BASE_GATEWAY_USERCLIENT_H

#include "../net/client.h"
#include "packet.h"
#include <iostream>

namespace base
{
    namespace gateway
    {
        class UserSession;

        class UserClient : public net::Client
        {
        public:
            struct EventHandler {
                virtual ~EventHandler() {}
                virtual void OnUserClientConnect(UserClient* sender) {}
                virtual void OnUserClientConnectFail(UserClient* sender, int eno, const char* reason) {}
                virtual void OnUserClientClose(UserClient* sender) = 0;
            };

            UserClient(EventHandler& event_handler, base::memory::MemoryPool& mempool)
                : net::Client(mempool), event_handler_(event_handler), session_(nullptr) {}
            virtual ~UserClient() ;

            UserSession* session() {
                return session_;
            }

            // 设置会话
            void SetSession(UserSession* sess) {
                session_ = sess;
            }
            
            // 清除会话
            void ResetSession() {
                session_ = nullptr;
            }
            
            // 发送协议包
            void Send(PacketOut& pktout) {
                pktout.WriteHead();
                std::vector<memory::RefMemoryChunk> data = pktout.FetchData();
                PushSend(data);
	    }

        private:
            virtual void OnClose() {
                event_handler_.OnUserClientClose(this);
            }
            virtual void OnConnect() {
                event_handler_.OnUserClientConnect(this);
            }
            virtual void OnConnectFail(int eno, const char* reason) {
                event_handler_.OnUserClientConnectFail(this, eno, reason);
            }
            virtual void OnReceive(std::size_t count) ;

            EventHandler& event_handler_;
            UserSession* session_;
            PacketIn::PacketHead pkthead_;
        };
    }
}
#endif
