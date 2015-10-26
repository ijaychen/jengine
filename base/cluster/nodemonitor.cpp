#include "nodemonitor.h"
#include "../logger.h"
#include "../framework.h"
#include "../memory/memorypool.h"
#include "../action/executor.h"
#include "../action/actionnextcall.h"
#include <list>
#include <boost/bind.hpp>

namespace base
{
    namespace cluster
    {
        using namespace std;

        enum NodeInternalCode {
            INTERNAL_AUTH = 1,                                  // 请求认证
            INTERNAL_REGISTER_NAMED_MAILBOX = 3,                // 注册命名邮箱
            INTERNAL_UNREGISTER_NAMED_MAILBOX = 4,              // 取消注册命名邮箱
        };

        /// Connector
        Connector::Connector(Connector::EventHandler& handler, memory::MemoryPool& mempool)
            : net::Client(mempool), info_(nullptr), required_(false), handler_(handler)
        {
        }

        Connector::~Connector()
        {
        }

        void Connector::SendMessage(MessageOut& msgout)
        {
            msgout.WriteHead();
            message_data_t data = msgout.FetchData();
            PushSend(data);
        }

        void Connector::HandleInternalMessage(MessageIn& msgin)
        {
            NodeInternalCode code = static_cast<NodeInternalCode>(msgin.code());
            switch (code) {
                case INTERNAL_AUTH: {
                    uint8_t nodeid = msgin.ReadByte();
                    uint16_t len = msgin.ReadUShort();
                    bool cookie_passed = false;
                    string cookie;
                    for (uint16_t i = 0; i < len; ++i) {
                        msgin.ReadString(cookie);
                        if (NodeMonitor::instance().config().local_node().IsCookieAllow(cookie)) {
                            cookie_passed = true;
                            break;
                        }
                    }
                    if (cookie_passed) {
                        info_ = NodeMonitor::instance().config().FindNodeInfoByID(nodeid);
                        if (info_) {
                            handler_.OnConnectorAuthSuccess(this);
                        } else {
                            LOG_ERROR("cluster auth fail, can not found node info with nodeid=%u\n", nodeid);
                            Close();
                        }
                    } else {
                        LOG_WARN("cluster auth fail, bad cookie from node %s:%d\n", ipaddr().c_str(), port());
                        Close();
                    }
                }
                break;
                default:
                    LOG_WARN("unexpected message handled by connector!\n");
                    break;
            }
        }

        void Connector::OnReceive(std::size_t count)
        {
            while (count >= MessageIn::HEAD_SIZE) {
                uint32_t pktlen = 0;
                CopyReceive((char*)&pktlen, 4);

                if (pktlen < MessageIn::HEAD_SIZE || pktlen > 1024 * 1024 * 5) {
                    LOG_WARN("the message size it too small or big, size= %u\n", pktlen);
                }

                if (pktlen >= MessageIn::HEAD_SIZE && count >= pktlen) {
                    message_data_t data;
                    PopReceive(data, pktlen);
                    MessageIn msgin(data);
                    msgin.ReadHead();
                    try {
                        if (!msgin.to() && !authed()) {
                            HandleInternalMessage(msgin);
                        } else {
                            handler_.OnConnectorReceiveMessage(this, msgin);
                        }
                    } catch (exception& ex) {
                        LOG_ERROR("occurred exception when handle node message: %s\n", ex.what());
                        Close();
                    }
                    count -= pktlen;
                } else {
                    break;
                }
            }
        }

        void Connector::OnConnect()
        {
            // TODO 进行认证
            MessageOut msgout((uint16_t)INTERNAL_AUTH, 20, mempool());
            msgout.WriteByte(NodeMonitor::instance().config().local_node().info->id);
            const vector<string>& cookies = NodeMonitor::instance().config().local_node().cookies;
            msgout.WriteUShort(cookies.size());
            for (uint32_t i = 0; i < cookies.size(); ++i) {
                msgout.WriteString(cookies[i]);
            }
            SendMessage(msgout);
        }

        void Connector::OnConnectFail(int eno, const char* reason)
        {
            handler_.OnConnectorAuthFail(this, reason);
        }

        void Connector::OnClose()
        {
            handler_.OnConnectorClose(this);
        }

        /// NodeMonitor
        NodeMonitor::NodeMonitor()
            : mempool_(nullptr), listener_(nullptr),
              state_(NODE_MONITOR_STATE_IN_SETUP),
              exe_(nullptr)
        {
        }

