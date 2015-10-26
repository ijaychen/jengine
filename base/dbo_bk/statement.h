#ifndef BASE_DBO_STATEMENT_H
#define BASE_DBO_STATEMENT_H

#include "../global.h"
#include "resultset.h"
#include "internal/rowreader.h"
#include <boost/function.hpp>

namespace base
{
    namespace dbo
    {
        class Connection;

        namespace internal
        {
            class PacketIn;
            class RealConnection;
        }

        class StatementBase
        {
        public:
            StatementBase(Connection& conn)
                : conn_(conn), realconn_(nullptr), sequenct_id_(0) {}
            virtual ~StatementBase();

            // 是否正在等待执行
            virtual bool IsWait() const = 0;
            // 是否已执行完毕
            virtual bool IsFinish() const = 0;
            virtual void Execute(const boost::function<void(ResultSet&)>& cb) = 0;
            void HandleRawResponse(internal::PacketIn& pktin);
            virtual void HandleResponse(internal::PacketIn& pktin) = 0;
            virtual void HandleClose() = 0;
            // 连接已准备就绪，正式执行语句
            void CallRealExecute() ;

        protected:
            internal::RealConnection* realconn() {
                return realconn_;
            }
            void ResetSequenceID() {
                sequenct_id_ = 0;
            }
            uint8_t FetchSequenceID() {
                return sequenct_id_++;
            }
            Connection& conn() {
                return conn_;
            }

        private:
            Connection& conn_;
            virtual void OnCallRealExecute() = 0;
            void SetRealConn(internal::RealConnection* realconn) {
                realconn_ = realconn;
            }
            internal::RealConnection* realconn_;
            uint8_t sequenct_id_;
            friend class Connection;
        };

        class Statement : public StatementBase
        {
        public:
            Statement(Connection& conn, const char* sql)
                : StatementBase(conn), sql_(sql), phase_(STATEMENT_NEW) {}
            virtual ~Statement();

            virtual void Execute(const boost::function<void(ResultSet&)>& cb);

            virtual bool IsWait() const {
                return phase_ == STATEMENT_WAIT;
            }
            virtual bool IsFinish() const {
                return phase_ == STATEMENT_FINISH;
            }

        private:
            virtual void OnCallRealExecute();
            virtual void HandleResponse(internal::PacketIn& pktin);
            virtual void HandleClose();

            std::string sql_;
            boost::function<void(ResultSet&)> cb_;
            ResultSet rs_;

            enum StatementPhase {
                STATEMENT_NEW,                  // 新建的语句
                STATEMENT_WAIT,         // 等待执行的语句
                STATEMENT_WAIT_RESPONSE,        // 等待执行结果
                STATEMENT_FINISH,               // 已完成的语句
            };
            StatementPhase phase_;
        };
    }
}

#endif // STATEMENT_H
