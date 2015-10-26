#ifndef BASE_CLUSTER_NODEMONITOR_H
#define BASE_CLUSTER_NODEMONITOR_H

#include "../global.h"
#include "../net/listener.h"
#include "../net/client.h"
#include "message.h"
#include "mailbox.h"
#include "configure.h"
#include "../event.h"
#include <string>
#include <map>
#include <list>
#include <boost/function.hpp>
#include <boost/unordered_map.hpp>

namespace base
{
    namespace action
    {

        class Executor;
    }

    namespace cluster
    {
        using namespace std;

        // 节点信息
        class NodeMonitor;

        // 连接 (TODO 自动重连功能?)
        class Connector : public net::Client
        {
        public:
            struct EventHandler {
                virtual ~EventHandler() {}
                virtual void OnConnectorAuthSuccess(Connector* conn) = 0;
                virtual void OnConnectorAuthFail(Connector* conn, const char* reason) = 0;
                virtual void OnConnectorClose(Connector* conn) = 0;
                virtual void OnConnectorReceiveMessage(Connector* conn, MessageIn& msgin) = 0;
            };

        public:
            DISABLE_COPY(Connector)
            Connector(EventHandler& handler, memory::MemoryPool& mempool);
            virtual ~Connector();

            const NodeInfo* info() const {
                return info_;
            }
            bool authed() const {
                return info_ != nullptr;
            }
            bool required() const {
                return required_;
            }

            void SendMessage(MessageOut& msgout);
            void SetRequired() {
                required_ = true;
            }

        private:
            virtual void OnConnect();
            virtual void OnConnectFail(int eno, const char* reason);
            virtual void OnReceive(size_t count);
            virtual void OnClose();

        private:
            void HandleInternalMessage(MessageIn& msgin);
            const NodeInfo* info_;
            bool required_;
            EventHandler& handler_;
        };

        // check if process name match
        // style 1: calculator          => [calculator] | []
        // style 2: calculator@*        => [calculator@a, calculator@b, calculator] | []
        // style 3: calculator@a        => [calculator@a] | []
        // style 4: calculator.*@a      => [calculator.1@a, calculator.2@b] | []
        bool inline is_process_name_match(const std::string& matcher, const std::string& process_name)
        {
            size_t m = 0, p = 0;
            bool star = false;
            while (m < matcher.size() && p < process_name.size()) {
                if (star) {
                    if (process_name[p] == '@') {
                        star = false;
                    } else {
                        ++p;
                    }
                } else {
                    if (matcher[m] == '*') {
                        star = true;
                        ++m;
                    } else {
                        if (matcher[m] == process_name[p]) {
                            ++m;
                            ++p;
                        } else {
                            return false;
                        }
                    }
                }
            }

            if (m < matcher.size()) {
                std::size_t rest = matcher.size() - m;
                if (rest == 2) {
                    if (matcher[m] == '@' && matcher[m + 1] == '*') {
                        return true;
                    }
                }
                return false;
            } else {
                if (p < process_name.size()) {
                    return matcher[matcher.length() - 1] == '*';
                } else {
                    return true;
                }
            }
        }

        typedef base::Event<void(const std::string& name, const MailboxID& pid)> NamedMailboxUpEvent;
        typedef base::Event<void(const std::string& name, const MailboxID& pid)> NamedMailboxDownEvent;

        typedef base::Event<void(uint16_t nodeid, const std::string& name)> NodeUpEvent;
        typedef base::Event<void(uint16_t nodeid, const std::string& name)> NodeDownEvent;

        typedef boost::unordered_map<uint8_t, Connector*> connector_map_t;
        typedef boost::function<void(const MailboxID&)> async_fetch_named_mailbox_callback_t;
        typedef boost::unordered_map<std::string, MailboxID> named_mailbox_map_t;

        class AsyncFetchLinker : public Object
        {
        };

        class NodeMonitor : public net::Listener::EventHandler, public Connector::EventHandler
        {
        public:
            static NodeMonitor& instance() {
                static NodeMonitor ins;
                return ins;
            }
            virtual ~NodeMonitor();

            Event<void(const char*, const MailboxID&)> evt_named_mailbox_up;
            Event<void(const char*)> evt_named_mailbox_down;
            Event<void(const NodeInfo&)> evt_node_up;
            Event<void(const NodeInfo&)> evt_node_down;

            // 配置信息
            const Configure& config() const {
                return config_;
            }
            memory::MemoryPool& mempool() {
                return *mempool_;
            }
            // 本地节点信息
            const NodeInfo* local_node_info() const {
                return config_.local_node().info;
            }

            // 判断mbid是否为本地邮箱
            bool inline IsLocalMailbox(const MailboxID& mbid) {
                return config_.local_node().info->id == mbid.nodeid();
            }

            // 开始启动
            void BeginSetup(boost::function<void(bool)> cb);
            // 开始停止
            void BeginCleanup(boost::function<void()> cb);