        NodeMonitor::~NodeMonitor()
        {
            SAFE_RELEASE(listener_);
            SAFE_RELEASE(exe_);
            SAFE_DELETE(mempool_);
            for (list<AsyncFetchNamedMailbox*>::iterator it = async_fetch_.begin(); it != async_fetch_.end(); ++it) {
                delete *it;
            }
            async_fetch_.clear();
        }

        void NodeMonitor::OnListenerAccept(net::Listener* sender, int clientfd)
        {
            Connector* conn = new Connector(*this, *mempool_);
            conn->Connect(clientfd);
            anonymous_connectors_.push_back(conn);
        }

        void NodeMonitor::OnConnectorAuthSuccess(Connector* conn)
        {
            std::pair<connector_map_t::iterator, bool> r = connectors_.emplace(conn->info()->id, conn);
            if (!r.second) {
                LOG_ERROR("duplicate node id!!!");
                framework.Stop();
                return;
            }
            anonymous_connectors_.remove(conn);
            CheckIfAllSetup();
            OnNodeUp(conn, conn->info());
        }

        void NodeMonitor::OnConnectorAuthFail(Connector* conn, const char* reason)
        {
            LOG_ERROR("cannot connect to link node[%s:%d]: %s\n", conn->ipaddr().c_str(),
                      conn->port(), reason);
            anonymous_connectors_.remove(conn);
            conn->Release();
            if (conn->required() && state_ == NODE_MONITOR_STATE_IN_SETUP) {
                state_ = NODE_MONITOR_STATE_IN_ROLLBACK;
                CloseAll();
            }
            CheckIfAllRollback();
        }

        void NodeMonitor::OnConnectorClose(Connector* conn)
        {
            if (conn->authed()) {
                connectors_.erase(conn->info()->id);
                OnNodeDown(conn->info());
            } else {
                anonymous_connectors_.remove(conn);
            }
            conn->Release();

            if (state_ == NODE_MONITOR_STATE_IN_SETUP) {
                state_ = NODE_MONITOR_STATE_IN_ROLLBACK;
                CloseAll();
            }
            CheckIfAllCleanup();
            CheckIfAllRollback();
        }

        void NodeMonitor::OnConnectorReceiveMessage(Connector* conn, MessageIn& msgin)
        {
            if (!msgin.to()) {
                NodeInternalCode code = static_cast<NodeInternalCode>(msgin.code());
                switch (code) {
                    case INTERNAL_REGISTER_NAMED_MAILBOX: {
                        string name;
                        MailboxID mbid;
                        msgin >> name >> mbid;
                        name.append("@");
                        name.append(conn->info()->name);
                        OnNamedMailboxUp(name, mbid);
                    }
                    break;
                    case INTERNAL_UNREGISTER_NAMED_MAILBOX: {
                        string name;
                        msgin >> name;
                        name.append("@");
                        name.append(conn->info()->name);
                        if (named_mailboxes_.erase(name) == 1) {
                            OnNamedMailboxDown(name);
                        }
                    }
                    break;
                    default:
                        LOG_ERROR("can not handle message without dest mailboxid!\n");
                        break;
                }
            } else {
                Mailbox* mb = FindMailboxByPID(msgin.to().pid());
                if (mb) {
                    try {
                        mb->HandleMessageReceive(msgin);
                    } catch (exception& ex) {
                        LOG_ERROR("catch exception when handle mailbox message, code=%u\n", msgin.code());
                    }
                }
            }
        }

        void NodeMonitor::BeginSetup(boost::function<void(bool)> cb)
        {
            exe_ = new action::Executor;

            state_ = NODE_MONITOR_STATE_IN_SETUP;
            // 读取配置
            if (!config_.ParseXml(framework.resource_dir(), framework.priv_dir())) {
                cb(false);
                return;
            }

            mempool_ = new memory::MemoryPool(256, 256);

            // 启动侦听
            if (config_.local_node().info->HasListener()) {
                listener_ = new net::Listener(*this);
                bool result = listener_->Bind(config_.local_node().info->listener.ip.c_str(), config_.local_node().info->listener.port);
                if (!result) {
                    cb(false);
                    return;
                }
            }

            // 连接其它节点
            vector<LinkNodeInfo> links = config_.FetchLinkNodeList();
            // 检查一下
            for (vector<LinkNodeInfo>::const_iterator it = links.begin(); it != links.end(); ++it) {
                if (!(*it).node().HasListener()) {
                    LOG_ERROR("bad link node info without listener section!\n");
                    cb(false);
                    break;
                }
            }

            cb_setup_ = cb;
            // 开始建立连接
            for (vector<LinkNodeInfo>::const_iterator it = links.begin(); it != links.end(); ++it) {
                const LinkNodeInfo& link = *it;
                Connector* conn = new Connector(*this, *mempool_);
                anonymous_connectors_.push_back(conn);
                if (link.require()) {
                    conn->SetRequired();
                }
                conn->Connect(link.node().listener.ip.c_str(), link.node().listener.port);
            }
            CheckIfAllSetup();
        }

