#include "delivery.h"
#include "comp/character.h"
#include <model/rpc/csrpccode.h>
#include <base/cluster/message.h>
namespace fs
{
    using namespace std;
    using namespace base::cluster;
    using namespace model;

    Delivery::Delivery ( PlayerSession& ps )
        :ps_ ( ps )
    {

    }

    void Delivery::CastChatPrivate ( const MailboxID& to, const string& content )
    {
        MSGOUT ( rpc::CHAT_PRIVATE , 50 );
        msgout << ps_.character().info().uid() << ps_.character().info().nickname() << content;
        Cast ( to , msgout );
    }

    void Delivery::CastOnlineNotify ( const MailboxID& to )
    {
        MSGOUT ( rpc::ONLINE_NOTIFY , 40 );
        msgout << ps_.character().info().uid()  ;
        Cast ( to , msgout );
    }

    void Delivery::CastOfflineNotify ( const MailboxID& to )
    {
        MSGOUT ( rpc::OFFLINE_NOTIFY , 40 );
        msgout << ps_.character().info().uid();
        Cast ( to , msgout );
    }

    void Delivery::CastFriendApply ( const MailboxID& to )
    {
        MSGOUT ( rpc::FRIENDAPPLY , 40 );
        msgout << ps_.uid() <<ps_.character().info().nickname() << ( uint8_t ) ps_.character().info().profession() <<ps_.character().info().table().level  ;
        Cast ( to , msgout );
    }

    void Delivery::CastFriendAccept ( const MailboxID& to )
    {
        MSGOUT ( rpc::FRIEND_ACCEPT_NOTIFY , 40 );
        msgout << ps_.character().info().uid();
        Cast ( to , msgout );
    }

    void Delivery::CastFriendDelete ( const MailboxID& to )
    {
        MSGOUT ( rpc::FRIEND_DELETE_NOTIFY , 40 );
        msgout << ps_.character().info().uid();
        Cast ( to , msgout );
    }

    void Delivery::CastNewMailNotify ( const MailboxID& to, const uint32_t& uid )
    {
        MSGOUT ( rpc::NEW_MAIL , 40 );
        msgout << uid ;
        Cast ( to , msgout );
    }
    void Delivery::CastSenderMsg ( const MailboxID& to, const string& msg )
    {
        MSGOUT ( rpc::MAIL_SENDER , 40 );
        msgout << msg;
        Cast ( to , msgout );
    }

    void Delivery::CastFriendApplyResultMsg ( const MailboxID& to, uint32_t result )
    {
        MSGOUT ( rpc::FRIENDAPPLYRESULT , 40 );
        msgout << result;
        Cast ( to , msgout );
    }

    void Delivery::CastFriendAddActionMsg ( const MailboxID& to, uint32_t result )
    {
        MSGOUT ( rpc::FRIEND_APPLY_NOTIFY , 40 );
        msgout<< ( uint8_t ) result<<ps_.uid() <<ps_.character().nickname() <<(int8_t)ps_.character().info().profession()<<(uint16_t)ps_.character().level();
        Cast ( to , msgout );
    }

    void Delivery::CastSetBanNotify ( const MailboxID& to )
    {
        MSGOUT ( rpc::SETBANNOTIFY , 40 );
        msgout<<ps_.uid() ;
        Cast ( to , msgout );
    }

}

