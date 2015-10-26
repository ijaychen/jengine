#ifndef MODULEGATEWAY_H
#define MODULEGATEWAY_H
#include <base/global.h>
namespace base
{
    namespace memory
    {
        class MemoryPool;
    }
    namespace gateway
    {
        class Gateway;
    }
   
}

namespace gateserver
{
    class LocalWorld;
    class GatewayEventHandler;
    class ActionCheckIfAllPlayerExit;

    class GatewayManager
    {
	public:
        ~GatewayManager();
        GatewayManager();
        void Setup(const char * ipaddr, int port, uint32_t max_connections);
        void Stop();
       
        base::memory::MemoryPool* mempool_;
        GatewayEventHandler* handler_;
        base::gateway::Gateway* gateway_;
       // LocalWorld* localworld_;
        
        friend class ActionCheckIfAllPlayerExit;
    };
}
#endif // MODULEGATEWAY_H
