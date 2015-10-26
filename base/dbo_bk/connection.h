#ifndef BASE_DBO_CONNECTION_H
#define BASE_DBO_CONNECTION_H

#include "../object.h"
#include "internal/realconnectiondatahandler.h"
#include "statement.h"
#include "preparedstatement.h"
#include <string>

namespace base
{
    namespace dbo
    {
        namespace internal
        {
            class RealConnection;
        }

        class Connection : public Object, public internal::RealConnectionDataHandler
        {
        public:
            static bool is_valid_poolid(int32_t poolid);
            static Connection* Create(int32_t poolid = 0);
            virtual ~Connection();
            
            bool pending() const;

            void OnAquireRealConnectionSuccess(internal::RealConnection* realconn);

            Statement& CreateStatement(const char* sql);
            Statement& CreateStatement(const std::string& sql) {
                return CreateStatement(sql.c_str());
            }
            PreparedStatement& CreatePreparedStatement(const char* sql);
            PreparedStatement& CreatePreparedStatement(const std::string& sql) {
                return CreatePreparedStatement(sql.c_str());
            }

        private:
            virtual void OnReceivePacket(internal::RealConnection* sender, internal::PacketIn& pktin);
            virtual void OnClose(internal::RealConnection* sender);

        private:
            // 尝试执行语话，如果处于未连接状态，则中止等待
            void TryExecuteStatement();
            Connection();

        private:
            internal::RealConnection* realconn_;
            // 当前执行的语句
            StatementBase* cur_stmt_;
            friend class StatementBase;
        };
    }
}

#endif // CONNECTION_H
