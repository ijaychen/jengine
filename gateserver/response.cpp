#include "response.h"
#include "playersession.h"
#include <base/gateway/packet.h>
#include <base/gateway/userclient.h>

namespace gateserver
{
    using namespace std;
    using namespace base;
    using namespace base::gateway;
   

#define MAKE_PKTOUT(code , approx_size) PacketOut pktout((uint16_t)code , approx_size , ps_.client()->mempool())
#define SEND(pktout) pktout.SetSessionID(session_); ps_.client()->Send(pktout)
#define SEND_SINGLE(pktout) ps_.client()->Send(pktout)

    ///Response

    Response::Response ( PlayerSession& ps )
        :ps_ ( ps ) , session_ ( 0 )
    {
    }


   /* void Response::SendPing ( uint16_t sequence )
    {
        MAKE_PKTOUT ( PING,30 );
        pktout << sequence;
        SEND ( pktout );
    }

    void Response::SendPingResult ( uint16_t delay )
    {
        MAKE_PKTOUT ( PING_RESULT,30 );
        pktout << delay;
        SEND ( pktout );
    }*/

    

    void Response::SendLogout ( uint8_t reason )
    {
        //MAKE_PKTOUT ( LOGOUT,30 );
		MAKE_PKTOUT ( 1,30 );
        pktout << reason;
        SEND ( pktout );
    }
}







