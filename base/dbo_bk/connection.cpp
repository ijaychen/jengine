#include "connection.h"
#include "internal/realconnectionpool.h"
#include "../exception.h"
#include "../logger.h"

namespace base
{
    namespace dbo
    {
        using namespace std;

        Connection::Connection()
            : realconn_(nullptr), cur_stmt_(nullptr)
        {
        }

        Connection::~Connection()
        {
            if (realconn_) {
                internal::RealConnectionPoolManager::instance().ReleaseRealConnection(realconn_);
                realconn_->Release();
                realconn_ = nullptr;
            }
            SAFE_DELETE(cur_stmt_);
        }

        bool Connection::pending() const
        {
            return cur_stmt_ && !cur_stmt_->IsFinish();
        }

        bool Connection::is_valid_poolid(int32_t poolid)
        {
            return internal::RealConnectionPoolManager::instance().config().is_valid_poolid(poolid);
        }

        Connection* Connection::Create(int32_t poolid)
        {
            // 创建一个连接
            Connection* ptr = new Connection;
            ptr->AutoRelease();
            internal::RealConnectionPoolManager::instance().AquireRealConnection(poolid, ptr);
            return ptr;
        }

        void Connection::OnAquireRealConnectionSuccess(internal::RealConnection* realconn)
        {
            realconn_ = realconn;
            realconn_->Retain();
            realconn_->SetDataHandler(this);
            TryExecuteStatement();
        }

        void Connection::TryExecuteStatement()
        {
            if (cur_stmt_) {
                cur_stmt_->CallRealExecute();
            }
        }

        Statement& Connection::CreateStatement(const char* sql)
        {
            if (cur_stmt_ != nullptr) {
                if (cur_stmt_->IsFinish()) {
                    SAFE_DELETE(cur_stmt_);
                } else {
                    throw Exception("another statement under progress");
                }
            }
            Statement* stmt = new Statement(*this, sql);
            cur_stmt_ = stmt;
            return *stmt;
        }

        PreparedStatement& Connection::CreatePreparedStatement(const char* sql)
        {
            if (cur_stmt_ != nullptr) {
                if (cur_stmt_->IsFinish()) {
                    SAFE_DELETE(cur_stmt_);
                } else {
                    throw Exception("another statement under progress");
                }
            }
            PreparedStatement* pstmt = new PreparedStatement(*this, sql);
            cur_stmt_ = pstmt;
            return *pstmt;
        }

        void Connection::OnReceivePacket(internal::RealConnection* sender, internal::PacketIn& pktin)
        {
            if (cur_stmt_) {
                cur_stmt_->HandleRawResponse(pktin);
            }
        }

        void Connection::OnClose(internal::RealConnection* sender)
        {
            LOG_ERROR("mysql server disconnected, so the statement can not execute any more!\n");
            if (cur_stmt_) {
                cur_stmt_->HandleClose();
            }
        }
    }
}
