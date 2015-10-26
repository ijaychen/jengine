#include "realconnectionpool.h"
#include "configure.h"
#include "../connection.h"
#include "../../exception.h"
#include "../../memory/memorypool.h"
#include "../../logger.h"
#include "../../framework.h"
#include "../../event/dispatcher.h"
#include <boost/bind.hpp>
#include <sys/socket.h>

namespace base
{
    namespace dbo
    {
        namespace internal
        {
            using namespace std;

            /// RealConnectionPool
            RealConnectionPool::RealConnectionPool(memory::MemoryPool& mempool, const ConnectionPoolInfo& poolinfo)
                : poolinfo_(poolinfo), mempool_(mempool)
            {
            }

            RealConnectionPool::~RealConnectionPool()
            {
            }

            bool RealConnectionPool::Setup()
            {
                for (uint32_t i = 0; i < pool_size(); ++i) {
                    // do connection
                    CreateNewConnection();
                }
                return true;
            }

            void RealConnectionPool::CreateNewConnection()
            {
                RealConnection* conn = new RealConnection(*this, mempool_, poolid());
                pending_.push_back(conn);
                conn->Connect(poolinfo_.serverinfo);
            }

            void RealConnectionPool::Cleanup()
            {
                // 清空申请列表
                while (!aquire_list_.empty()) {
                    Connection* conn = aquire_list_.front();
                    conn->Release();
                    aquire_list_.pop();;
                }

                for (list<RealConnection*>::iterator it = pending_.begin(); it != pending_.end();) {
                    (*it)->Close();
                    if (!(*it)->connect()) {
                        (*it)->Release();
                        it = pending_.erase(it);
                    } else {
                        ++it;
                    }
                }
                for (list<RealConnection*>::iterator it = free_.begin(); it != free_.end();) {
                    (*it)->Close();
                    if (!(*it)->connect()) {
                        (*it)->Release();
                        it = free_.erase(it);
                    } else {
                        ++it;
                    }
                }
            }

            void RealConnectionPool::OnConnect(RealConnection* sender)
            {
            }

            void RealConnectionPool::OnConnectFail(RealConnection* sender, const char* reason)
            {
                pending_.remove(sender);
                sender->Release();
            }

            void RealConnectionPool::OnClose(RealConnection* sender)
            {
                pending_.remove(sender);
                free_.remove(sender);
                busy_.remove(sender);
                sender->Release();
            }

            void RealConnectionPool::OnAuthSuccess(RealConnection* sender)
            {
                pending_.remove(sender);
                free_.push_back(sender);
                CheckAquireList();
            }

            void RealConnectionPool::OnAuthFail(RealConnection* sender, int errcode, const char* reason)
            {
                LOG_ERROR("mysql authorize fail: %d, %s\n", errcode, reason);
            }

            void RealConnectionPool::AquireRealConnection(Connection* conn)
            {
                aquire_list_.push(conn);
                conn->Retain();
                CheckAquireList();
                CheckCreateMore();
            }

            void RealConnectionPool::CheckAquireList()
            {
                while (!free_.empty() && !aquire_list_.empty()) {
                    RealConnection* realconn = free_.front();
                    free_.pop_front();

                    Connection* conn = aquire_list_.front();
                    aquire_list_.pop();

                    busy_.push_back(realconn);

                    conn->OnAquireRealConnectionSuccess(realconn);
                    conn->Release();
                }
            }

            void RealConnectionPool::CheckCreateMore()
            {
                size_t exist = busy_.size() + free_.size() + pending_.size();
                for (size_t i = exist; i < pool_size(); ++i) {
                    CreateNewConnection();
                    LOG_ERROR("auto create more mysql connect in pool!\n");
                }
            }

            void RealConnectionPool::ReleaseRealConnection(RealConnection* conn)
            {
                conn->ResetDataHandler();
                busy_.remove(conn);
                if (conn->connect()) {
                    free_.push_back(conn);
                    CheckAquireList();
                } else {
                    LOG_DEBUG("the real connection is close, so the free list not need it any more!\n");
                }
            }

            /// RealConnectionPoolManager
            RealConnectionPoolManager::RealConnectionPoolManager()
                : mempool_(nullptr), setup_checker_(nullptr), roolback_(false), cleanup_checker_(nullptr), probe_checker_(nullptr)
            {
            }