        void NodeMonitor::BeginCleanup(boost::function<void()> cb)
        {
            // FIXME wait all action stopped then make cleanup finish
            // because then send at next call contain reference of memorychunk
            state_ = NODE_MONITOR_STATE_IN_CLEANUP;
            cb_cleanup_ = cb;
            CloseAll();
            exe_->BeginStopAllAction(boost::bind(&NodeMonitor::CheckIfAllCleanup, this));
            CheckIfAllCleanup();
        }

        void NodeMonitor::CloseAll()
        {
            if (listener_) {
                listener_->Close();
            }
            for (list<Connector*>::iterator it = anonymous_connectors_.begin();
                    it != anonymous_connectors_.end();) {
                (*it)->Close();
                if (!(*it)->connect()) {
                    (*it)->Release();
                    it = anonymous_connectors_.erase(it);
                } else {
                    ++it;
                }
            }
            for (connector_map_t::iterator it = connectors_.begin();
                    it != connectors_.end();) {
                it->second->Close();
                if (!it->second->connect()) {
                    it->second->Release();
                    it = connectors_.erase(it);
                } else {
                    ++it;
                }
            }
        }

        void NodeMonitor::CheckIfAllSetup()
        {
            if (state_ == NODE_MONITOR_STATE_IN_SETUP) {
                if (anonymous_connectors_.empty()) {
                    cb_setup_(true);
                    state_ = NODE_MONITOR_STATE_OK;
                }
            }
        }

        void NodeMonitor::CheckIfAllRollback()
        {
            if (state_ == NODE_MONITOR_STATE_IN_ROLLBACK) {
                if (anonymous_connectors_.empty() && connectors_.empty()) {
                    cb_setup_(false);
                    state_ = NODE_MONITOR_STATE_STOP;
                }
            }
        }

        void NodeMonitor::CheckIfAllCleanup()
        {
            if (state_ == NODE_MONITOR_STATE_IN_CLEANUP) {
                if (anonymous_connectors_.empty() && connectors_.empty() && exe_->actions().empty()) {
                    cb_cleanup_();
                    state_ = NODE_MONITOR_STATE_STOP;
                    if (!mailboxes_.empty()) {
                        LOG_WARN("there are still exist mailboxs registed at monitor, size=%u\n", mailboxes_.size());
                    }
                }
            }
        }

        MailboxID NodeMonitor::GenerateMailboxID(bool sys)
        {
            MailboxID mbid(local_node_info()->id, pid_gen_.Gen(sys));
            return mbid;
        }

        bool NodeMonitor::Register(Mailbox* mb, bool sys)
        {
            if (!mb->name().empty()) {
                if (named_mailboxes_.find(mb->name()) != named_mailboxes_.end()) {
                    LOG_WARN2("duplicate named mailbox %s\n", mb->name().c_str());
                    return false;
                }
            }
            mb->mbid_ = GenerateMailboxID(sys);
            mailboxes_.insert(make_pair(mb->mbid_.pid(), mb));
            if (!mb->name().empty()) {
                OnNamedMailboxUp(mb->name(), mb->mbid());
                BroadcastRegisterNamedMailbox(mb->name(), mb->mbid());
            }
            return true;
        }

        void NodeMonitor::UnRegister(Mailbox* mb)
        {
            if (!mb->name().empty()) {
                named_mailboxes_.erase(mb->name());
                OnNamedMailboxDown(mb->name());
                BroadcastUnRegisterNamedMailbox(mb->name());
            }
            mailboxes_.erase(mb->mbid_.pid());
            mb->mbid_.Clear();
        }

        void NodeMonitor::BroadcastRegisterNamedMailbox(const string& name, const MailboxID& mbid)
        {
            for (connector_map_t::iterator it = connectors_.begin(); it != connectors_.end(); ++it) {
                SendRegisterNamedMailbox(it->second, name, mbid);
            }
        }

        void NodeMonitor::SendRegisterNamedMailbox(Connector* conn, const string& name, const MailboxID& mbid)
        {
            MessageOut msgout((uint16_t)INTERNAL_REGISTER_NAMED_MAILBOX, 30, *mempool_);
            msgout << name << mbid;
            conn->SendMessage(msgout);
        }

