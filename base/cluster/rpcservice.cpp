#include "rpcservice.h"
#include "message.h"

namespace base
{
    namespace cluster
    {
        RpcService::RpcService(const char* service_name, bool sys)
            : mailbox_(nullptr), service_name_(service_name), sys_(sys)
        {
        }

        RpcService::~RpcService()
        {
            SAFE_DELETE(mailbox_);
        }

        bool RpcService::Setup()
        {
            mailbox_ = Mailbox::Create(*this, service_name_.c_str(), sys_);
            return mailbox_ != nullptr && OnSetup();
        }

        void RpcService::Reply(const MailboxID& from, uint16_t session, MessageOut& msgout)
        {
            msgout.SetSession(session);
            mailbox_->Cast(from, msgout);
        }

        void RpcService::OnMessageReceive(MessageIn& msgin)
        {
            if (msgin.session() != 0) {
                OnCall(msgin.from(), msgin.session(), msgin);
            } else {
                OnCast(msgin.from(), msgin);
            }
        }
    }
}

