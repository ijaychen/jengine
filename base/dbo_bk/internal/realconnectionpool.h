#ifndef BASE_DBO_INTERNAL_REALCONNECTIONPOOL_H
#define BASE_DBO_INTERNAL_REALCONNECTIONPOOL_H

#include "../../global.h"
#include "realconnection.h"
#include "configure.h"
#include "../../exception.h"
#include "../../timer.h"
#include <vector>
#include <list>
#include <queue>
#include <boost/function.hpp>

namespace base
{
    namespace dbo
    {
        class Connection;

        namespace internal
        {
            // 真实连接池
            // 申请和释放连接, 在有空闲连接时抛出事件，在发生错误时抛出事件
            class RealConnectionPool : public RealConnection::EventHandler
            {
            public:
                RealConnectionPool(memory::MemoryPool& mempool, const ConnectionPoolInfo& poolinfo);
                virtual ~RealConnectionPool();

                const ConnectionPoolInfo& poolinfo() const {
                    return poolinfo_;
                }
                uint32_t poolid() const {
                    return poolinfo_.poolid;
                }
                std::size_t free_size() const {
                    return free_.size();
                }
                std::size_t busy_size() const {
                    return busy_.size();
                }
                std::size_t pending_size() const {
                    return pending_.size();
                }
                std::size_t pool_size() const {
                    return poolinfo_.poolsize;
                }

                bool empty() const {
                    return free_.empty() && busy_.empty() && pending_.empty();
                }

                // 启动
                bool Setup();
                // 停止
                void Cleanup();
                // 申请
                void AquireRealConnection(Connection* conn);
                // 释放
                void ReleaseRealConnection(RealConnection* conn);

            private:
                virtual void OnConnect(RealConnection* sender);
                virtual void OnConnectFail(RealConnection* sender, const char* reason);
                virtual void OnClose(RealConnection* sender);
                virtual void OnAuthSuccess(RealConnection* sender);
                virtual void OnAuthFail(RealConnection* sender, int errcode, const char* reason);

            private:
                // 检查申请列表
                void CheckAquireList();
                // 检查是否创建新的连接
                void CheckCreateMore();
                // 创建一个新连接
                void CreateNewConnection();

            private:
                std::list<RealConnection*> free_;               // 空闲的
                std::list<RealConnection*> busy_;               // 正在使用中
                std::list<RealConnection*> pending_;            // 未决的,正在初始化
                std::queue<Connection*> aquire_list_;           // 申请列表

                ConnectionPoolInfo poolinfo_;
                memory::MemoryPool& mempool_;
            };

            // 真实连接池管理器
            // 管理申请队列，有空闲边接时进行分配, 可创建多个边接池，应对多个数据库服务器
            class RealConnectionPoolManager
            {
            public:
                static RealConnectionPoolManager& instance() {
                    static RealConnectionPoolManager ins;
                    return ins;
                }
                virtual ~RealConnectionPoolManager();

                // 用于mysql协议包的内存池
                memory::MemoryPool& mempool() {
                    return *mempool_;
                }

                const Configure& config() const {
                    return config_;
                }

                // 启动, 回调成功或失败结果
                void BeginSetup(const boost::function<void(bool)>& cb);
                // 停止
                void BeginCleanup(const boost::function<void()>& cb);

                // 给指定的Connection分配一个连接, 成功时会调用Connection::OnAquireSuccess
                void AquireRealConnection(uint32_t poolid, Connection* conn) {
                    if (poolid < pools_.size()) {
                        pools_[poolid]->AquireRealConnection(conn);
                    } else {
                        throw Exception("not exist mysql connection pool id!");
                    }
                }
                // 释放
                void ReleaseRealConnection(RealConnection* conn) {
                    pools_[conn->poolid()]->ReleaseRealConnection(conn);
                }

                // 创建一个连接池
                void CreateRealConnectionPool(const ConnectionPoolInfo& poolinfo);

            private:
                RealConnectionPoolManager();

                void CheckIfSetupFinish();
                void CheckIfCleanupFinish();
                
                void DebugDump();

                memory::MemoryPool* mempool_;
                // 配置
                Configure config_;
                // 连接池列表
                std::vector<RealConnectionPool*> pools_;

                boost::function<void(bool)> cb_setup_;
                base::TimeoutLinker* setup_checker_;
                bool roolback_;
                boost::function<void()> cb_cleanup_;
                base::TimeoutLinker* cleanup_checker_;
                
                base::TimeoutLinker* probe_checker_;
            };
        }
    }
}

#endif // REALCONNECTIONPOOL_H
