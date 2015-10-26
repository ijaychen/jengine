#ifndef BASE_GATEWAY_USER_SESSION_H
#define BASE_GATEWAY_USER_SESSION_H

#include "../object.h"

namespace base
{
    namespace gateway
    {
        class UserClient;
        class PacketIn;
        class PacketOut;

        /// gateway 持有 userclient
        /// usersession 持有 userclient

        // 用户会话
        class UserSession : public Object
        {
        public:
            UserSession(UserClient* client);
            virtual ~UserSession();

            UserClient* client() {
                return client_;
            }
            
            void Send(base::gateway::PacketOut& pktout);

            virtual void OnUserClientReceivePacket(PacketIn& pktin) = 0;
            virtual void OnUserClientClose() = 0;

        private:
            UserClient* client_;
        };
    }
}

#endif
