#include "usersession.h"
#include "userclient.h"

namespace base
{
    namespace gateway
    {
        UserSession::UserSession(UserClient* client)
            : client_(client)
        {
            client_->SetSession(this);
            client_->Retain();
        }

        UserSession::~UserSession()
        {
            SAFE_RELEASE(client_);
        }

        void UserSession::Send(PacketOut& pktout)
        {
            client_->Send(pktout);
        }
    }
}