        void NodeMonitor::BroadcastUnRegisterNamedMailbox(const string& name)
        {
            for (connector_map_t::iterator it = connectors_.begin(); it != connectors_.end(); ++it) {
                MessageOut msgout((uint16_t)INTERNAL_UNREGISTER_NAMED_MAILBOX, 30, *mempool_);
                msgout << name;
                it->second->SendMessage(msgout);
            }
        }

        void NodeMonitor::SendInternalMessageAtNextCall(MailboxID to, message_data_t data)
        {
            Mailbox* mb = FindMailboxByPID(to.pid());
            if (mb) {
                MessageIn msgin(data);
                msgin.ReadHead();
                mb->HandleMessageReceive(msgin);
            }
        }

        void NodeMonitor::SendMessage(MessageOut& msgout)
        {
            const MailboxID& to = msgout.to();

            if (IsLocalMailbox(to)) {
                msgout.WriteHead();
                message_data_t data = msgout.FetchData();
                action::ActionNextCall* act = new action::ActionNextCall(boost::bind(&NodeMonitor::SendInternalMessageAtNextCall, this, to, data));
                exe_->RunAction(act);
                act->Release();
            } else {
                Connector* conn = FindConnectorByNodeID(to.nodeid());
                if (conn) {
                    conn->SendMessage(msgout);
                }
            }
        }

        void NodeMonitor::OnNamedMailboxUp(const std::string& name, const MailboxID& mbid)
        {
            cout << "OnNamedMailboxUp:" << name << "," << mbid << endl;
            named_mailboxes_.insert(make_pair(name, mbid));
            CheckAsyncFetchList(name, mbid);
            evt_named_mailbox_up.Trigger(name.c_str(), mbid);
        }

        void NodeMonitor::OnNamedMailboxDown(const std::string& name)
        {
            cout << "OnNamedMailboxDown:" << name << endl;
            evt_named_mailbox_down.Trigger(name.c_str());
        }

        void NodeMonitor::OnNodeUp(Connector* conn, const NodeInfo* info)
        {
            cout << "[node]:" << (uint32_t)info->id << "," << info->name << "  up.." << endl;
            // 将所有本地命名邮箱同步给新来的节点
            for (named_mailbox_map_t::iterator it = named_mailboxes_.begin(); it != named_mailboxes_.end(); ++it) {
                if (IsLocalMailbox(it->second)) {
                    SendRegisterNamedMailbox(conn, it->first, it->second);
                }
            }
            evt_node_up.Trigger(*info);
        }

        void NodeMonitor::OnNodeDown(const NodeInfo* info)
        {
            cout << "[node]:" << (uint32_t)info->id << "," << info->name << "  down.." << endl;
            // 移除该节点对应的命名邮箱
            for (named_mailbox_map_t::iterator it = named_mailboxes_.begin(); it != named_mailboxes_.end();) {
                if (it->second.nodeid() == info->id) {
                    OnNamedMailboxDown(it->first);
                    it = named_mailboxes_.erase(it);
                } else {
                    ++it;
                }
            }
            evt_node_down.Trigger(*info);
        }

        void NodeMonitor::FetchNamedMailbox(const string& matcher, async_fetch_named_mailbox_callback_t cb)
        {
            // check all
            for (named_mailbox_map_t::iterator it = named_mailboxes_.begin(); it != named_mailboxes_.end(); ++it) {
                if (is_process_name_match(matcher, it->first)) {
                    cb(it->second);
                    return;
                }
            }
            async_fetch_.push_back(new AsyncFetchNamedMailbox(matcher, cb));
        }

        AsyncFetchLinker* NodeMonitor::FetchNamedMailboxWithLinker(const string& matcher, async_fetch_named_mailbox_callback_t cb)
        {
            for (named_mailbox_map_t::iterator it = named_mailboxes_.begin(); it != named_mailboxes_.end(); ++it) {
                if (is_process_name_match(matcher, it->first)) {
                    cb(it->second);
                    return nullptr;
                }
            }
            AsyncFetchLinker* linker = new AsyncFetchLinker;
            async_fetch_.push_back(new AsyncFetchNamedMailbox(matcher, cb, linker));
            return linker;
        }

        void NodeMonitor::CheckAsyncFetchList(const string& name, const MailboxID& mbid)
        {
            for (list<AsyncFetchNamedMailbox*>::iterator it = async_fetch_.begin(); it != async_fetch_.end();) {
                if ((*it)->linker != nullptr && (*it)->linker->reference_count() != 2) {
                    delete *it;
                    it = async_fetch_.erase(it);
                    continue;
                }
                if (is_process_name_match((*it)->matcher, name)) {
                    (*it)->callback(mbid);
                    delete *it;
                    it = async_fetch_.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }
}
