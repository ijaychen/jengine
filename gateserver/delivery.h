#ifndef DELIVERY_H
#define DELIVERY_H

#include "playersession.h"
#include "string"

namespace base
{
    namespace memory
    {
        class MemoryPool;
    }
}

namespace fs
{
    class Delivery
    {
    public:
        Delivery ( PlayerSession& ps );
        virtual ~Delivery() {}

        //1对1聊天
        void CastChatPrivate ( const base::cluster::MailboxID& to , const std::string& content );
        //上线通知，用于friend组件、guild组件等
        void CastOnlineNotify ( const base::cluster::MailboxID& to );
        //下线通知
        void CastOfflineNotify ( const base::cluster::MailboxID& to );
        //添加好友
        void CastFriendApply ( const base::cluster::MailboxID& to );
        //同意添加好友
        void CastFriendAccept ( const base::cluster::MailboxID& to );
        //删除好友
        void CastFriendDelete ( const base::cluster::MailboxID& to );
        //通知有新邮件
        void CastNewMailNotify ( const base::cluster::MailboxID& to , const uint32_t& uid );
        //通知发件人:本人邮件数超过500,可能接收不到
        void CastSenderMsg ( const base::cluster::MailboxID& to ,const std::string& count );
        //通知申请人:本人的好友情况  0:成功  1:失败  2: 已是好友  3:本人好友列表已满 4: 你在对方黑名单中
        void CastFriendApplyResultMsg ( const base::cluster::MailboxID& to ,uint32_t result );
        //好友申请审批结果
        void CastFriendAddActionMsg ( const base::cluster::MailboxID& to ,uint32_t result );
        //设置黑名单通知
        void CastSetBanNotify ( const base::cluster::MailboxID& to );
    private:
        base::memory::MemoryPool& mempool() {
            return ps_.mb().mempool();
        }
        void Cast ( const base::cluster::MailboxID& to , base::cluster::MessageOut& msgout ) {
            ps_.mb().Cast ( to , msgout );
        }
    private:
        PlayerSession& ps_;
    };
}
#endif // DELIVERY_H
