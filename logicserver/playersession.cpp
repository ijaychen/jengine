#include "playersession.h"
#include "response.h"
#include "delivery.h"
#include "localworld.h"
#include "rpc/worldstub.h"
#include <base/framework.h>
#include <base/gateway/userclient.h>
#include <base/logger.h>
#include <base/event/dispatcher.h>
#include <base/cluster/message.h>
#include <model/fsprotocol.h>
#include <boost/bind.hpp>
#include "comp/componentbase.h"
#include "comp/character.h"
#include "comp/ping.h"
#include "comp/dict.h"
#include "comp/bag.h"
#include "comp/chat.h"
#include "comp/map.h"
#include "comp/game.h"
#include "comp/mail.h"
#include "comp/spell.h"
#include "comp/property.h"
#include "comp/cdlist.h"
#include "comp/buff.h"
#include "comp/friend.h"
#include "comp/polycopper.h"
#include "comp/task.h"
#include "comp/qqopenapi.h"

namespace fs
{
    using namespace std;
    using namespace base;
    using namespace base::cluster;
    using namespace base::gateway;
    using namespace command;
    using namespace comp;
    using namespace boost;

    PlayerSession::PlayerSession ( UserClient* uc )
        : UserSession ( uc ), executor_ ( *this ) , cursor_ ( 0 ) , mb_ ( nullptr ) ,
          response_ ( nullptr ) , delivery_ ( nullptr ) , character_ ( nullptr ) ,  uid_ ( 0 ) , timeout_linker_ ( nullptr )
    {
        cout << "ctor player session..." << endl;
    }

    PlayerSession::~PlayerSession()
    {
        cout << "dtor player session..." << endl;
        for ( vector<ComponentBase*>::iterator it = components_.begin(); it!=components_.end(); ++it ) {
            delete *it;
        }
        components_.clear();
        SAFE_DELETE ( response_ );
        SAFE_DELETE ( mb_ );
        SAFE_DELETE ( timeout_linker_ );
    }

    bool PlayerSession::Setup()
    {
        response_ = new Response ( *this );
        delivery_ = new Delivery ( *this );
        mb_ = Mailbox::Create ( *this );
        //按依赖关系添加组件
        components_.push_back ( new comp::Ping ( *this ) );
        components_.push_back ( new comp::Character ( *this ) );
        components_.push_back ( new comp::QqOpenApi ( *this ) );
        components_.push_back ( new comp::Dict ( *this ) );
        components_.push_back ( new comp::Spell ( *this ) );
        components_.push_back ( new comp::Buff ( *this ) );
        components_.push_back ( new comp::Bag ( *this ) );
        components_.push_back ( new comp::Friend ( *this ) );
        components_.push_back ( new comp::Property ( *this ) );
        components_.push_back ( new comp::Map ( *this ) );
        components_.push_back ( new comp::Game ( *this ) );
        components_.push_back ( new comp::Chat ( *this ) );
        components_.push_back ( new comp::Mail ( *this ) );
        components_.push_back ( new comp::CDList ( *this ) );
        components_.push_back ( new comp::PolyCopper ( *this ) );
        components_.push_back ( new comp::Task ( *this ) );
        //开始组件启动
        components_[cursor_]->BeginSetup();
        return true;
    }

    void PlayerSession::Exit()
    {
        flags().set ( ( size_t ) Terminate );
        client()->Close();
    }

    void PlayerSession::OnUserClientClose()
    {
        cout << "on user client close..." << endl;
        StopTimer();
        ready_ = false;
        if ( cursor_ == components_.size() ) {
            //开始反向组件关闭
            --cursor_;
            components_.back()->BeginCleanup();
        } else {
            //未完成初始话不需要清理，直接退出
            g_localworld->Delete ( this );
        }
    }

    void PlayerSession::OnCompSetup ( ComponentBase* c )
    {
        cout << "on component setup:" << c->comp_name() << endl;
        //建立快速定位
        if ( c->comp_name() == "ping" ) {
            ping_ = static_cast<comp::Ping*> ( c );
        } else if ( c->comp_name() == "character" ) {
            character_ = static_cast<comp::Character*> ( c );
            uid_ = character_->uid();
        } else if ( c->comp_name() == "qqapi" ) {
            qqapi_ = static_cast<comp::QqOpenApi*> ( c );
        }  else if ( c->comp_name() == "dict" ) {
            dict_ = static_cast<comp::Dict*> ( c );
        }  else if ( c->comp_name() == "bag" ) {
            bag_ = static_cast<comp::Bag*> ( c );
        } else if ( c->comp_name() == "map" ) {
            map_ = static_cast<comp::Map*> ( c );
        } else if ( c->comp_name() == "game" ) {
            game_ = static_cast<comp::Game*> ( c );
        } else if ( c->comp_name() == "spell" ) {
            spell_ = static_cast<comp::Spell*> ( c );
        } else if ( c->comp_name() == "buff" ) {
            buff_ = static_cast<comp::Buff*> ( c );
        } else if ( c->comp_name() == "property" ) {
            property_ = static_cast<comp::Property*> ( c );
        } else if ( c->comp_name() == "chat" ) {
            chat_ = static_cast<comp::Chat*> ( c );
        } else if ( c->comp_name() == "mail" ) {
            mail_ = static_cast<comp::Mail*> ( c );
        } else if ( c->comp_name() == "cdlist" ) {
            cdlist_ = static_cast<comp::CDList*> ( c );
        } else if ( c->comp_name() == "friend" ) {
            friend_ = static_cast<comp::Friend*> ( c );
        }  else if ( c->comp_name() == "poly" ) {
            poly_ = static_cast<comp::PolyCopper*> ( c );
        }  else if ( c->comp_name() == "task" ) {
            task_ = static_cast<comp::Task*> ( c );
        } else {
            LOG_ERROR ( "unknown component with name=%s\n" , c->comp_name().c_str() );
        }
        ++cursor_;

        if ( cursor_ == components_.size() ) {
            //TODO 以下代码做成单独的函数OnXX
            //OK,所有组件已经成功启动
            cout << "^^^^^^^^^" << endl;
            ready_ = true;
            rpc::g_worldstub->Cast_LoginEnd ( uid_ , mb_->mbid() );

            StartTimer();
        } else {
            components_[cursor_]->BeginSetup(); //启动下一个组件
        }
    }

