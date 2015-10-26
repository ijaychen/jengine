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
namespace fs
{
    class Response;
    class Delivery;
    namespace comp
    {
        class ComponentBase;
        class Character;
        class Ping;
        class Dict;
        class Bag;
        class Chat;
        class Map;
        class Mail;
        class Game;
        class Property;
        class Spell;
        class CDList;
        class Buff;
        class Friend;
        class PolyCopper;
        class Task;
        class QqOpenApi;
    }

    typedef boost::function<void ( base::gateway::PacketIn& ) > pkthandler_t;
    typedef boost::unordered_map<uint16_t , pkthandler_t> pkthandler_map_t;

    typedef boost::function<void ( base::cluster::MessageIn& ) > msghandler_t;
    typedef boost::unordered_map<uint16_t,msghandler_t> msghandler_map_t;

    //玩家状态
    enum PlayerFlag {
        Authed = 0, //是否认证状态
        DataReady,  //数据已就绪
        InRoom,     //是否在房间中
        InGame,     //是否游戏中
        Terminate,  //被终止（执行了PlayerSession::Exit()）

        SIZE,       //确定枚举大小，一定要在最后
    };

    typedef std::bitset< ( std::size_t ) SIZE> player_flag_t;

    class PlayerSession : public base::gateway::UserSession , public base::cluster::Mailbox::EventHandler
    {
        INTRUSIVE_LIST ( PlayerSession )
    public:
        PlayerSession ( base::gateway::UserClient* uc );
        virtual ~PlayerSession();

        uint32_t uid() const {
            return uid_;
        }

        player_flag_t flags() {
            return flags_;
        }

        bool ready() const {
            return ready_;
        }

        base::cluster::Mailbox& mb() {
            return *mb_;
        }
        //游戏角色
        comp::Character& character() {
            return *character_;
        }

        //网络检查
        comp::Ping& ping() {
            return *ping_;
        }

        //玩家字典
        comp::Dict& dict() {
            return *dict_;
        }

        //玩家背包
        comp::Bag& bag() {
            return *bag_;
        }

        //聊天组件
        comp::Chat& chat() {
            return *chat_;
        }

        //地图组件
        comp::Map& map() {
            return *map_;
        }
        comp::Map* map_ptr() {
            return map_;
        }

        //技能组件
        comp::Spell& spell() {
            return *spell_;
        }

        //战斗属性组件
        comp::Property& property() {
            return *property_;
        }

        //游戏组件
        comp::Game& game() {
            return *game_;
        }

        //冷却队列
        comp::CDList& cdlist() {
            return *cdlist_;
        }

        //BUFF
        comp::Buff& buff() {
            return *buff_;
        }

        //响应
        Response& response() {
            return *response_;
        }

        //邮箱投递
        Delivery& delivery() {
            return *delivery_;
        }

        comp::Mail& mail() {
            return * mail_;
        }

        //好友
        comp::Friend& friends() {
            return * friend_;
        }
        comp::Friend* friend_ptr() {
            return friend_;
        }
        comp::QqOpenApi& qqapi() {
            return * qqapi_;
        }

        int64_t last_speak_time;

        //定时保存事件
        base::Event<void() > evt_save;

        //每五分钟执行一次更新
        base::Event<void() > evt_update;

        void PushCommand ( base::command::CommandT<PlayerSession>* cmd ) {
            executor_.PushCommand ( cmd );
        }
        //初始化
        bool Setup();

#define PACKET_MAP(code , fun) ps().RegisterPacketHandler((uint16_t)code, boost::bind(&fun,this,_1))
        void RegisterPacketHandler ( uint16_t code , pkthandler_t hd );
#define MESSAGE_MAP(code , fun) ps().RegisterMessageHandler((uint16_t)code , boost::bind(&fun,this,_1))
        void RegisterMessageHandler ( uint16_t code , msghandler_t hd );
        //强制退出
        void Exit();
        //当组件启动完成
        void OnCompSetup ( comp::ComponentBase* c );
        //当组件关闭完成
        void OnCompCleanup ( comp::ComponentBase* c );
    private:
        virtual void OnUserClientReceivePacket ( base::gateway::PacketIn& pktin );
        virtual void OnUserClientClose();
        virtual void OnMessageReceive ( base::cluster::MessageIn& msgin );

    private:
        bool IsAllCompSetup() const;
        void StartTimer();
        void EvtUpdateTrigger();
        void StopTimer();

    private:
        base::command::RunnerT<PlayerSession> executor_;
        player_flag_t flags_;
        pkthandler_map_t pkthandlers_;
        msghandler_map_t msghandlers_;
        std::size_t cursor_;
        std::vector<comp::ComponentBase*> components_; //所有组件

        base::cluster::Mailbox* mb_;

        Response* response_;
        Delivery* delivery_;
        comp::Character* character_;
        comp::Ping* ping_;
        comp::Dict* dict_;
        comp::Bag* bag_;
        comp::Chat* chat_;
        comp::Map* map_;
        comp::Mail* mail_;
        comp::Game* game_;
        comp::Spell* spell_;
        comp::Property* property_;
        comp::CDList* cdlist_;
        comp::Buff* buff_;
        comp::Friend* friend_;
        comp::PolyCopper* poly_;
        comp::Task* task_;
        comp::QqOpenApi * qqapi_;
        uint32_t uid_;
        bool ready_; //TODO 可以考虑整合重构ready_,flags_等玩家标志

        base::TimeoutLinker* timeout_linker_;
    };
}
#endif // PLAYERSESSION_H
