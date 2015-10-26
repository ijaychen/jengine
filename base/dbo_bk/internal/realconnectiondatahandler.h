#ifndef BASE_DBO_INTERNAL_REALCONNECTIONDATAHANDLER_H
#define BASE_DBO_INTERNAL_REALCONNECTIONDATAHANDLER_H

namespace base
{
    namespace dbo
    {
        namespace internal
        {
            class PacketIn;
            class RealConnection;

            struct RealConnectionDataHandler {
                virtual ~RealConnectionDataHandler() {}
                virtual void OnReceivePacket(RealConnection* sender, PacketIn& pktin) = 0;
                virtual void OnClose(RealConnection* sender) = 0;
            };
        }
    }
}

#endif
