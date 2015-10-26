#ifndef BASE_GATEWAY_PACKETFILTER_H
#define BASE_GATEWAY_PACKETFILTER_H

#include "../utils/intrusive_list.h"

namespace base
{
    namespace gateway
    {
        class PacketIn;
        class PacketFilter
        {
            INTRUSIVE_LIST(PacketFilter)
        public:
            virtual ~PacketFilter() {}

            // 返回false阻止后续过滤器接收协议包
            virtual bool HandleReceivePacket(PacketIn& pktin) = 0;
        };
    }
}

#endif
