#ifndef BASE_DBO_PREPAREDSTATEMENT_H
#define BASE_DBO_PREPAREDSTATEMENT_H

#include "statement.h"
#include "resultset.h"

namespace base
{
    namespace dbo
    {
        namespace internal
        {
            class PacketOut;
        }

        class PreparedStatement : public StatementBase
        {
        public:
            PreparedStatement(Connection& conn, const char* sql);
            virtual ~PreparedStatement();
            
            const std::string& sql() const {
                return sql_;
            }

            // 考虑到效率，我们需要按顺序设置参数值
            void SetNull();
            void SetBoolean(bool param) {
                SetInt8(param ? 1 : 0);
            }
            void SetInt8(int8_t param);
            void SetUInt8(uint8_t param);
            void SetInt16(int16_t param);
            void SetUInt16(uint16_t param);
            void SetInt32(int32_t param);
            void SetUInt32(uint32_t param);
            void SetInt64(int64_t param);
            void SetUInt64(uint64_t param);
            void SetString(const char* param, uint32_t len);
            void SetString(const std::string& param);
            void SetBinary(const std::string& param);
            void SetFloat(float param);
            void SetDouble(double param);
            // 不支持时间包含毫秒段
            void SetDateTime(const tm& dt);

            virtual void Execute(const boost::function< void(ResultSet&) >& cb);

            virtual bool IsWait() const {
                return phase_ == PREPARE_WAIT;
            }
            virtual bool IsFinish() const {
                return phase_ == PREPARE_FINISH;
            }

            void SetCursorFlag(uint8_t flag) {
                cursor_flag_ = flag;
            }

        private:
            virtual void OnCallRealExecute();
            virtual void HandleResponse(internal::PacketIn& pktin);
            virtual void HandleClose();
            void HandleFinish();
            void HandlePrepareFinish();
            std::string sql_;
            uint8_t cursor_flag_;
            boost::function<void(ResultSet&)> cb_;
            PreparedStatementMetadata* metadata_pstmt_;
            ResultSet rs_;
            enum PreparePhase {
                PREPARE_NEW,                    // 新建，需要进行prepare
                PREPARE_WAIT,                   // 等待执行
                PREPARE_WAIT_PREPARE_RESPONSE,  // 等待prepare结果
                PREPARE_WAIT_PREPARE_RESPONSE_PARAM_DEF,
                PREPARE_WAIT_PREPARE_RESPONSE_COLUMN_DEF,
                PREPARE_PREPARE_OK,             // 准备OK, 可以execute或send_long_data
                PREPARE_WAIT_EXEC_RESPONSE,     // 等待接收执行结果
                PREPARE_FINISH,                 // 执行完华
            };
            PreparePhase phase_;
            std::vector<char> bitmap_;
            std::vector<uint16_t> params_type_;
            internal::PacketOut* params_;
            uint32_t params_idx_;
            void inline EnsureParamBitmapCapacity() {
                uint32_t bitmap_len = (params_idx_ + 7) / 8;
                if (bitmap_.size() < bitmap_len) {
                    bitmap_.resize(bitmap_len);
                }
            }
        };
    }
}

#endif // PREPAREDSTATEMENT_H
