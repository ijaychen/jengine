#ifndef PLAYERSESSION_H
#define PLAYERSESSION_H

#include <base/event.h>
#include <base/timer.h>
#include <base/cluster/mailbox.h>
#include <base/gateway/usersession.h>
#include <base/command/runner.h>
#include <boost/function.hpp>
#include <boost/unordered_map.hpp>
#include <base/utils/intrusive_list.h>
#include <bitset>
namespace gateserver
{
	class Response;
    typedef boost::function<void ( base::gateway::PacketIn& ) > pkthandler_t;
    typedef boost::unordered_map<uint16_t , pkthandler_t> pkthandler_map_t;

    class PlayerSession : public base::gateway::UserSession
    {
        INTRUSIVE_LIST ( PlayerSession )
    public:
        PlayerSession ( base::gateway::UserClient* uc );
        virtual ~PlayerSession();
		Response& GetResponse()
		{
			return *response_;
		}
        uint32_t uid() const {
            return uid_;
        }

		//每五分钟执行一次更新
		base::Event<void() > evt_update;

        //初始化
        bool Setup();

#define PACKET_MAP(code , fun) ps().RegisterPacketHandler((uint16_t)code, boost::bind(&fun,this,_1))
        void RegisterPacketHandler ( uint16_t code , pkthandler_t hd );

        //强制退出
        void Exit();
      
    private:
        virtual void OnUserClientReceivePacket ( base::gateway::PacketIn& pktin );
        virtual void OnUserClientClose();

    private:
        void StartTimer();
        void EvtUpdateTrigger();
        void StopTimer();

    private:

        pkthandler_map_t pkthandlers_;
		Response* response_;

      
        uint32_t uid_;
        bool ready_; //TODO 可以考虑整合重构ready_,flags_等玩家标志

        base::TimeoutLinker* timeout_linker_;
    };
}
#endif // PLAYERSESSION_H
