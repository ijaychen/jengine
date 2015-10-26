#include "rpcstub.h"
#include "message.h"
#include "nodemonitor.h"
#include <boost/bind.hpp>

namespace base
{
    namespace cluster
    {
        using namespace std;
        using namespace base;

        RpcStub::RpcStub(const char* service_name) : mailbox_(nullptr), session_cur_(0), service_name_(service_name)
        {
        }

        RpcStub::~RpcStub()
        {
            SAFE_DELETE(mailbox_);
        }

        bool RpcStub::BeginSetup(boost::function<void()> cb)
        {
            mailbox_ = Mailbox::Create(*this);
            if (mailbox_ == nullptr) {
                return false;
            }
            cb_setup_ = cb;
            NodeMonitor::instance().FetchNamedMailbox(service_name_, boost::bind(&RpcStub::OnServiceUp, this, _1));
            return true;
        }

        void RpcStub::OnServiceUp(const MailboxID& mbid)
        {
            mbid_service_ = mbid;
            cb_setup_();
        }

        RpcCallLinker* RpcStub::Call(MessageOut& msgout, rpc_callback_t cb)
        {
            msgout.SetSession(GenSessionID());
            RpcCall call(cb, new RpcCallLinker);
            calls_.insert(make_pair(msgout.session(), call));
            mailbox_->Cast(mbid_service_, msgout);
            return call.linker;
        }

        void RpcStub::CallWithoutLinker(MessageOut& msgout, rpc_callback_t cb)
        {
            msgout.SetSession(GenSessionID());
            RpcCall call(cb);
            calls_.insert(make_pair(msgout.session(), call));
            mailbox_->Cast(mbid_service_, msgout);
        }

        void RpcStub::Cast(MessageOut& msgout)
        {
            msgout.SetSession(0);
            mailbox_->Cast(mbid_service_, msgout);
        }

        void RpcStub::OnMessageReceive(MessageIn& msgin)
        {
            call_map_t::iterator it = calls_.find(msgin.session());
            if (it != calls_.end()) {
                if ((*it).second.linker == nullptr || (*it).second.linker->reference_count() == 2) {
                    (*it).second.fun(msgin);
                }
                calls_.erase(it);
            }
        }
    }
}