            // 注册邮箱, 返回是否成功
            bool Register(Mailbox* mb, bool sys);
            // 取消注册邮箱
            void UnRegister(Mailbox* mb);
            // 发送消息
            void SendMessage(MessageOut& msgout);

            // 获取命名邮箱
            // matcher是匹配模式, 有如下四种方式
            // style 1: calculator          => [calculator] | []
            // style 2: calculator@*        => [calculator@a, calculator@b, calculator] | []
            // style 3: calculator@a        => [calculator@a] | []
            // style 4: calculator.*@a      => [calculator.1@a, calculator.2@b] | []
            void FetchNamedMailbox(const std::string& matcher, async_fetch_named_mailbox_callback_t cb);
            AsyncFetchLinker* FetchNamedMailboxWithLinker(const std::string& matcher, async_fetch_named_mailbox_callback_t cb);

        private:
            NodeMonitor();
            virtual void OnListenerAccept(net::Listener* sender, int clientfd);
            virtual void OnConnectorAuthSuccess(Connector* conn);
            virtual void OnConnectorAuthFail(Connector* conn, const char* reason);
            virtual void OnConnectorClose(Connector* conn);
            virtual void OnConnectorReceiveMessage(Connector* conn, MessageIn& msgin);

        private:
            void CloseAll();
            void CheckIfAllSetup();
            void CheckIfAllCleanup();
            void CheckIfAllRollback();

            void SendInternalMessageAtNextCall(MailboxID to, message_data_t);

        private:
            void OnNodeUp(Connector* conn, const NodeInfo* info);
            void OnNodeDown(const NodeInfo* info);

            void BroadcastRegisterNamedMailbox(const std::string& name, const MailboxID& mbid);
            void SendRegisterNamedMailbox(Connector* conn, const std::string& name, const MailboxID& mbid);
            void BroadcastUnRegisterNamedMailbox(const std::string& name);

            void OnNamedMailboxUp(const std::string& name, const MailboxID& mbid);
            void OnNamedMailboxDown(const std::string& name);

        private:
            // 获取pid获取mailbox
            Mailbox* FindMailboxByPID(int32_t pid) {
                std::map<int32_t, Mailbox*>::iterator it = mailboxes_.find(pid);
                return it == mailboxes_.end() ? nullptr : it->second;
            }

            Connector* FindConnectorByNodeID(uint8_t nodeid) {
                connector_map_t::iterator it = connectors_.find(nodeid);
                return it == connectors_.end() ? nullptr : it->second;
            };

        private:
            boost::function<void(bool)> cb_setup_;
            boost::function<void()> cb_cleanup_;
            memory::MemoryPool* mempool_;
            net::Listener* listener_;                           // 侦听器
            std::list<Connector*> anonymous_connectors_;        // 匿名连接
            connector_map_t connectors_;                        // 已认证的连接
            std::map<int32_t, Mailbox*> mailboxes_;             // 所有已注册的邮箱
            named_mailbox_map_t named_mailboxes_;               // 命名邮箱
            enum NodeMonitorState {
                NODE_MONITOR_STATE_IN_SETUP,
                NODE_MONITOR_STATE_IN_ROLLBACK,
                NODE_MONITOR_STATE_OK,
                NODE_MONITOR_STATE_IN_CLEANUP,
                NODE_MONITOR_STATE_STOP,
            };
            NodeMonitorState state_;
            Configure config_;
            action::Executor* exe_;

            MailboxID GenerateMailboxID(bool sys);
            // 本地邮箱ID生成器
            class ProcessIDGen
            {
            public:
                ProcessIDGen() : sys_cur_(1), normal_cur_(10000) {}

                // 生成进程ID
                // sys 系统进程, 1 ~ 10000, 不可循环使用
                int32_t Gen(bool sys) {
                    if (sys) {
                        assert (sys_cur_ < 10000);
                        return sys_cur_++;
                    } else {
                        normal_cur_++;
                        if (normal_cur_ >= MAX_NORMAL_ID) {
                            normal_cur_ = 1000;
                        }
                        return normal_cur_++;
                    }
                }

            private:
                static const int32_t MAX_NORMAL_ID = 256 * 256 * 256;
                int32_t sys_cur_;
                int32_t normal_cur_;
            };
            ProcessIDGen pid_gen_;

        private:
            struct AsyncFetchNamedMailbox {
                DISABLE_COPY(AsyncFetchNamedMailbox);
                std::string matcher;
                async_fetch_named_mailbox_callback_t callback;
                AsyncFetchLinker* linker;
                AsyncFetchNamedMailbox(const std::string& m, const async_fetch_named_mailbox_callback_t& c, AsyncFetchLinker* l = nullptr)
                    : matcher(m), callback(c), linker(l) {}
                ~AsyncFetchNamedMailbox() {
                    SAFE_RELEASE(linker);
                }
            };
            std::list<AsyncFetchNamedMailbox*> async_fetch_;
            // 检查是否有适合的
            void CheckAsyncFetchList(const std::string& name, const MailboxID& mbid);
        };
    }
}

#endif // BASE_CLUSTER_NODEMONITOR_H
