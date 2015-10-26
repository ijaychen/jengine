#include "playersession.h"
#include <base/framework.h>
#include <base/gateway/userclient.h>
#include <base/logger.h>
#include <base/event/dispatcher.h>
#include <boost/bind.hpp>
#include "response.h"
#include "sessionmanager.h"

namespace gateserver
{
    using namespace std;
    using namespace base;
	using namespace boost;
    using namespace base::gateway;

    PlayerSession::PlayerSession ( UserClient* uc )
        : UserSession ( uc ), uid_ ( 0 ) , response_ ( nullptr ), timeout_linker_ ( nullptr )
    {
        cout << "ctor player session..." << endl;
    }

    PlayerSession::~PlayerSession()
    {
        cout << "dtor player session..." << endl;       
        SAFE_DELETE ( timeout_linker_ );
		SAFE_DELETE ( response_ );
    }

    bool PlayerSession::Setup()
    {
		response_ = new Response ( *this );
        return true;
    }

    void PlayerSession::Exit()
    {
        client()->Close();
    }

    void PlayerSession::OnUserClientClose()
    {
        cout << "on user client close..." << endl;
        StopTimer();
        ready_ = false;
		g_sessionManager->DeleteSession(this);
    } 

    void PlayerSession::StartTimer()
    {
        SAFE_RELEASE ( timeout_linker_ );
        timeout_linker_ = event::Dispatcher::instance().quicktimer().SetIntervalWithLinker ( bind ( &PlayerSession::EvtUpdateTrigger , this ) , 5*60*1000 );
        timeout_linker_->Retain();
    }

    void PlayerSession::EvtUpdateTrigger()
    {
		LOG_WARN("EvtUpdateTrigger");
        evt_update.Trigger();
    }


    void PlayerSession::StopTimer()
    {
        SAFE_RELEASE ( timeout_linker_ );
    }



    void PlayerSession::OnUserClientReceivePacket ( PacketIn& pktin )
    {
        uint16_t code = pktin.code();
		LOG_WARN("OnUserClientReceivePacket(code:%d)",code);
        if ( code != 604 ) {
            cout << "recv pktin:" << code << endl;
        }
        try {
            pkthandler_map_t::iterator it = pkthandlers_.find ( pktin.code() );
            if ( it != pkthandlers_.end() ) {
                it->second ( pktin );
            } else {
                LOG_ERROR ( "unhandler pktin,code=%u\n" , pktin.code() );
            }
        } catch ( std::exception& ex ) {
            LOG_ERROR2 ( "recv fail:%s\n" , ex.what() );
        }
    }

	void PlayerSession::RegisterPacketHandler ( uint16_t code, pkthandler_t hd )
    {
        pair<pkthandler_map_t::iterator , bool> p = pkthandlers_.emplace ( code , hd );
        if ( !p.second ) {
            LOG_ERROR ( "duplicate packet handler when register with code=%u\n" , code );
        }
    }
}