            RealConnectionPoolManager::~RealConnectionPoolManager()
            {
                for (vector<RealConnectionPool*>::iterator it = pools_.begin(); it != pools_.end(); ++it) {
                    delete *it;
                }
                pools_.clear();

                SAFE_DELETE(mempool_);
                SAFE_RELEASE(setup_checker_);
                SAFE_RELEASE(cleanup_checker_);
                SAFE_RELEASE(probe_checker_);
            }

            void RealConnectionPoolManager::BeginSetup(const boost::function<void(bool)>& cb)
            {
                cb_setup_ = cb;
                if (!config_.ParseXml()) {
                    cb_setup_(false);
                    return;
                }

                mempool_ = new memory::MemoryPool(256, 2560);

                const vector<ConnectionPoolInfo>& pools = config_.pools();
                for (vector<ConnectionPoolInfo>::const_iterator it = pools.begin(); it != pools.end(); ++it) {
                    CreateRealConnectionPool(*it);
                }

                // 定时输出
                probe_checker_ = g_dispatcher->quicktimer().SetIntervalWithLinker(boost::bind(&RealConnectionPoolManager::DebugDump, this), 10 * 60 * 1000);
                probe_checker_->Retain();

                // 定时检查
                setup_checker_ = g_dispatcher->quicktimer().SetIntervalWithLinker(boost::bind(&RealConnectionPoolManager::CheckIfSetupFinish, this), 200);
                setup_checker_->Retain();
            }

            void RealConnectionPoolManager::CheckIfSetupFinish()
            {
                if (roolback_) {
                    bool ok = true;
                    for (vector<RealConnectionPool*>::iterator it = pools_.begin(); it != pools_.end(); ++it) {
                        if (!(*it)->empty()) {
                            ok = false;
                            break;
                        }
                    }
                    if (ok) {
                        cb_setup_(false);
                        SAFE_RELEASE(setup_checker_);
                    }
                } else {
                    bool ok = true;
                    for (vector<RealConnectionPool*>::iterator it = pools_.begin(); it != pools_.end(); ++it) {
                        if ((*it)->empty()) {
                            ok = false;
                            roolback_ = true;
                            break;
                        }
                        if ((*it)->free_size() == 0) {
                            ok = false;
                            break;
                        }
                    }
                    if (roolback_) {
                        // 回滚
                        for (vector<RealConnectionPool*>::iterator it = pools_.begin(); it != pools_.end(); ++it) {
                            (*it)->Cleanup();
                        }
                    }
                    if (ok) {
                        cb_setup_(true);
                        SAFE_RELEASE(setup_checker_);
                    }
                }
            }

            void RealConnectionPoolManager::BeginCleanup(const boost::function< void() >& cb)
            {
                cb_cleanup_ = cb;
                for (vector<RealConnectionPool*>::iterator it = pools_.begin(); it != pools_.end(); ++it) {
                    (*it)->Cleanup();
                }
                // 定时检查
                cleanup_checker_ = g_dispatcher->quicktimer().SetIntervalWithLinker(boost::bind(&RealConnectionPoolManager::CheckIfCleanupFinish, this), 200);
                cleanup_checker_->Retain();
            }

            void RealConnectionPoolManager::CheckIfCleanupFinish()
            {
                bool ok = true;
                for (vector<RealConnectionPool*>::iterator it = pools_.begin(); it != pools_.end(); ++it) {
                    if (!(*it)->empty()) {
                        ok = false;
                        break;
                    }
                }
                if (ok) {
                    cb_cleanup_();
                    SAFE_RELEASE(cleanup_checker_);
                }
            }

            void RealConnectionPoolManager::CreateRealConnectionPool(const ConnectionPoolInfo& poolinfo)
            {
                if (poolinfo.poolid >= pools_.size()) {
                    pools_.resize(poolinfo.poolid + 1);
                } else {
                    if (pools_[poolinfo.poolid] != nullptr) {
                        throw Exception("duplicate poolid in dbo connection pool!");
                    }
                }
                RealConnectionPool* pool = new RealConnectionPool(*mempool_, poolinfo);
                pools_[poolinfo.poolid] = pool;
                pool->Setup();
            }

            void RealConnectionPoolManager::DebugDump()
            {
                for (vector<RealConnectionPool*>::iterator it = pools_.begin(); it != pools_.end(); ++it) {
                    RealConnectionPool* p = *it;
                    LOG_DEBUG("数据库连接池(%u):, pending:%u, busy:%u, free:%u\n", p->poolid(), p->pending_size(), p->busy_size(), p->free_size());
                }
            }
        }
    }
}
