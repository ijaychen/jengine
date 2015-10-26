#ifndef BASE_DBO_INTERNAL_REALCONNECTION_H
#define BASE_DBO_INTERNAL_REALCONNECTION_H

#include "../../net/client.h"
#include "realconnectiondatahandler.h"
#include "configure.h"
#include <string>
#include <map>

namespace base
{
    namespace dbo
    {
        class Statement;
        class PreparedStatement;
        class PreparedStatementMetadata;

        namespace internal
        {
            class PacketIn;
            class PacketOut;

            enum ConnectionPhase {
                CONN_PHASE_INIT_HANDSHAKE,
                CONN_PHASE_HANDSHAKE_RESPONSE,
                CONN_PHASE_OK,
            };

            // 代表一个真实的连接
            class RealConnection : public net::Client
            {
            public:
                struct EventHandler {
                    virtual ~EventHandler() {}
                    // 网络链接成功
                    virtual void OnConnect(RealConnection* sender) = 0;
                    // 网络链接失败
                    virtual void OnConnectFail(RealConnection* sender, const char* reason) = 0;
                    // 网络链接关闭
                    virtual void OnClose(RealConnection* sender) = 0;
                    // 认证成功
                    virtual void OnAuthSuccess(RealConnection* sender) = 0;
                    // 认证失败
                    virtual void OnAuthFail(RealConnection* sender, int errcode, const char* reason) = 0;
                };
                
                RealConnection(EventHandler& handler, memory::MemoryPool& mempool, uint32_t poolid);
                virtual ~RealConnection();

                uint32_t poolid() const {
                    return poolid_;
                }

                void Send(PacketOut& pktout);
                void Connect(const ServerInfo& serverinfo);

                void SetDataHandler(RealConnectionDataHandler* dthd) {
                    data_handler_ = dthd;
                }
                void ResetDataHandler() {
                    data_handler_ = nullptr;
                }

                PreparedStatementMetadata* FetchCachedPreparedStatement(const std::string& sql) {
                    std::map<std::string, PreparedStatementMetadata*>::iterator it = prepared_stmt_pool_.find(sql);
                    return it == prepared_stmt_pool_.end() ? 0 : it->second;
                }

                void SavePreparedStatementCache(const std::string& sql, PreparedStatementMetadata* pstmt) {
                    prepared_stmt_pool_[sql] = pstmt;
                }

            private:
                virtual void OnConnect();
                virtual void OnConnectFail(int eno, const char* reason);
                virtual void OnClose();
                virtual void OnReceive(std::size_t count);

                void OnReceivePacket(PacketIn& pktin);

                void HandleAuthInitHandshake(PacketIn& pktin);
                void HandleAuthResponse(PacketIn& pktin);

                EventHandler& handler_;
                RealConnectionDataHandler* data_handler_;
                ConnectionPhase conn_phase_;
                ServerInfo serverinfo_;
                uint32_t poolid_;

            private:
                // prepared statment 池
                std::map<std::string, PreparedStatementMetadata*> prepared_stmt_pool_;
            };
        }
    }
}

#endif // REALCONNECTION_H
