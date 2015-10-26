#ifndef BASE_CLUSTER_MAILBOX_H
#define BASE_CLUSTER_MAILBOX_H

#include "mailboxid.h"
#include <string>

namespace base
{
    namespace memory
    {
        class MemoryPool;
    }

    namespace cluster
    {
        class MessageOut;
        class MessageIn;
        class NodeMonitor;

        class Mailbox
        {
        public:
            struct EventHandler {
                virtual ~EventHandler() {}
                virtual void OnMessageReceive(MessageIn& msgin) = 0;
            };
            ~Mailbox();

            // 名称
            const std::string& name() const {
                return name_;
            }

            memory::MemoryPool& mempool() {
                return mempool_;
            }

            // 邮箱ID
            const MailboxID& mbid() const {
                return mbid_;
            }

            // 创建一个邮箱
            static Mailbox* Create(EventHandler& handler, const char* name = "", bool sys = false);

            // 给其它邮箱发送消息
            void Cast(const MailboxID& to, MessageOut& msgout);

        private:
            Mailbox(EventHandler& handler, const char* name);

            void HandleMessageReceive(MessageIn& msgin) {
                handler_.OnMessageReceive(msgin);
            }
            memory::MemoryPool& mempool_;
            EventHandler& handler_;
            MailboxID mbid_;
            std::string name_;
            friend class NodeMonitor;
        };

#define MSGOUT(code, approx_len) base::cluster::MessageOut msgout((uint16_t)code, approx_len, mempool())
    }
}

#endif // MAILBOX_H

