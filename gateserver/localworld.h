#ifndef LOCALWORLD_H
#define LOCALWORLD_H

#include "playersession.h"
#include <base/utils/intrusive_list.h>
#include <base/objectmaintainer.h>
#include <model/profile/public.h>
#include <base/timer.h>
#include <base/cluster/mailbox.h>
#include <base/command/runner.h>
#include <boost/unordered_map.hpp>
#include <base/callback.h>

namespace base
{
    namespace cluster
    {
        class NodeInfo;
    }
}
namespace engine
{
    namespace game
    {
        class Map;
    }
}
namespace fs
{
    //当前服务器玩家管理
    class LocalWorld : public base::cluster::Mailbox::EventHandler
    {
    public:
        ~LocalWorld();
        static LocalWorld* Create();

        //当前服务器连接数
        std::size_t size() const {
            return players_.size();
        }
        //最近在世界频道发言的玩家
        const std::list<model::profile::BaseLite*>& recent_worldchat_players() const {
            return recent_worldchat_players_;
        }

        //创建一个玩家会话
        PlayerSession* Create ( base::gateway::UserClient* client );
        //删除一个玩家会话
        void Delete ( PlayerSession* player );
        //断开所有玩家
        void DisconnectAll();
        //更新公共资料
        void UpdatePublicProfile ( uint32_t uid , const model::profile::Public& profile );
        //查询玩家公共资料（仅缓存）
        const model::profile::Public* QueryCachedPlayerPfofile ( uint32_t uid ) const {
            boost::unordered_map<uint32_t , Player*>::const_iterator it = globals_.find ( uid );
            return it != globals_.end() ? it->second->FetchProfile() : nullptr;
        }
        //查询玩家公共资料
        void QueryPlayerProfile ( uint32_t uid , base::CallbackObject< void ( const model::profile::Public* ) >* cb );
        //根据玩家昵称查询玩家ＵＩＤ（仅缓存）
        uint32_t QueryCachedPlayerUID ( const std::string& nickname ) const {
            boost::unordered_map<std::string , uint32_t>::const_iterator it = nickname_uid_index_.find ( nickname );
            return it == nickname_uid_index_.end() ? 0 : it->second;
        }
        //根据玩家昵称查询玩家ＩＤ
        void QueryPlayerUID ( const std::string& nickname , base::CallbackObject<void ( uint32_t ) >* cb );
        //查询玩家邮箱ID
        const base::cluster::MailboxID* QueryPlayerMailboxID ( uint32_t uid ) const {
            boost::unordered_map<uint32_t , Player*>::const_iterator it = globals_.find ( uid );
            if ( it != globals_.end() && it->second->online() ) {
                return & ( it->second->mbid() );
            }
            return nullptr;
        }

        void PushRecentWorldChatPlayer ( model::profile::BaseLite* profile ) {
            if ( recent_worldchat_players_.size() >=5 ) {
                recent_worldchat_players_.pop_back();
            }
            for ( std::list< model::profile::BaseLite * > ::iterator it = recent_worldchat_players_.begin(); it != recent_worldchat_players_.end(); ++ it ) {
                if ( ( * it )->uid==profile->uid ) {
                    return;
                }
            }
            recent_worldchat_players_.push_front ( profile );
        }
        std::vector<const model::profile::Public*> ExtractFriends ( uint32_t size ,uint32_t& begin , PlayerSession& player ) ;

    private:
        virtual void OnMessageReceive ( base::cluster::MessageIn& msgin );
    private:
        LocalWorld();
        bool Setup();
        void CleanupExpirePlayer();
        void BroadcastWorldChat ( base::cluster::MessageIn& msgin );
        void NodeDownEventHandler ( const base::cluster::NodeInfo& node );

        base::utils::IntrusiveList<PlayerSession> players_;
        base::cluster::Mailbox* mailbox_;
        base::ObjectMaintainer maintainer_;

        class Player
        {
        public:
            Player ( uint32_t uid ) : uid_ ( uid ) , lru_ ( 0 ) , profile_ ( nullptr ) {}
            ~Player() {
                SAFE_DELETE ( profile_ );
            }

            int64_t lru() const {
                return lru_;
            }
            const base::cluster::MailboxID& mbid() const {
                return mbid_;
            }
            bool online() const {
                return mbid_;
            }
            const model::profile::Public* profile() const {
                return profile_;
            }
            model::profile::Public& mutable_profile() {
                if ( profile_ == nullptr ) {
                    profile_ = new model::profile::Public;
                    profile_->base.puid.uid = uid_;
                }
                return *profile_;
            }
            const model::profile::Public* FetchProfile();
            void SetOnline ( const base::cluster::MailboxID& mbid ) {
                mbid_ = mbid;
            }
            void SetOffline() {
                mbid_.Clear();
            }
        private:
            uint32_t uid_;
            base::cluster::MailboxID mbid_;
            int64_t lru_; //最近使用时间
            model::profile::Public* profile_;
        };

        boost::unordered_map<uint32_t , Player*> globals_; //全局所有玩家
        boost::unordered_map<std::string , uint32_t> nickname_uid_index_; //昵称与uid之间的索引 //TODO （注意改名时重建）
        base::command::Runner runner_;

        std::list<model::profile::BaseLite*> recent_worldchat_players_;  //最近在世界频道发言的玩家

    };
    extern LocalWorld* g_localworld;
}
#endif // LOCALWORLD_H
