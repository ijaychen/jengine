#ifndef BASE_CLUSTER_RPCSERVICE_H
#define BASE_CLUSTER_RPCSERVICE_H

#include "mailbox.h"

namespace base
{
    namespace cluster
    {
        class Mailbox;

        class RpcService : Mailbox::EventHandler
        {
        public:
            RpcService(const char* service_name, bool sys = false);
            virtual ~RpcService();

            const std::string& service_name() const {
                return service_name_;
            }

            bool Setup();

        protected:
            memory::MemoryPool& mempool() {
                return mailbox_->mempool();
            }
            Mailbox& self() {
                return *mailbox_;
            }

            void Reply(const MailboxID& from, uint16_t session, MessageOut& msgout);

        private:
            virtual bool OnSetup() {
                return true;
            }
            virtual void OnCall(const MailboxID& from, uint16_t session, MessageIn& msgin) = 0;
            virtual void OnCast(const MailboxID& from, MessageIn& msgin) = 0;
            virtual void OnMessageReceive(MessageIn& msgin);

            Mailbox* mailbox_;
            std::string service_name_;
            bool sys_;
        };
    }
}

#endif // RPCSERVICE_H