    void PlayerSession::OnCompCleanup ( ComponentBase* c )
    {
        cout << "on component cleanup:" << c->comp_name() << endl;
        //检查是否所有组件都已关闭
        if ( c->comp_name() == "ping" ) {
            ping_ = nullptr;
        } else if ( c->comp_name() == "character" ) {
            character_ = nullptr;
        } else if ( c->comp_name() == "qqapi" ) {
            qqapi_ = nullptr;
        } else if ( c->comp_name() == "dict" ) {
            dict_ = nullptr;
        } else if ( c->comp_name() == "bag" ) {
            bag_ = nullptr;
        } else if ( c->comp_name() == "map" ) {
            map_ = nullptr;
        } else if ( c->comp_name() == "game" ) {
            game_ = nullptr;
        } else if ( c->comp_name() == "spell" ) {
            spell_ = nullptr;
        } else if ( c->comp_name() == "buff" ) {
            buff_ = nullptr;
        } else if ( c->comp_name() == "property" ) {
            property_ = nullptr;
        } else if ( c->comp_name() == "chat" ) {
            chat_ = nullptr;
        } else if ( c->comp_name() == "mail" ) {
            mail_ = nullptr;
        } else if ( c->comp_name() == "cdlist" ) {
            cdlist_ = nullptr;
        }  else if ( c->comp_name() == "friend" ) {
            friend_ = nullptr;
        } else if ( c->comp_name() == "poly" ) {
            poly_ = nullptr;
        } else if ( c->comp_name() == "task" ) {
            task_ = nullptr;
        } else {
            LOG_ERROR ( "unknown component with name=%s!\n" , c->comp_name().c_str() );
        }
        if ( cursor_ == 0 ) {
            //OK 所有组件都已经成功退出
            g_localworld->Delete ( this );
        } else {
            --cursor_;
            components_[cursor_]->BeginCleanup();
        }
    }

    void PlayerSession::StartTimer()
    {
        SAFE_RELEASE ( timeout_linker_ );
        timeout_linker_ = event::Dispatcher::instance().quicktimer().SetIntervalWithLinker ( bind ( &PlayerSession::EvtUpdateTrigger , this ) , 5*60*1000 );
        timeout_linker_->Retain();
    }

    void PlayerSession::EvtUpdateTrigger()
    {
        evt_update.Trigger();
    }


    void PlayerSession::StopTimer()
    {
        SAFE_RELEASE ( timeout_linker_ );
    }

    bool PlayerSession::IsAllCompSetup() const
    {
        for ( vector<ComponentBase*>::const_iterator it = components_.begin(); it!=components_.end(); ++it ) {
            if ( ( *it )->state() != OK ) {
                return false;
            }
        }
        return true;
    }

    void PlayerSession::OnUserClientReceivePacket ( PacketIn& pktin )
    {
        uint16_t code = pktin.code();
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

    void PlayerSession::OnMessageReceive ( cluster::MessageIn& msgin )
    {
        msghandler_map_t::iterator it = msghandlers_.find ( msgin.code() );
        if ( it != msghandlers_.end() ) {
            it->second ( msgin );
        } else {
            LOG_ERROR ( "unhandler message,code=%u\n" , msgin.code() );
        }
    }

    void PlayerSession::RegisterPacketHandler ( uint16_t code, pkthandler_t hd )
    {
        pair<pkthandler_map_t::iterator , bool> p = pkthandlers_.emplace ( code , hd );
        if ( !p.second ) {
            LOG_ERROR ( "duplicate packet handler when register with code=%u\n" , code );
        }
    }

    void PlayerSession::RegisterMessageHandler ( uint16_t code, msghandler_t hd )
    {
        pair<msghandler_map_t::iterator,bool> p = msghandlers_.emplace ( code , hd );
        if ( !p.second ) {
            LOG_ERROR ( "duplicate message handler when register with code=%u\n" , code );
        }
    }
}

