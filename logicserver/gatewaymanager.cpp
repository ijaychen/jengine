#include "gatewaymanager.h"
#include <base/global.h>
#include <base/memory/memorypool.h>
#include <base/gateway/gateway.h>
//#include "playersession.h"
//#include "localworld.h"
#include <boost/bind.hpp>
namespace gateserver
{
    using namespace std;
    using namespace base;
    using namespace base::memory;
    using namespace base::gateway;

    class GatewayEventHandler : public Gateway::EventHandler
    {
        virtual bool HandleAcceptUserClient ( ClientPtr client ) {
            //PlayerSession* ps =  g_localworld->Create ( client );
            //return ps != nullptr;
			return false;
        }

        virtual void OnUserClientClose ( ClientPtr client ) {
            //do nothing
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
        //SAFE_DELETE ( localworld_ );
    }


    void GatewayManager::Setup(const char * ipaddr, int port, uint32_t max_connections)
    {
		/*
        localworld_ = LocalWorld::Create();
        if ( !localworld_ ) {
            LOG_WARN ( "setup gateway fail..\n" );
            SetModuleState ( base::MODULE_STATE_DELETE );
            return;
        }
		*/
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
        //localworld_->DisconnectAll();       
    }   
}
