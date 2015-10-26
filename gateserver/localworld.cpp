#include "localworld.h"
#include "response.h"
#include "rpc/worldstub.h"
#include <base/logger.h>
#include "playersession.h"
#include "comp/character.h"
#include "qo/commandquerypublicprofile.h"
#include "qo/commandqueryuidbynickname.h"
#include <base/cluster/message.h>
#include <base/cluster/nodemonitor.h>
#include <base/gateway/userclient.h>
#include <base/framework.h>
#include <base/event/dispatcher.h>
#include <boost/bind.hpp>
#include <model/rpc/csrpccode.h>
#include <model/tpl/templateloader.h>
#include <engine/game/map.h>
#include "comp/chat.h"
#include "comp/friend.h"

namespace fs
{
    using namespace std;
    using namespace rpc;
    using namespace boost;
    using namespace base;
    using namespace base::cluster;
    using namespace base::gateway;
    using namespace model::tpl;
    using namespace engine::game;

    LocalWorld* g_localworld = nullptr;

    LocalWorld::LocalWorld()
        :mailbox_ ( nullptr )
    {
        assert ( g_localworld == nullptr );
        g_localworld = this;
    }

    LocalWorld::~LocalWorld()
    {
        g_localworld = nullptr;
        SAFE_DELETE ( mailbox_ );
        for ( unordered_map<uint32_t , Player*>::iterator it = globals_.begin(); it != globals_.end(); ++it ) {
            delete it->second;
        }
        globals_.clear();
        for ( list<model::profile::BaseLite*>::iterator it = recent_worldchat_players_.begin(); it != recent_worldchat_players_.end(); ++it ) {
            delete *it;
        }
        recent_worldchat_players_.clear();
    }

    LocalWorld* LocalWorld::Create()
    {
        LocalWorld* localworld = new LocalWorld();
        if ( !localworld->Setup() ) {
            SAFE_DELETE ( localworld );
        }
        return localworld;
    }

    bool LocalWorld::Setup()
    {
        mailbox_ = Mailbox::Create ( *this , "" , true );
        if ( mailbox_ != nullptr ) {
            g_worldstub->Cast_RegisterLocalWorld ( mailbox_->mbid() );
        }
        //如果中心服务器停机，则关闭前端服务器
        maintainer_.Add ( NodeMonitor::instance().evt_node_down.Attach ( bind ( &LocalWorld::NodeDownEventHandler , this , _1 ) ) );
        //定时清理过期数据
        maintainer_.Add ( g_dispatcher->quicktimer().SetIntervalWithLinker ( bind ( &LocalWorld::CleanupExpirePlayer , this ) , 1000 * 60 * 5 ) );
        return mailbox_ != nullptr;
    }

    void LocalWorld::NodeDownEventHandler ( const NodeInfo& node )
    {
        if ( node.name == "cs" ) {
            LOG_WARN ( "frontServer will shutdown because the centerServer shutdown!\n" );
            framework.Stop();
        }
    }

    PlayerSession* LocalWorld::Create ( UserClient* client )
    {
        PlayerSession* session = new PlayerSession ( client );
        if ( session->Setup() ) {
            players_.push_front ( session );
            return session;
        } else {
            session->Release();
            LOG_ERROR ( "LocalWorld:player init failed!\n" );
            return nullptr;
        }
    }

    void LocalWorld::Delete ( PlayerSession* player )
    {
        if ( player->uid() != 0 ) {
            g_worldstub->Cast_Logout ( player->uid() );
        }
        players_.erase ( player );
        player->Release();

        //base::command::RunnerMgr::instance().Dump();
    }

    void LocalWorld::DisconnectAll()
    {
        PlayerSession* it = players_.front();
        while ( it ) {
            it->response().SendLogout ( 2 );
            it->Exit();
            it = it->list_next();
        }
    }

    void LocalWorld::UpdatePublicProfile ( uint32_t uid, const model::profile::Public& profile )
    {
        unordered_map<uint32_t , Player*>::iterator it = globals_.find ( uid );
        Player* p = nullptr;
        if ( it == globals_.end() ) {
            p = new Player ( uid );
            globals_.emplace ( uid , p );
        } else {
            p = it->second;
        }
        p->mutable_profile() = profile;
        g_worldstub->Cast_UpdatePublicProfile ( profile );
        //构建索引
        const string& nickname = p->mutable_profile().base.nickname;
        if ( !nickname.empty() ) {
            nickname_uid_index_[nickname] = uid;
        }
    }

