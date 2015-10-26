#ifndef RESPONSE_H
#define RESPONSE_H

#include <base/global.h>
#include <model/tpl/metadata.h>
#include <string>
#include <vector>
#include <deque>
#include <list>
#include <map>

namespace model
{
    namespace profile
    {
        struct Base;
        struct Public;
        struct BaseLite;

    }

    namespace tpl
    {
        struct DropItem;
        struct RandomAttribute;
    }

    class CharacterInfo;
    class ItemInfo;
    class MailInfo;
    class SpellInfo;
    class Property;
    class CDInfo;
    class BuffInfo;
    class FriendInfo;
    class GuildFriend;
    class PolyCopperRecord;
    class TaskInfo;
}

namespace fs
{
    class PlayerSession;
    class Response
    {
    public:
        Response ( PlayerSession& ps );
        ~Response() {};

        void SetSession ( uint16_t session ) {
            session_ = session;
        }
        void SendLoginResponse ( bool result );
        void SendPing ( uint16_t sequence );
        void SendPingResult ( uint16_t delay );
        //@reason 1.帐号在别处登录 2.服务器维护 3.检测到异常操作 4.内部错误
        void SendLogout ( uint8_t reason );
        void SendNotice ( const char* msg , model::tpl::NoticeType type = model::tpl::nWarn );
        void SendNotice ( const std::string& msg , model::tpl::NoticeType type = model::tpl::nWarn ) {
            SendNotice ( msg.c_str() , type );
        }
        void SendExpUpdate ( uint32_t exp );
        void SendEnergyUpdate ( uint16_t energy );
        void SendCharacterLevelUp ( uint16_t level );
        void SendCharacterUpdate ( const model::CharacterInfo& info );
        void SendWalletUpdate ( const model::CharacterInfo& info );
        void SendPlayerSpeedUpdate ( uint16_t speed );
        void SendNickNameUpdate ( uint16_t result );

        void SendBagUpdate ( const std::vector<const model::ItemInfo*>& items , const std::vector<uint16_t>& removelist );
        void SendBagUpdateOpenCount ( uint16_t count );
        void SendBagGridUnlock ( uint8_t result );

        void SendPropertyUpdate ( const model::Property& property );

        void SendCDListUpdate ( const std::vector<const model::CDInfo*> updates );

        void SendSpellUpdate ( const model::SpellInfo& spell , const std::vector<int32_t>& pendings );
        void SendSpellBarUpdate ( const model::SpellInfo& spell );

        void SendBuffUpdate ( const std::vector<const model::BuffInfo*>& updates , const std::vector<int32_t>& removelist );

        void SendWorkshopStrength ( bool result );
        void SendWorkshopStrengthReplace ( bool result );
        void SendWorkshopInlay ( bool result );
        void SendWorkshopGemRemove ( uint64_t gem_itemid );
        void SendWorkshopWashResultUpdate ( uint64_t itemid , const std::vector<model::tpl::RandomAttribute>& attrs );
        void SendWorkshopMake ( bool result );

        //聊天
        void PrivateChat ( uint32_t uid,std::string nickname,std::string msg );
        void WorldChat ( uint32_t uid,std::string nickname,std::string msg ,uint32_t level,model::tpl::ProfessionType  profession );
        //邮件
        void SendMailRrivateResponese ( bool result,int8_t msg=0  );
        void SendMailCount ( uint32_t count );
        void SendMailDrawAttachmentResponse ( uint32_t eid,uint32_t type );
        void SendMailList ( int32_t& begin,int32_t& end,  std::vector<model::MailInfo>& mlist,int32_t size ,int8_t box_type );
        void ReadMail ( model::MailInfo& info,std::vector<const model::ItemInfo*>& item );
        void SendMailDelResult ( std::vector<uint32_t>& fresult );
        void SendMailDelResult ( std::map<uint32_t, int8_t>& fresult );
        //好友系统  --组
        void SendFriendGroudAddResponse ( uint32_t result );
        void SendFriendGroudDelResponse ( std::vector< uint8_t > fail_gids );
        void SendFriendGroudUpdateResponse ( uint32_t result );
        //好友系统
        void SendFriendAddResponse ( uint32_t result );                                 //你的好友申请是否正常
        void SendFriendCallAddResponse ( uint32_t uid, const std::string& nickname );   //某人想加你为好友
        void SendFriendUpdateResponse ( uint32_t result );
        void SendFriendDeleteMsg ( uint32_t result );
        void SendFriendList ( std::vector<model::GuildFriend>&  friends );
        void SendFriendApplyPage ( ushort& begin,ushort& end,  std::vector< model::FriendInfo* >& mlist ,ushort size );
        void SendFriendApplyResultResponse ( uint32_t result , uint32_t uid, const std::string& nickname ,uint8_t p ,uint16_t level ); //审批好友申请
        void SendOnlineNotice ( uint32_t uid );         //TODO  抽出来
        void SendMoveFriendGroudNotice ( uint8_t result );
        void SendFrindSettingNotice ( uint8_t uid );
        void SendFriendOneKeySelect ( std::vector<const model::profile::Public*> players );
        void SendEnmityResponse ( uint32_t uid,const std::string& nikename,ushort level,model::tpl::ProfessionType p );

        //煮铜鼎
        void SendTripodListResponse ( std::vector<model::PolyCopperRecord> records );
        void SendTripodResponse ( bool cd, uint32_t copper_coin ,uint32_t exp );
        void SendPolyReceive ( uint32_t nextlevel,uint32_t nextgold );
        void SendPolyFriends ( std::vector<model::FriendInfo*> );

        //任务系统
        void SendTaskUpdate ( std::vector<model::TaskInfo>&  task );

    private:
        PlayerSession& ps_;
        uint16_t session_;
    };
}
#endif // RESPONSE_H
