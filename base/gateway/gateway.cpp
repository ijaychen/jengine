#include "gateway.h"
#include "usersession.h"

namespace base
{
    namespace gateway
    {
        Gateway::~Gateway()
        {
            SAFE_RELEASE(listener_);
        }

        void Gateway::OnUserClientConnect(ClientPtr client)
        {
            clients_.insert(client);
            if (!handler_.HandleAcceptUserClient(client)) {
                client->Close();
            }
        }

        void Gateway::OnUserClientClose(ClientPtr client)
        {
            if (client->session()) {
                client->session()->OnUserClientClose();
            }
            handler_.OnUserClientClose(client);
            if (client->session()) {
                client->ResetSession();
            }
            size_t r = clients_.erase(client);
            debug_assert(r == 1);
            client->Release();
            CheckIfCleanupFinish();
        }

        bool Gateway::Setup(const char* ipaddr, int port, uint32_t max_connections)
        {
            max_connections_ = max_connections;
            listener_ = new net::Listener(*this);
            return listener_->Bind(ipaddr, port);
        }

        bool Gateway::SetupByConfigFile()
        {
			/*
            Configure conf;
            if (!conf.Parse()) {
                return false;
            }
            return Setup(conf.gatewayinfo().ip.c_str(), conf.gatewayinfo().port, conf.gatewayinfo().max_connections);
			*/
			return true;
        }

        void Gateway::OnListenerAccept(net::Listener* sender, int clientfd)
        {
            if (clients_.size() >= max_connections_) {
                LOG_WARN("gateway refuse connect because the maximum number of connections has been exceeded!\n");
                close(clientfd);
            } else {
                UserClient* client = new UserClient(*this, mempool_);
                client->Connect(clientfd);
            }
        }

        void Gateway::BeginCleanup(boost::function<void()> cb)
        {
            is_cleanup_ = true;
            cleanup_finsh_cb_ = cb;
            StopListener();
            for (client_set_t::iterator it = clients_.begin(); it != clients_.end(); ++it) {
                (*it)->Close();
            }
            CheckIfCleanupFinish();
        }

        void Gateway::Broadcast(PacketOut& pktout)
        {
            for (client_set_t::iterator it = clients_.begin(); it != clients_.end(); ++it) {
                (*it)->Send(pktout);
            }
        }
        
        void Gateway::StopListener()
        {
            if (listener_) {
                listener_->Close();
                SAFE_RELEASE(listener_);
            }
        }
    }
}