    std::vector<const model::profile::Public*> LocalWorld::ExtractFriends ( uint32_t size, uint32_t & begin, PlayerSession& player )
    {
        uint32_t index=0;
        uint32_t end =index+size;
        if ( begin>globals_.size() ) begin=0;
        std::vector< const model::profile::Public*>  playerlist;
        for ( boost::unordered_map<uint32_t , Player*>::iterator it = globals_.begin() ; it!= globals_.end(); ++it ) {
            if ( index > begin && index < end  )
                if ( ! player.friends().IsFriends ( it->second->profile()->base.puid.uid ) ) {
                    playerlist.push_back ( it->second->profile() );
                }
            index++;
        }
        return playerlist;
    }


    void LocalWorld::QueryPlayerProfile ( uint32_t uid, base::CallbackObject< void ( const model::profile::Public* ) >* cb )
    {
        const model::profile::Public* p = QueryCachedPlayerPfofile ( uid );
        if ( p ) {
            cb->Retain();
            ( *cb ) ( p );
            cb->Release();
        } else {
            runner_.PushCommand ( new qo::CommandQueryPublicProfile ( uid , cb ) );
        }
    }

    void LocalWorld::QueryPlayerUID ( const string& nickname, base::CallbackObject< void ( uint32_t ) >* cb )
    {
        boost::unordered_map<std::string , uint32_t>::iterator it = nickname_uid_index_.find ( nickname );
        if ( it != nickname_uid_index_.end() ) {
            cb->Retain();
            ( *cb ) ( it->second );
            cb->Release();
        } else {
            runner_.PushCommand ( new qo::CommandQueryUIDByNickname ( nickname , cb ) );
        }
    }



    void LocalWorld::CleanupExpirePlayer()
    {
        int64_t now_tick = framework.GetTickCache();
        for ( unordered_map<uint32_t , Player*>::iterator it = globals_.begin(); it != globals_.end(); ) {
            //已经离线，且最近数据访问时间大于5分钟
            if ( !it->second->online() && now_tick - it->second->lru() > 1000*60*5 ) {
                delete it->second;
                it = globals_.erase ( it );
            } else {
                ++it;
            }
        }
        //TODO 清理昵称缓存，定期一个礼拜清理一次
    }

    void LocalWorld::OnMessageReceive ( MessageIn& msgin )
    {
        uint16_t code = msgin.code();
        cout << "localworld recv code = " << code << endl;
        switch ( code ) {
            case model::rpc::UPDATE_TO_ONLINE: {
                uint32_t uid = msgin.ReadUInt();
                MailboxID mbid;
                msgin >> mbid;
                Player* p = nullptr;
                unordered_map<uint32_t , Player*>::iterator it = globals_.find ( uid );
                if ( it == globals_.end() ) {
                    p = new Player ( uid );
                    globals_.emplace ( uid , p );
                } else {
                    p = it->second;
                }
                cout << "# player:" << uid << " online!" << endl;
                p->SetOnline ( mbid );
            }
            break;
            case model::rpc::UPDATE_TO_OFFLINE: {
                uint32_t uid = msgin.ReadUInt();
                unordered_map<uint32_t , Player*>::iterator it = globals_.find ( uid );
                if ( it!=globals_.end() ) {
                    cout << "# player:" << uid << " offline!" << endl;
                    it->second->SetOffline();
                }
            }
            break;
            case model::rpc::UPDATE_PUBLIC_PROFILE: {

            }
            break;
            case model::rpc::WORLD_CHATE: {  //世界频道聊天
                BroadcastWorldChat ( msgin );
            }
            break;
            default:
                LOG_ERROR ( "no handler for code %d!\n" , code );
                break;
        }
    }

    void LocalWorld::BroadcastWorldChat ( MessageIn& msgin )
    {
        uint32_t uid=msgin.ReadUInt();
        string nickname =msgin.ReadString() ;
        string msg=msgin.ReadString() ;
        uint32_t level=msgin.ReadUInt();
        uint32_t profession=msgin.ReadUInt();
        PushRecentWorldChatPlayer ( new model::profile::BaseLite ( uid, ( enum model::tpl::ProfessionType ) profession,nickname, level ) );
        PlayerSession* it = players_.front();
        while ( it ) {
            it->ready();  //ready之后...
            if ( it->chat().chat_switch.world==0 )
                continue ;
            it->response().WorldChat ( uid,nickname,msg,it->character().level(),it->character().info().table().profession );
            it = it->list_next();
        }
    }

    const model::profile::Public* LocalWorld::Player::FetchProfile()
    {
        if ( profile_!=nullptr ) {
            lru_ = framework.GetTickCache();
        }
        return profile_;
    }

}

