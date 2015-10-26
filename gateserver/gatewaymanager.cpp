#include "gatewaymanager.h"
#include <base/global.h>
#include <base/memory/memorypool.h>
#include <base/gateway/gateway.h>
#include "playersession.h"
#include "sessionmanager.h"
#include <boost/bind.hpp>
#include <base/logger.h>
namespace gateserver
{
    using namespace std;
    using namespace base;
    using namespace base::memory;
    using namespace base::gateway;

    class GatewayEventHandler : public Gateway::EventHandler
    {
        virtual bool HandleAcceptUserClient ( ClientPtr client ) {
            PlayerSession* ps =  g_sessionManager->CreateSession ( client );
            return ps != nullptr;			
        }

        virtual void OnUserClientClose ( ClientPtr client ) {
            //do nothing
			LOG_WARN("OnUserClientClose");
        }

    };

    /// ModuleGateway
    GatewayManager::GatewayManager()
        : mempool_ ( nullptr ), handler_ ( nullptr ), gateway_ ( nullptr ) //, localworld_ ( nullptr )
    {

    }

    GatewayManager::~GatewayManager()
    {
        SAFE_DELETE ( gateway_ );
        SAFE_DELETE ( handler_ );
        SAFE_DELETE ( mempool_ );
        
    }


    void GatewayManager::Setup(const char * ipaddr, int port, uint32_t max_connections)
    {
        mempool_ = new MemoryPool ( 64, 1280 );
        handler_ = new GatewayEventHandler;
        gateway_ = new Gateway ( *handler_, *mempool_ );

        if ( !gateway_->Setup(ipaddr, port, max_connections) ) {           
            LOG_WARN ( "setup gateway fail..\n" );
        } 
		 LOG_WARN ( "setup gateway success..\n" );
    }

    void GatewayManager::Stop()
    {      
        //停止接收新的连接
        gateway_->StopListener();
        //将所有用户踢下线
        g_sessionManager->DisconnectAllSession();       
    }   
}
