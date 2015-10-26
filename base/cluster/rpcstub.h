#ifndef BASE_CLUSTER_RPCSTUB_H
#define BASE_CLUSTER_RPCSTUB_H

#include "mailbox.h"
#include "../object.h"
#include <boost/function.hpp>
#include <boost/unordered_map.hpp>

namespace base
{
    namespace cluster
    {
        typedef boost::function<void(MessageIn&)> rpc_callback_t;

        class RpcCallLinker : public Object {};

        class RpcStub : public Mailbox::EventHandler
        {
        public:
            RpcStub(const char* service_name);
            ~RpcStub();

            bool ready() const {
                return mailbox_;
            }

            memory::MemoryPool& mempool() {
                return mailbox_->mempool();
            }

            bool BeginSetup(boost::function<void()> cb);

            RpcCallLinker* Call(MessageOut& msgout, rpc_callback_t cb);
            void CallWithoutLinker(MessageOut& msgout, rpc_callback_t cb);
            void Cast(MessageOut& msgout);

        private:
            virtual void OnMessageReceive(MessageIn& msgin);

        private:
            void OnServiceUp(const MailboxID& mbid) ;
            uint16_t GenSessionID() {
                uint16_t ret = session_cur_++;
                if (ret == 0) {
                    ret = session_cur_++;
                }
                return ret;
            }

            struct RpcCall {
                rpc_callback_t fun;
                RpcCallLinker* linker;
                RpcCall(const rpc_callback_t& cb, RpcCallLinker* _linker = nullptr) : fun(cb), linker(_linker) {}
                RpcCall(const RpcCall& rhs) : fun(rhs.fun), linker(rhs.linker) {
                    if (linker) {
                        linker->Retain();
                    }
                }
                const RpcCall& operator = (const RpcCall& rhs) {
                    fun = rhs.fun;
                    linker = rhs.linker;
                    if (linker) {
                        linker->Retain();
                    }
                    return *this;
                }
                ~RpcCall() {
                    SAFE_RELEASE(linker);
                }
            };

            Mailbox* mailbox_;
            uint16_t session_cur_;
            MailboxID mbid_service_;
            std::string service_name_;
            
            typedef boost::unordered_map<uint16_t, RpcCall> call_map_t;
            call_map_t calls_;
            
            boost::function<void()> cb_setup_;
        };
    }
}

#endif // RPCSTUB_H
